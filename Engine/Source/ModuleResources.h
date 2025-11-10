#pragma once
#include "Module.h"
#include <d3d12.h>
#include <wrl.h>
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class ModuleD3D12;

class ModuleResources : public Module {
public:
    ModuleResources();
    ~ModuleResources() override;

    bool init() override;
    bool postInit() override;
    void update() override;
    void preRender() override;
    void render() override;
    void postRender() override;
    bool cleanUp() override;

public:
    void CreateUploadBuffer(const void* cpuData, UINT64 bufferSize);
    void CreateDefaultBuffer(const void* cpuData, UINT64 bufferSize);

private:
    ModuleD3D12* d3d12 = nullptr;
    ID3D12Device5* device = nullptr;

    ComPtr<ID3D12Resource> buffer;
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> stagingBuffer;
};
