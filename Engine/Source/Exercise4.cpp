#include "Globals.h"
#include "Exercise4.h"

#include "Application.h"
#include "Settings.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ModuleRender.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleCamera.h"
#include "ModuleUI.h"

#include "ReadData.h"

#include "DirectXTex.h"
#include <d3d12.h>
#include "d3dx12.h"

bool Exercise4::init() {
    settings = app->getSettings();

    moduleD3d12 = app->getModuleD3D12();
    moduleResources = app->getModuleResources();
    moduleRender = app->getModuleRender();
    moduleCamera = app->getModuleCamera();
    moduleShaderDescriptors = app->getModuleShaderDescriptors();

    // Register UI window
    ModuleUI* moduleUI = app->getModuleUI();
    moduleUI->registerWindow([this]() { drawGUI(); });

    struct Vertex {
        Vector3 position;
        Vector2 uv;
    };

    static Vertex vertices[6] =
    {
        { Vector3(-1.0f, -1.0f, 0.0f),  Vector2(-0.2f, 1.2f) },
        { Vector3(-1.0f, 1.0f, 0.0f),   Vector2(-0.2f, -0.2f) },
        { Vector3(1.0f, 1.0f, 0.0f),    Vector2(1.2f, -0.2f) },
        { Vector3(-1.0f, -1.0f, 0.0f),  Vector2(-0.2f, 1.2f) },
        { Vector3(1.0f, 1.0f, 0.0f),    Vector2(1.2f, -0.2f) },
        { Vector3(1.0f, -1.0f, 0.0f),   Vector2(1.2f, 1.2f) }
    };

    bool ok = createVertexBuffer(&vertices[0], sizeof(vertices), sizeof(Vertex));
    ok = ok && createRootSignature();
    ok = ok && createPSO();

    if (ok) {
        textureDog = moduleResources->createTextureFromFile(L"Assets/Textures/dog.dds");
        ok = (textureDog != nullptr);
    }

    if (ok) {
        textureTable = moduleShaderDescriptors->alloc(1);
        moduleShaderDescriptors->createTextureSRV(textureTable, 0, textureDog.Get());
    }

    if (ok) {
        debugDrawPass = std::make_unique<DebugDrawPass>(
            moduleD3d12->getDevice(),
            moduleD3d12->getDrawCommandQueue()
        );
    }

    return ok;
}

bool Exercise4::cleanUp() {
    return true;
}

void Exercise4::render() {
    moduleRender->registerWorldPass([this](ID3D12GraphicsCommandList* cmd) {
        unsigned width = moduleD3d12->getWindowWidth();
        unsigned height = moduleD3d12->getWindowHeight();

        Matrix model =
            Matrix::CreateScale(textScale) *
            Matrix::CreateFromYawPitchRoll(
                XMConvertToRadians(textRot.y),
                XMConvertToRadians(textRot.x),
                XMConvertToRadians(textRot.z)
            ) *
            Matrix::CreateTranslation(textPos);

        Matrix view = moduleCamera->getView();
        Matrix proj = moduleCamera->getProjection();
        mvp = (model * view * proj).Transpose();

        cmd->SetGraphicsRootSignature(rootSignature.Get());
        cmd->SetPipelineState(pso.Get());

        cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd->IASetVertexBuffers(0, 1, &vertexBufferView);

        ID3D12DescriptorHeap* heaps[] = { moduleShaderDescriptors->getHeap() };
        cmd->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);
        cmd->SetDescriptorHeaps(1, heaps);
        cmd->SetGraphicsRootDescriptorTable(1, moduleShaderDescriptors->getGPUHandle(textureTable));

        cmd->DrawInstanced(6, 1, 0, 0);

        // ---- DebugDraw ----
        if (settings->sceneEditor.showGrid)
            dd::xzSquareGrid(-10.f, 10.f, 0.f, 1.f, dd::colors::LightGray);

        if (settings->sceneEditor.showAxis)
            dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

        debugDrawPass->record(cmd, width, height, view, proj);
    });
}


bool Exercise4::createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride) {
    vertexBuffer = moduleResources->createDefaultBuffer(bufferData, bufferSize, "QuadVB");

    if (vertexBuffer)
    {
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = stride;
        vertexBufferView.SizeInBytes = bufferSize;

        return true;
    }

    return false;
}


bool Exercise4::createRootSignature() {
    CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
    CD3DX12_DESCRIPTOR_RANGE srvRange;

    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    // b0
    rootParameters[0].InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    // t0
    rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

    // --- STATIC SAMPLER s0 ---
    CD3DX12_STATIC_SAMPLER_DESC staticSampler;
    staticSampler.Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init(2, rootParameters, 1, &staticSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> blob;
    if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr)))
        return false;

    return SUCCEEDED(
        moduleD3d12->getDevice()->CreateRootSignature( 0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))
    );
}


bool Exercise4::createPSO()
{
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                                              {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

    auto dataVS = DX::ReadData(L"Exercise4VS.cso");
    auto dataPS = DX::ReadData(L"Exercise4PS.cso");

    assert(!dataVS.empty() && !dataPS.empty());

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
    return SUCCEEDED(moduleD3d12->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void Exercise4::drawGUI() {
    if (ImGui::Begin("Exercise 4 Controls"))
    {
        if (ImGui::CollapsingHeader("Texture position"))
        {
            ImGui::DragFloat3("Position###TextPos", &textPos.x, 0.1f);
            ImGui::DragFloat3("Rotation###TextRot", &textRot.x, 0.5f);
            ImGui::DragFloat3("Scale###TextScale", &textScale.x, 0.1f);
        }
        /*if (ImGui::CollapsingHeader("Samplers")) {
            ImGui::Combo("Sampler", &sampler, "Linear/Wrap\0Point/Wrap\0Linear/Clamp\0Point/Clamp", ModuleSamplers::COUNT);
            ImGui::End();
        }*/
    }
    ImGui::End();
}
