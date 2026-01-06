#include "Globals.h"
#include "ModuleDSDescriptors.h"

#include "Application.h"
#include "ModuleD3D12.h"

bool ModuleDSDescriptors::init() {
    moduleD3d12 = app->getModuleD3D12();
    auto* device = moduleD3d12->getDevice();

    capacity = 64;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    desc.NumDescriptors = capacity;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))))
        return false;

    descriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);

    // Inicializar free list
    freeList.reserve(capacity);
    for (UINT i = 0; i < capacity; ++i) {
        freeList.push_back(capacity - 1 - i);
    }

    return true;
}

bool ModuleDSDescriptors::cleanUp() {
    freeList.clear();
    heap.Reset();
    return true;
}

UINT ModuleDSDescriptors::alloc() {
    if (freeList.empty()) {
        _ASSERT_EXPR(false, "DSV Descriptor Heap exhausted");
        return UINT(-1);
    }

    UINT index = freeList.back();
    freeList.pop_back();
    return index;
}

void ModuleDSDescriptors::free(UINT index) {
    if (index == UINT(-1))
        return;

    freeList.push_back(index);
}

void ModuleDSDescriptors::create(UINT index, ID3D12Resource* resource) {
    auto* device = moduleD3d12->getDevice();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = resource->GetDesc().Format;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D.MipSlice = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(index);
    device->CreateDepthStencilView(resource, &dsvDesc, handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleDSDescriptors::getCPUHandle(UINT index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += size_t(index) * size_t(descriptorSize);
    return handle;
}
