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
