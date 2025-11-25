#include "Globals.h"
#include "ModuleExercise3.h"

#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ModuleUI.h"

#include "ReadData.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

bool ModuleExercise3::init()
{
    moduleD3d12 = app->getModuleD3D12();
    moduleResources = app->getModuleResources();
    ui = app->getModuleUI();

    // UI window registration
    ui->registerWindow([this]() { drawGUI(); });

    // ---- Triangle vertex buffer ----
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

void ModuleExercise3::render()
{
    if (camPos == camTarget)
        camTarget.x += 0.001f;

    ID3D12GraphicsCommandList* commandList = moduleD3d12->getCommandList();
    unsigned width = moduleD3d12->getWindowWidth();
    unsigned height = moduleD3d12->getWindowHeight();

    // --- Compute MVP ---
    Matrix model =
        Matrix::CreateScale(triScale) *
        Matrix::CreateFromYawPitchRoll(
            XMConvertToRadians(triRot.y),
            XMConvertToRadians(triRot.x),
            XMConvertToRadians(triRot.z)
        ) *
        Matrix::CreateTranslation(triPos);

    Matrix view = Matrix::CreateLookAt(camPos, camTarget, Vector3::Up);
    Matrix proj = Matrix::CreatePerspectiveFieldOfView(
        XM_PIDIV4, float(width) / float(height), 0.1f, 1000.0f
    );

    mvp = (model * view * proj).Transpose();

    // ---- Pipeline ----
    commandList->SetPipelineState(pso.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());

    // ---- Draw triangle ----
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    commandList->SetGraphicsRoot32BitConstants(
        0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0
    );

    commandList->DrawInstanced(3, 1, 0, 0);

    // ---- DebugDraw ----
    dd::xzSquareGrid(-10.f, 10.f, 0.f, 1.f, dd::colors::LightGray);
    dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

    debugDrawPass->record(commandList, width, height, view, proj);
}

bool ModuleExercise3::createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride)
{
    ModuleResources* resources = app->getModuleResources();

    vertexBuffer = resources->createDefaultBuffer(bufferData, bufferSize, "Triangle");
    bool ok = vertexBuffer != nullptr;

    if (ok)
    {
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = bufferSize;
        vertexBufferView.StrideInBytes = stride;
    }

    return ok;
}

bool ModuleExercise3::createRootSignature()
{
    CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
    CD3DX12_ROOT_PARAMETER params[1];

    params[0].InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0);

    rootDesc.Init(
        1, params,
        0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> error;

    if (FAILED(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error)))
        return false;

    if (FAILED(moduleD3d12->getDevice()->CreateRootSignature(
        0, blob->GetBufferPointer(), blob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature))))
        return false;

    return true;
}

bool ModuleExercise3::createPSO()
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

    return SUCCEEDED(moduleD3d12->getDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso)));
}


bool ModuleExercise3::cleanUp()
{
    debugDrawPass.reset();
    return true;
}

// ---------------- GUI ----------------

void ModuleExercise3::drawGUI()
{
    if (ImGui::Begin("Exercise 3 Controls"))
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            ImGui::DragFloat3("Position###CameraPos", &camPos.x, 0.1f);
            ImGui::DragFloat3("LookAt###CameraLookAt", &camTarget.x, 0.1f);
        }

        if (ImGui::CollapsingHeader("Triangle"))
        {
            ImGui::DragFloat3("Position###TriPos", &triPos.x, 0.1f);
            ImGui::DragFloat3("Rotation###TriRot", &triRot.x, 0.5f);
            ImGui::DragFloat3("Scale###TriScale", &triScale.x, 0.1f);
        }
    }
    ImGui::End();
}
