#pragma once

#include "Module.h"
#include "DebugDrawPass.h"

class ModuleD3D12;
class ModuleResources;
class ImGuiPass;

class ModuleExercise3 : public Module {
private:
    ModuleD3D12* moduleD3d12;
    ModuleResources* moduleResources;

    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pso;
    std::unique_ptr<DebugDrawPass> debugDrawPass;

    Matrix mvp;
public:
    virtual bool init() override;
    virtual void preRender() override;
    virtual void render() override;
    virtual bool cleanUp() override;

private:
    bool createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride);
    bool createRootSignature();
    bool createPSO();

private:
    ImGuiPass* imguiPass;
private:
    void drawGUI();

    Vector3 triPos = { 0.0f, 0.0f, 0.0f };
    Vector3 triRot = { 0.0f, 0.0f, 0.0f };
    Vector3 triScale = { 1.0f, 1.0f, 1.0f };

    Vector3 camPos = { 0.0f, 10.0f, 10.0f };
    Vector3 camTarget = { 0.0f, 0.0f, 0.0f };
};
