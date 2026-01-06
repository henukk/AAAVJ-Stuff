#include "Globals.h"
#include "ModuleRTDescriptors.h"

#include "Application.h"
#include "ModuleD3D12.h"

bool ModuleRTDescriptors::init() {
    moduleD3d12 = app->getModuleD3D12();
    auto* device = moduleD3d12-> getDevice();

    capacity = 64;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = capacity;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))))
        return false;

    descriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);

    freeList.reserve(capacity);
    for (UINT i = 0; i < capacity; ++i) {
        freeList.push_back(capacity - 1 - i);
    }

    return true;
}

bool ModuleRTDescriptors::cleanUp() {
    freeList.clear();
    heap.Reset();
    return true;
}

UINT ModuleRTDescriptors::alloc() {
    if (freeList.empty())
    {
        _ASSERT_EXPR(false, "RTV Descriptor Heap exhausted");
        return UINT(-1);
    }

    UINT index = freeList.back();
    freeList.pop_back();
    return index;
}

void ModuleRTDescriptors::free(UINT index) {
    if (index == UINT(-1))
        return;

    freeList.push_back(index);
}

void ModuleRTDescriptors::create(UINT index, ID3D12Resource* resource) {
    auto* device = moduleD3d12->getDevice();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = resource->GetDesc().Format;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(index);
    device->CreateRenderTargetView(resource, &rtvDesc, handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleRTDescriptors::getCPUHandle(UINT index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += size_t(index) * size_t(descriptorSize);
    return handle;
}
