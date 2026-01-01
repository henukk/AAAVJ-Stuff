#pragma once
#include "Module.h"

class ModuleD3D12;

using Microsoft::WRL::ComPtr;

class ModuleRingBuffer : public Module {
private:
    ModuleD3D12* moduleD3d12;
    
    UINT64 align(UINT64 size) const;

    ComPtr<ID3D12Resource> buffer;
    uint8_t* cpuBasePtr = nullptr;

    UINT64 totalSize = 0;
    UINT64 head = 0;
    UINT64 tail = 0;

    static const UINT MAX_FRAMES = 3;
    UINT64 frameAllocations[MAX_FRAMES] = {};

    UINT currentFrameIndex = 0;
public:
    ModuleRingBuffer();
    ~ModuleRingBuffer();

    bool init() override;
    void preRender() override;
    bool cleanUp() override;

    D3D12_GPU_VIRTUAL_ADDRESS allocBuffer(void** cpuPtrOut) {
        return allocBuffer(alignUp(sizeof(void**), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), cpuPtrOut);
    }
    D3D12_GPU_VIRTUAL_ADDRESS allocBuffer(UINT64 size, void** cpuPtrOut);
};
