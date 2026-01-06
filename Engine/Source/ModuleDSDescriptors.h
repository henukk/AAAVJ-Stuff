#pragma once

#include "Module.h"
#include <d3d12.h>

class ModuleD3D12;

class ModuleDSDescriptors : public Module {
private:
    ModuleD3D12* moduleD3d12;

    ComPtr<ID3D12DescriptorHeap> heap;
    UINT descriptorSize = 0;
    UINT capacity = 0;

    std::vector<UINT> freeList;
public:
    ModuleDSDescriptors() = default;
    ~ModuleDSDescriptors() override = default;

    bool init() override;
    bool cleanUp() override;

    UINT alloc();
    void free(UINT index);

    void create(UINT index, ID3D12Resource* resource);

    D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(UINT index) const;

};
