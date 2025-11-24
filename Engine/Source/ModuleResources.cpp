#include "Globals.h"
#include "ModuleResources.h"

#include "Application.h"
#include "ModuleD3D12.h"

ModuleResources::ModuleResources() {

}

ModuleResources::~ModuleResources() {

}

bool ModuleResources::init() {
    d3d12 = app->getModuleD3D12();
    device = d3d12->getDevice();
	return true;
}

bool ModuleResources::postInit() {
	return true;
}

void ModuleResources::update() {

}

void ModuleResources::preRender() {

}

void ModuleResources::render() {

}

void ModuleResources::postRender() {

}

bool ModuleResources::cleanUp() {
	buffer.Reset();
	vertexBuffer.Reset();
	stagingBuffer.Reset();

	return true;
}

void ModuleResources::CreateUploadBuffer(const void* cpuData, UINT64 bufferSize) { //parametro o constexpr??
	// 1. Describe the buffer
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	// 2. Specify UPLOAD heap properties
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	// 3. Create the resource
	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	// Map the buffer: get a CPU pointer to its memory
	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0); // We won't read from it, so range is (0,0)
	buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
	// Copy our application data into the GPU buffer
	memcpy(pData, cpuData, bufferSize);
	// Unmap the buffer (invalidate the pointer)
	buffer->Unmap(0, nullptr);
}

ComPtr<ID3D12Resource> ModuleResources::CreateDefaultBuffer(
    const void* cpuData, UINT64 bufferSize)
{
    ComPtr<ID3D12Resource> defaultBuffer;
    ComPtr<ID3D12Resource> uploadBuffer;

    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    // Create default buffer (GPU only)
    device->CreateCommittedResource(
        &defaultHeap,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer)
    );

    // Create upload buffer (CPU-visible)
    device->CreateCommittedResource(
        &uploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );

    // Copy CPU data into upload buffer
    BYTE* pData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
    memcpy(pData, cpuData, bufferSize);
    uploadBuffer->Unmap(0, nullptr);

    // Copy upload -> default
    auto cmd = d3d12->getCommandList();
    cmd->CopyBufferRegion(defaultBuffer.Get(), 0, uploadBuffer.Get(), 0, bufferSize);

    // Add required barrier (VERY IMPORTANT)
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_GENERIC_READ
    );
    cmd->ResourceBarrier(1, &barrier);

    // return the GPU buffer, but keep uploadBuffer alive long enough if needed
    this->stagingBuffer = uploadBuffer; // keep ref for this frame
    return defaultBuffer;
}
