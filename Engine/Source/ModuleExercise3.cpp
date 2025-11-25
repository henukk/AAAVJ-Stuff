#include "Globals.h"
#include "ModuleExercise3.h"

#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"

#include "ReadData.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

#include "ImGuiPass.h"

bool ModuleExercise3::init() {
    struct Vertex { Vector3 position; };
    static Vertex vertices[3] = {
        { Vector3(-1, -1, 0) },
        { Vector3(0,  1, 0) },
        { Vector3(1, -1, 0) }
    };

    bool ok = createVertexBuffer(vertices, sizeof(vertices), sizeof(Vertex));
    ok = ok && createRootSignature();
    ok = ok && createPSO();

    moduleD3d12 = app->getModuleD3D12();
    debugDrawPass = std::make_unique<DebugDrawPass>(moduleD3d12->getDevice(), moduleD3d12->getDrawCommandQueue());

    imguiPass = new ImGuiPass(moduleD3d12->getDevice(), moduleD3d12->getHWnd());

    return ok;
}

void ModuleExercise3::preRender()
{
    imguiPass->startFrame();

    drawGUI();
}

void ModuleExercise3::render()
{
    if (camPos == camTarget) {
        camTarget.x += 0.001f;
    }

    ID3D12GraphicsCommandList* commandList = moduleD3d12->getCommandList();

    unsigned width = moduleD3d12->getWindowWidth();
    unsigned height = moduleD3d12->getWindowHeight();

    // Matrix con los valores actualizados desde ImGui
    Matrix model =
        Matrix::CreateScale(triScale) *
        Matrix::CreateFromYawPitchRoll(
            XMConvertToRadians(triRot.y),
            XMConvertToRadians(triRot.x),
            XMConvertToRadians(triRot.z)
        ) *
        Matrix::CreateTranslation(triPos);

    Matrix view = Matrix::CreateLookAt(camPos, camTarget, Vector3::Up);
    Matrix proj = Matrix::CreatePerspectiveFieldOfView(XM_PIDIV4, float(width) / float(height), 0.1f, 1000.0f);

    mvp = (model * view * proj).Transpose();

    //Exercise
    commandList->SetPipelineState(pso.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());

    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.Width = float(width);
    viewport.Height = float(height);

    D3D12_RECT scissor;
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = width;
    scissor.bottom = height;

    float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = moduleD3d12->getRenderTargetDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = moduleD3d12->getDepthStencilDescriptor();

    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);   // set the primitive topology
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);                   // set the vertex buffer (using the vertex buffer view)

    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0);

    commandList->DrawInstanced(3, 1, 0, 0);                                     // finally draw 3 vertices (draw the triangle)

    dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
    dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

    debugDrawPass->record(commandList, width, height, view, proj);

    imguiPass->record(commandList);
}

bool ModuleExercise3::createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride)
{
    ModuleResources* resources = app->getModuleResources();

    vertexBuffer = resources->createDefaultBuffer(bufferData, bufferSize, "Triangle");
    bool ok = vertexBuffer;

    if (ok)
    {
        vertexBufferView = { vertexBuffer->GetGPUVirtualAddress(), bufferSize, stride };
    }

    return ok;
}

bool ModuleExercise3::createRootSignature()
{
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    CD3DX12_ROOT_PARAMETER rootParameters[1];

    rootParameters[0].InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0);
    rootSignatureDesc.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> rootSignatureBlob;

    if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, nullptr)))
    {
        return false;
    }

    if (FAILED(app->getModuleD3D12()->getDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))))
    {
        return false;
    }

    return true;
}

bool ModuleExercise3::createPSO()
{
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

    auto dataVS = DX::ReadData(L"Exercise3VS.cso");
    auto dataPS = DX::ReadData(L"Exercise3PS.cso");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };  // the structure describing our input layout
    psoDesc.pRootSignature = rootSignature.Get();                                                   // the root signature that describes the input data this pso needs
    psoDesc.VS = { dataVS.data(), dataVS.size() };                                                  // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = { dataPS.data(), dataPS.size() };                                                  // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;                         // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                                             // format of the render target
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc = { 1, 0 };                                                                    // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff;                                                                // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);                               // a default rasterizer state.
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);                                         // a default blend state.
    psoDesc.NumRenderTargets = 1;                                                                   // we are only binding one render target

    // create the pso
    return SUCCEEDED(app->getModuleD3D12()->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

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

bool ModuleExercise3::cleanUp()
{
    delete imguiPass;
    imguiPass = nullptr;
    return true;
}

