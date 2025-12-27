#include "Globals.h"
#include "Exercise2.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ModuleUI.h"
#include "ModuleRender.h"
#include "ReadData.h"
#include "imgui.h"
#include "d3dx12.h"

bool Exercise2::init()
{
    d3d12 = app->getModuleD3D12();
    moduleResources = app->getModuleResources();
    ui = app->getModuleUI();
    moduleRender = app->getModuleRender();

    // Register window in UI
    ui->registerWindow([this]() { drawWindow(); });

    // Save initial state
    memcpy(lastVertices, vertices, sizeof(vertices));

    // Upload initial buffer
    recreateVertexBuffer();

    // ---- Root Signature ----
    CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
    rootDesc.Init(
        0, nullptr, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    Microsoft::WRL::ComPtr<ID3DBlob> sig;
    Microsoft::WRL::ComPtr<ID3DBlob> err;
    D3D12SerializeRootSignature(
        &rootDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &sig,
        &err
    );

    d3d12->getDevice()->CreateRootSignature(
        0,
        sig->GetBufferPointer(),
        sig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    );

    // ---- PSO ----
    auto vs = DX::ReadData(L"Exercise2VS.cso");
    auto ps = DX::ReadData(L"Exercise2PS.cso");

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
          0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.VS = { vs.data(), vs.size() };
    psoDesc.PS = { ps.data(), ps.size() };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1, 0 };
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    d3d12->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));

    return true;
}

void Exercise2::update()
{
    // If vertices changed, re-upload buffer
    if (memcmp(lastVertices, vertices, sizeof(vertices)) != 0)
    {
        recreateVertexBuffer();
        memcpy(lastVertices, vertices, sizeof(vertices));
    }
}

void Exercise2::render()
{
    // Register a world pass in the renderer
    moduleRender->registerWorldPass(
        [this](ID3D12GraphicsCommandList* cmd)
        {
            // Clear background
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();
            cmd->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

            // Pipeline
            cmd->SetPipelineState(pso.Get());
            cmd->SetGraphicsRootSignature(rootSignature.Get());

            // Input Assembler
            cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmd->IASetVertexBuffers(0, 1, &vertexBufferView);

            // Draw
            cmd->DrawInstanced(3, 1, 0, 0);
        }
    );
}

void Exercise2::recreateVertexBuffer()
{
    UINT size = sizeof(vertices);

    vertexBuffer = moduleResources->createDefaultBuffer(vertices, size, "Exercise2");

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = size;
}

void Exercise2::drawWindow()
{
    if (ImGui::Begin("Exercise 2 Settings"))
    {
        ImGui::SeparatorText("Background");
        ImGui::ColorEdit4("Clear Color", clearColor);

        ImGui::SeparatorText("Vertices");

        ImGui::DragFloat3("Vertex 0", &vertices[0].x, 0.01f);
        ImGui::DragFloat3("Vertex 1", &vertices[1].x, 0.01f);
        ImGui::DragFloat3("Vertex 2", &vertices[2].x, 0.01f);
    }
    ImGui::End();
}
