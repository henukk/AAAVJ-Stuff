#pragma once
#include "Module.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class ModuleD3D12;
class ModuleResources;
class ModuleUI;

class ModuleExercise2 : public Module {
private:
    struct Vertex { float x, y, z; };

    // Editable vertices
    Vertex vertices[3] = {
        { -1.0f, -1.0f, 0.0f },
        {  0.0f,  1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f }
    };

    // Last synced state (to detect changes)
    Vertex lastVertices[3];

    float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.f };

    ModuleD3D12* d3d12 = nullptr;
    ModuleResources* moduleResources = nullptr;
    ModuleUI* ui = nullptr;

    // GPU data
    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pso;

public:
    bool init() override;
    void update() override;
    void render() override;
    bool cleanUp() override { return true; }

private:
    void recreateVertexBuffer();
    void drawWindow();
};
