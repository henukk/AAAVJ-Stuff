#pragma once
#include "Globals.h"
#include "Module.h"

class ModuleShaderDescriptors : public Module {
    struct FreeBlock {
        UINT base;
        UINT count;
    };

private:
    ComPtr<ID3D12DescriptorHeap> heap;
    UINT descriptorSize = 0;

    UINT capacity = 0;
    UINT nextFree = 0;

    std::vector<FreeBlock> freeBlocks;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart{};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart{};

public:
    bool init() override;

    UINT alloc(UINT count = 1);

    void createTextureSRV(UINT baseIndex, UINT slot, ID3D12Resource* texture);
	void createNullTexture2DSRV(UINT baseIndex, UINT slot);

    D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(UINT baseIndex, UINT slot = 0) const;
    D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(UINT baseIndex, UINT slot = 0) const;

    ID3D12DescriptorHeap* getHeap() const { return heap.Get(); }

    void free(UINT baseIndex, UINT count = 1);
};
