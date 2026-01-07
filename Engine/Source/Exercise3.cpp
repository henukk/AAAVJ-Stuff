#include "Globals.h"
#include "Exercise3.h"

#include "Application.h"
#include "Settings.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleUI.h"
#include "ModuleEditor.h"

#include "ReadData.h"
#include <d3dcompiler.h>
#include "d3dx12.h"

bool Exercise3::init() {
    settings = app->getSettings();

    moduleD3d12 = app->getModuleD3D12();
    moduleResources = app->getModuleResources();
    ui = app->getModuleUI();
    moduleRender = app->getModuleRender();
    moduleCamera = app->getModuleCamera();

    // Register UI window
    ui->registerWindow([this]() { drawGUI(); });

    // --- Vertex buffer ---
    struct Vertex { Vector3 position; };
    static Vertex vertices[3] = {
        { Vector3(-1, -1, 0) },
        { Vector3(0,  1, 0) },
        { Vector3(1, -1, 0) }
    };

    bool ok = createVertexBuffer(vertices, sizeof(vertices), sizeof(Vertex));
    ok = ok && createRootSignature();
    ok = ok && createPSO();

    debugDrawPass = std::make_unique<DebugDrawPass>(
        moduleD3d12->getDevice(),
        moduleD3d12->getDrawCommandQueue()
    );

    return ok;
}

void Exercise3::render() {
    moduleRender->registerWorldPass([this](ID3D12GraphicsCommandList* cmd) {
            ImVec2 sceneSize = app->getModuleEditor()->getSceneSize();
            unsigned width = sceneSize.x;
            unsigned height = sceneSize.y;

            // --- Compute MVP ---
            Matrix model =
                Matrix::CreateScale(triScale) *
                Matrix::CreateFromYawPitchRoll(
                    XMConvertToRadians(triRot.y),
                    XMConvertToRadians(triRot.x),
                    XMConvertToRadians(triRot.z)
                ) *
                Matrix::CreateTranslation(triPos);

            Matrix view = moduleCamera->getView();
            Matrix proj = moduleCamera->getProjection();

            mvp = (model * view * proj).Transpose();

            // ---- Pipeline ----
            cmd->SetPipelineState(pso.Get());
            cmd->SetGraphicsRootSignature(rootSignature.Get());

            cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmd->IASetVertexBuffers(0, 1, &vertexBufferView);

            cmd->SetGraphicsRoot32BitConstants(
                0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0
            );

            cmd->DrawInstanced(3, 1, 0, 0);

            // ---- DebugDraw ----
            if (settings->sceneEditor.showGrid)
                dd::xzSquareGrid(-10.f, 10.f, 0.f, 1.f, dd::colors::LightGray);

            if (settings->sceneEditor.showAxis)
                dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

            debugDrawPass->record(cmd, width, height, view, proj);
        });
}

bool Exercise3::createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride)
{
    vertexBuffer = moduleResources->createDefaultBuffer(bufferData, bufferSize, "Triangle");

    if (!vertexBuffer)
        return false;

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = bufferSize;
    vertexBufferView.StrideInBytes = stride;

    return true;
}

bool Exercise3::createRootSignature()
{
    CD3DX12_ROOT_PARAMETER params[1];
    params[0].InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0);

    CD3DX12_ROOT_SIGNATURE_DESC desc;
    desc.Init(1, params, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> blob, err;
    if (FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err)))
        return false;

    return SUCCEEDED(moduleD3d12->getDevice()->CreateRootSignature(
        0, blob->GetBufferPointer(), blob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature)
    ));
}

bool Exercise3::createPSO()
{
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
          0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    auto vs = DX::ReadData(L"Exercise3VS.cso");
    auto ps = DX::ReadData(L"Exercise3PS.cso");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = rootSignature.Get();
    desc.InputLayout = { inputLayout, _countof(inputLayout) };
    desc.VS = { vs.data(), vs.size() };
    desc.PS = { ps.data(), ps.size() };
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    desc.SampleDesc = { 1, 0 };
    desc.SampleMask = UINT_MAX;
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    return SUCCEEDED(
        moduleD3d12->getDevice()->CreateGraphicsPipelineState(
            &desc, IID_PPV_ARGS(&pso)
        )
    );
}

bool Exercise3::cleanUp()
{
    debugDrawPass.reset();
    return true;
}

void Exercise3::drawGUI() {
    if (ImGui::Begin("Exercise 3 Controls"))
    {
        if (ImGui::CollapsingHeader("Triangle"))
        {
            ImGui::DragFloat3("Position###TriPos", &triPos.x, 0.1f);
            ImGui::DragFloat3("Rotation###TriRot", &triRot.x, 0.5f);
            ImGui::DragFloat3("Scale###TriScale", &triScale.x, 0.1f);
        }
    }
    ImGui::End();
}
