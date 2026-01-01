#include "Globals.h"
#include "ModuleRingBuffer.h"

#include "Application.h"
#include "ModuleD3D12.h"
#include <cassert>


#define TOTAL_SIZE_BYTES (10*2<<8)
ModuleRingBuffer::ModuleRingBuffer() {
    totalSize = align(TOTAL_SIZE_BYTES);
    memset(frameAllocations, 0, sizeof(frameAllocations));
}

ModuleRingBuffer::~ModuleRingBuffer() {

}

bool ModuleRingBuffer::init() {
    moduleD3d12 = app->getModuleD3D12();
    auto device = moduleD3d12->getDevice();

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

    HRESULT ok = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));

    if (FAILED(ok)) {
        return false;
    }

    ok = buffer->Map(0, nullptr, reinterpret_cast<void**>(&cpuBasePtr));
    if (FAILED(ok)) {
        return false;
    }

    head = 0;
    tail = 0;

    return true;
}

void ModuleRingBuffer::preRender() {
    currentFrameIndex = moduleD3d12->getCurrentBackBufferIdx();

    UINT64 toFree = frameAllocations[currentFrameIndex];
    frameAllocations[currentFrameIndex] = 0;

    tail += toFree;
    if (tail >= totalSize)
        tail -= totalSize;
}

D3D12_GPU_VIRTUAL_ADDRESS ModuleRingBuffer::allocBuffer(UINT64 size, void** cpuPtrOut) {
    size = align(size);

    UINT64 freeSpace = 0;

    if (tail > head) {
        freeSpace = tail - head;
    } 
    else if (tail < head) {
        freeSpace = totalSize - head;
        if (freeSpace < size) {
            head = 0;
            freeSpace = tail;
        }
    }
    else {
        head = 0;
        tail = 0;
        freeSpace = totalSize;
    }

    assert(size <= freeSpace && "RingBuffer OUT OF MEMORY");

    UINT64 offset = head;
    head += size;
    if (head >= totalSize) {
        head -= totalSize;
    }

    frameAllocations[currentFrameIndex] += size;

    *cpuPtrOut = cpuBasePtr + offset;
    return buffer->GetGPUVirtualAddress() + offset;
}
 
bool ModuleRingBuffer::cleanUp() {
    if (buffer) {
        buffer->Unmap(0, nullptr);
        cpuBasePtr = nullptr;
        buffer.Reset();
    }
    return true;
}

UINT64 ModuleRingBuffer::align(UINT64 size) const {
    const UINT64 alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    return (size + alignment - 1) & ~(alignment - 1);
}
