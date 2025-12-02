#pragma once

#include "Module.h"
#include "DebugDrawPass.h"

class ModuleD3D12;
class ModuleResources;
class ModuleUI;
class ModuleRender;
class ModuleCamera;

class ModuleExercise3 : public Module {
private:
    ModuleD3D12* moduleD3d12;
    ModuleResources* moduleResources;
    ModuleUI* ui;
    ModuleRender* moduleRender;
    ModuleCamera* moduleCamera;

    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pso;
    std::unique_ptr<DebugDrawPass> debugDrawPass;

    Matrix mvp;

public:
    bool init() override;
    void render() override;
    bool cleanUp() override;

private:
    bool createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride);
    bool createRootSignature();
    bool createPSO();

    void drawGUI();

    Vector3 triPos = { 0.0f, 0.0f, 0.0f };
    Vector3 triRot = { 0.0f, 0.0f, 0.0f };
    Vector3 triScale = { 1.0f, 1.0f, 1.0f };
};
