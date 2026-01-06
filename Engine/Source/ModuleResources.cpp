#include "Globals.h"
#include "ModuleResources.h"

#include "Application.h"
#include "ModuleD3D12.h"

#include "DirectXTex.h"

bool ModuleResources::init() {
    d3d12 = app->getModuleD3D12();
    device = d3d12->getDevice();

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList));
    commandList->Reset(commandAllocator.Get(), nullptr);

	return true;
}

bool ModuleResources::cleanUp() {
	return true;
}

ComPtr<ID3D12Resource> ModuleResources::createUploadBuffer(const void* data, size_t size, const char* name) {
    //ID3D12CommandQueue* queue = d3d12->getDrawCommandQueue();

    ComPtr<ID3D12Resource> buffer;

    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));

    std::wstring convertStr(name, name + strlen(name));
    buffer->SetName(convertStr.c_str());

    BYTE* pData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
    memcpy(pData, data, size);
    buffer->Unmap(0, nullptr);

    return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::createDefaultBuffer(const void* data, size_t size, const char* name) {
    ID3D12CommandQueue* queue = d3d12->getDrawCommandQueue();

    ComPtr<ID3D12Resource> buffer;

    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
    bool ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer)));

    heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    ComPtr<ID3D12Resource> upload = getUploadHeap(size);

    if (ok)
    {
        BYTE* pData = nullptr;
        CD3DX12_RANGE readRange(0, 0);
        upload->Map(0, &readRange, reinterpret_cast<void**>(&pData));
        memcpy(pData, data, size);
        upload->Unmap(0, nullptr);

        commandList->CopyBufferRegion(buffer.Get(), 0, upload.Get(), 0, size);
        commandList->Close();

        ID3D12CommandList* commandLists[] = { commandList.Get() };
        queue->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

        d3d12->flush();

        commandAllocator->Reset();
        ok = SUCCEEDED(commandList->Reset(commandAllocator.Get(), nullptr));

        std::wstring convertStr(name, name + strlen(name));
        buffer->SetName(convertStr.c_str());
    }

    return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::getUploadHeap(size_t size) {
    ComPtr<ID3D12Resource> uploadHeap;

    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadHeap));

    return uploadHeap;
}

ComPtr<ID3D12Resource> ModuleResources::createTextureFromFile(const std::filesystem::path& path, bool defaultSRGB, bool createMipmaps) {
    const wchar_t* fileName = path.c_str();
    ScratchImage image;
    bool ok = SUCCEEDED(LoadFromDDSFile(fileName, DDS_FLAGS_NONE, nullptr, image));
    ok = ok || SUCCEEDED(LoadFromHDRFile(fileName, nullptr, image));
    ok = ok || SUCCEEDED(LoadFromTGAFile(fileName, defaultSRGB ? TGA_FLAGS_DEFAULT_SRGB : TGA_FLAGS_NONE, nullptr, image));
    ok = ok || SUCCEEDED(LoadFromWICFile(fileName, defaultSRGB ? DirectX::WIC_FLAGS_DEFAULT_SRGB : DirectX::WIC_FLAGS_NONE, nullptr, image));

    if (ok) {
        const TexMetadata& meta = image.GetMetadata();

        if (createMipmaps && meta.mipLevels <= 1 && meta.dimension == TEX_DIMENSION_TEXTURE2D) {
            ScratchImage mipChain;

            HRESULT hr = GenerateMipMaps(
                image.GetImages(),
                image.GetImageCount(),
                meta,
                TEX_FILTER_DEFAULT,
                0,
                mipChain
            );

            if (SUCCEEDED(hr))
            {
                image = std::move(mipChain);
            }
        }

        return createTextureFromImage(image, path.string().c_str());
    }

    return nullptr;
}

ComPtr<ID3D12Resource> ModuleResources::createTextureFromImage(const ScratchImage& image, const char* name) {
    ComPtr<ID3D12Resource> texture;
    const TexMetadata& metaData = image.GetMetadata();

    _ASSERTE(metaData.dimension == TEX_DIMENSION_TEXTURE2D);

    if (metaData.dimension == TEX_DIMENSION_TEXTURE2D)
    {
        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(metaData.format, UINT64(metaData.width), UINT(metaData.height),
            UINT16(metaData.arraySize), UINT16(metaData.mipLevels));

        CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        bool ok = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture)));

        ComPtr<ID3D12Resource> upload;
        if (ok)
        {
            _ASSERTE(metaData.mipLevels * metaData.arraySize == image.GetImageCount());
            upload = getUploadHeap(GetRequiredIntermediateSize(texture.Get(), 0, UINT(image.GetImageCount())));
            ok = upload != nullptr;
        }

        if (ok)
        {
            std::vector<D3D12_SUBRESOURCE_DATA> subData;
            subData.reserve(image.GetImageCount());

            for (size_t item = 0; item < metaData.arraySize; ++item)
            {
                for (size_t level = 0; level < metaData.mipLevels; ++level)
                {
                    const DirectX::Image* subImg = image.GetImage(level, item, 0);

                    D3D12_SUBRESOURCE_DATA data = { subImg->pixels, (LONG_PTR)subImg->rowPitch, (LONG_PTR)subImg->slicePitch };

                    subData.push_back(data);
                }
            }

            ok = UpdateSubresources(commandList.Get(), texture.Get(), upload.Get(), 0, 0, UINT(image.GetImageCount()), subData.data()) != 0;
        }

        if (ok)
        {
            CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandList->ResourceBarrier(1, &barrier);
            commandList->Close();

            ID3D12CommandList* commandLists[] = { commandList.Get() };
            ID3D12CommandQueue* queue = d3d12->getDrawCommandQueue();

            queue->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

            d3d12->flush();

            commandAllocator->Reset();
            ok = SUCCEEDED(commandList->Reset(commandAllocator.Get(), nullptr));

            texture->SetName(std::wstring(name, name + strlen(name)).c_str());
            return texture;
        }
    }

    return ComPtr<ID3D12Resource>();
}

ComPtr<ID3D12Resource> ModuleResources::createRenderTarget(DXGI_FORMAT format, uint32_t width, uint32_t height, const float clearColor[4]) {
    ComPtr<ID3D12Resource> texture;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    memcpy(clearValue.Color, clearColor, sizeof(float) * 4);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = device->CreateCommittedResource( &heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&texture));

    _ASSERT(SUCCEEDED(hr));
    return texture;
}

ComPtr<ID3D12Resource> ModuleResources::createDepthStencil( DXGI_FORMAT format, uint32_t width, uint32_t height, float clearDepth) {
    ComPtr<ID3D12Resource> texture;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.DepthStencil.Depth = clearDepth;
    clearValue.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&texture));

    _ASSERT(SUCCEEDED(hr));
    return texture;
}

