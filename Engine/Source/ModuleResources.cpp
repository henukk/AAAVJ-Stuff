#include "Globals.h"
#include "ModuleResources.h"

#include "Application.h"
#include "ModuleD3D12.h"

ModuleResources::ModuleResources() {

}

ModuleResources::~ModuleResources() {

}

bool ModuleResources::init() {
	return true;
}

bool ModuleResources::postInit() {
	d3d12 = app->getModuleD3D12();
	device = d3d12->getDevice();

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

void ModuleResources::CreateDefaultBuffer(const void* cpuData, UINT64 bufferSize) {
    // --- CREATE THE FINAL GPU BUFFER (DEFAULT HEAP) ---
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &defaultHeap,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)
    );

    // --- CREATE THE STAGING BUFFER (UPLOAD HEAP) ---
    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device->CreateCommittedResource(
        &uploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&stagingBuffer)
    );

    // --- CPU: FILL STAGING BUFFER ---
    BYTE* pData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    stagingBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
    memcpy(pData, cpuData, bufferSize);
    stagingBuffer->Unmap(0, nullptr);

    // --- GPU: COPY DATA ---
    auto commandList = d3d12->getCommandList();
    commandList->CopyResource(vertexBuffer.Get(), stagingBuffer.Get());

    // --- Flush GPU commands to ensure completion ---
    d3d12->flush();
}

