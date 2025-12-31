#pragma once

#include "Module.h"
#include "DebugDrawPass.h"
class Settings;

class ModuleD3D12;
class ModuleResources;
class ModuleShaderDescriptors;
class ModuleRender;
class ModuleCamera;

class Exercise4 : public Module {
private:
    Settings* settings;
    ModuleD3D12* moduleD3d12;
    ModuleResources* moduleResources;
    ModuleShaderDescriptors* moduleShaderDescriptors;
    ModuleRender* moduleRender;
    ModuleCamera* moduleCamera;

    ComPtr<ID3D12Resource>          textureDog;
    ComPtr<ID3D12Resource>          vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW        vertexBufferView;
    ComPtr<ID3D12RootSignature>     rootSignature;
    ComPtr<ID3D12PipelineState>     pso;
    std::unique_ptr<DebugDrawPass>  debugDrawPass;

    Matrix mvp;
    Vector3 textPos = { 0.0f, 0.0f, 0.0f };
    Vector3 textRot = { 0.0f, 0.0f, 0.0f };
    Vector3 textScale = { 1.0f, 1.0f, 1.0f };

    ComPtr<ID3D12DescriptorHeap> srvHeap;
    D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle{};

    UINT textureTable = UINT(-1);

public:
    virtual bool init() override;
    virtual bool cleanUp() override;
    virtual void render() override;

private:
    bool createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride);
    bool createRootSignature();
    bool createPSO();

    void drawGUI();
};