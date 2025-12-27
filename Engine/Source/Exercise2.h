#pragma once
#include "Module.h"
#include <wrl.h>

class ModuleD3D12;
class ModuleResources;
class ModuleUI;
class ModuleRender;

class Exercise2 : public Module {
public:
    struct Vertex { float x, y, z; };

private:
    ModuleD3D12* d3d12;
    ModuleResources* moduleResources;
    ModuleUI* ui;
    ModuleRender* moduleRender;

    Vertex vertices[3] = {
        { -1.f, -1.f, 0.f },
        {  0.f,  1.f, 0.f },
        {  1.f, -1.f, 0.f }
    };

    Vertex lastVertices[3];

    float clearColor[4] = { 0.f, 0.f, 0.f, 1.f };

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

public:
    bool init() override;
    void update() override;
    void render() override;

private:
    void recreateVertexBuffer();
    void drawWindow();
};
