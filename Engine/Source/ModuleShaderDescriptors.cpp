#include "Globals.h"
#include "ModuleShaderDescriptors.h"

#include "Application.h"
#include "ModuleD3D12.h"


bool ModuleShaderDescriptors::init() {
    ID3D12Device* device = app->getModuleD3D12()->getDevice();

    capacity = 1024;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = capacity;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))))
        return false;

    descriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);
    cpuStart = heap->GetCPUDescriptorHandleForHeapStart();
    gpuStart = heap->GetGPUDescriptorHandleForHeapStart();

    nextFree = 1; //imgui reserve
    freeBlocks.clear();

    return true;
}


UINT ModuleShaderDescriptors::alloc(UINT count) {
    for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
        if (it->count >= count) {
            UINT base = it->base;

            if (it->count == count) {
                freeBlocks.erase(it);
            } else {
                it->base += count;
                it->count -= count;
            }

            return base;
        }
    }

    if (nextFree + count > capacity) {
        _ASSERT_EXPR(false, "Shader Descriptor Heap exhausted");
        return UINT(-1);
    }

    UINT base = nextFree;
    nextFree += count;
    return base;
}


void ModuleShaderDescriptors::createTextureSRV(UINT baseIndex, UINT slot, ID3D12Resource* texture) {
    ModuleShaderDescriptors* descriptors = app->getModuleShaderDescriptors();

    D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(baseIndex, slot);
    app->getModuleD3D12()->getDevice()->CreateShaderResourceView(texture, nullptr, handle);
}

void ModuleShaderDescriptors::createNullTexture2DSRV(UINT baseIndex, UINT slot) {
    ID3D12Device* device = app->getModuleD3D12()->getDevice();
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    
    D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(baseIndex, slot);
	device->CreateShaderResourceView(nullptr, &srvDesc, handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleShaderDescriptors::getCPUHandle(UINT base, UINT slot) const {
    return { cpuStart.ptr + (base + slot) * descriptorSize };
}

D3D12_GPU_DESCRIPTOR_HANDLE ModuleShaderDescriptors::getGPUHandle(UINT base, UINT slot) const {
    return { gpuStart.ptr + (base + slot) * descriptorSize };
}

void ModuleShaderDescriptors::free(UINT baseIndex, UINT count) {
    if (baseIndex == UINT(-1) || count == 0)
        return;

    freeBlocks.push_back({ baseIndex, count });
}

