#include "Globals.h"
#include "ModuleAssigment1.h"

#include "Application.h"
#include "Settings.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ModuleSamplers.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleUI.h"

#include "ReadData.h"

#include "DirectXTex.h"
#include <d3d12.h>
#include "d3dx12.h"

ModuleAssigment1::ModuleAssigment1() {
    sampler = int(ModuleSamplers::LINEAR_WRAP);

    textureVector.resize(ModuleAssigment1_TextureList::COUNT);
	selectedTexture = ModuleAssigment1_TextureList::DOG;
}

bool ModuleAssigment1::init() {
    settings = app->getSettings();

    moduleD3d12 = app->getModuleD3D12();
    moduleResources = app->getModuleResources();
	moduleSamplers = app->getModuleSamplers();

    moduleRender = app->getModuleRender();
    moduleCamera = app->getModuleCamera();

    moduleUI = app->getModuleUI();

    // Register UI window
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
        ok = loadTextures();
    }

    ID3D12Device* device = moduleD3d12->getDevice();
    if (ok) {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = ModuleAssigment1_TextureList::COUNT;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        ok = SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap)));
    }

    if (ok) {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
        UINT descriptorSize = device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
        );

        for (int i = 0; i < ModuleAssigment1_TextureList::COUNT; ++i)
        {
            ID3D12Resource* tex = textureVector[i].Get();
            const D3D12_RESOURCE_DESC& desc = tex->GetDesc();

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = desc.MipLevels;

            device->CreateShaderResourceView(tex, &srvDesc, handle);

            handle.ptr += descriptorSize;
        }

        srvGPUHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
    }

    if (ok) {
        debugDrawPass = std::make_unique<DebugDrawPass>(
            moduleD3d12->getDevice(),
            moduleD3d12->getDrawCommandQueue()
        );
    }

    return ok;
}

bool ModuleAssigment1::loadTextures() {
    textureVector[ModuleAssigment1_TextureList::CHECKERBOARD_NO_MIPMAPS]    = moduleResources->createTextureFromFile(L"Assets/Textures/checkboard.jpg", false, false);
    textureVector[ModuleAssigment1_TextureList::CHECKERBOARD]               = moduleResources->createTextureFromFile(L"Assets/Textures/checkboard.jpg");
    textureVector[ModuleAssigment1_TextureList::THIN_GRID_NO_MIPMAPS]       = moduleResources->createTextureFromFile(L"Assets/Textures/thingrid.jpg", false, false);
    textureVector[ModuleAssigment1_TextureList::THIN_GRID]                  = moduleResources->createTextureFromFile(L"Assets/Textures/thingrid.jpg");
    textureVector[ModuleAssigment1_TextureList::SMALL_TEXT_NO_MIPMAPS]      = moduleResources->createTextureFromFile(L"Assets/Textures/small_text.png", false, false);
    textureVector[ModuleAssigment1_TextureList::SMALL_TEXT]                 = moduleResources->createTextureFromFile(L"Assets/Textures/small_text.png");
    textureVector[ModuleAssigment1_TextureList::NOISE_NO_MIPMAPS]           = moduleResources->createTextureFromFile(L"Assets/Textures/noise.jpg", false, false);
    textureVector[ModuleAssigment1_TextureList::NOISE]                      = moduleResources->createTextureFromFile(L"Assets/Textures/noise.jpg");
    textureVector[ModuleAssigment1_TextureList::UV_TEST_NO_MIPMAPS]         = moduleResources->createTextureFromFile(L"Assets/Textures/uvTest.png", false, false);
    textureVector[ModuleAssigment1_TextureList::UV_TEST]                    = moduleResources->createTextureFromFile(L"Assets/Textures/uvTest.png");
    textureVector[ModuleAssigment1_TextureList::METAL_GRATE_NO_MIPMAPS]     = moduleResources->createTextureFromFile(L"Assets/Textures/metalgrate.jpg", false, false);
    textureVector[ModuleAssigment1_TextureList::METAL_GRATE]                = moduleResources->createTextureFromFile(L"Assets/Textures/metalgrate.jpg");
    textureVector[ModuleAssigment1_TextureList::DOG]                        = moduleResources->createTextureFromFile(L"Assets/Textures/dog.dds");

    bool ok = true;
    for (int i = 0; i < ModuleAssigment1_TextureList::COUNT && ok; ++i) {
        ok = (textureVector[i] != nullptr);
    }

    return ok;
}

bool ModuleAssigment1::cleanUp() {
    return true;
}

void ModuleAssigment1::render() {
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

        ID3D12DescriptorHeap* descriptorHeaps[] = { srvHeap.Get(), moduleSamplers->getHeap() };
        cmd->SetDescriptorHeaps(2, descriptorHeaps);

        cmd->SetGraphicsRoot32BitConstants(
            0,
            sizeof(Matrix) / sizeof(UINT32),
            &mvp,
            0
        );

        //cmd->SetGraphicsRootDescriptorTable(1, srvGPUHandle);

        cmd->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

        UINT descriptorSize = moduleD3d12->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
        textureHandle.ptr += selectedTexture * descriptorSize;

        cmd->SetGraphicsRootDescriptorTable(1, textureHandle);
        cmd->SetGraphicsRootDescriptorTable(2, moduleSamplers->getGPUHandle(ModuleSamplers::Type(sampler)));


        cmd->DrawInstanced(6, 1, 0, 0);

        // ---- DebugDraw ----
        if (settings->sceneEditor.showGrid)
            dd::xzSquareGrid(-10.f, 10.f, 0.f, 1.f, dd::colors::LightGray);

        if (settings->sceneEditor.showAxis)
            dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

        debugDrawPass->record(cmd, width, height, view, proj);
        });
}


bool ModuleAssigment1::createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride) {
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


bool ModuleAssigment1::createRootSignature() {
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    CD3DX12_ROOT_PARAMETER rootParameters[3] = {};
    CD3DX12_DESCRIPTOR_RANGE srvRange, sampRange;

    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, ModuleSamplers::COUNT, 0);

    rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

    rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> rootSignatureBlob;

    if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, nullptr)))
    {
        return false;
    }

    if (FAILED(moduleD3d12->getDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))))
    {
        return false;
    }

    return true;
}


bool ModuleAssigment1::createPSO()
{
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = { {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                                              {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };

    auto dataVS = DX::ReadData(L"Assigment1VS.cso");
    auto dataPS = DX::ReadData(L"Assigment1PS.cso");

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

void ModuleAssigment1::drawGUI() {
    if (ImGui::Begin("Assigment 1 Controlls")) {
        if (ImGui::CollapsingHeader("Texture position")) {
            ImGui::DragFloat3("Position###TextPos", &textPos.x, 0.1f);
            ImGui::DragFloat3("Rotation###TextRot", &textRot.x, 0.5f);
            ImGui::DragFloat3("Scale###TextScale", &textScale.x, 0.1f);
        }
        if (ImGui::CollapsingHeader("Samplers")) {
            ImGui::Combo("Sampler", &sampler, "Linear/Wrap\0Point/Wrap\0Linear/Clamp\0Point/Clamp\0", ModuleSamplers::COUNT);
        }
        if (ImGui::CollapsingHeader("Textures")) {
            ImGui::Combo(
                "Texture",
                &selectedTexture,
                "CHECKERBOARD (no mipmaps)\0CHECKERBOARD\0THIN_GRID (no mipmaps)\0THIN_GRID\0SMALL_TEXT (no mipmaps)\0SMALL_TEXT\0NOISE (no mipmaps)\0NOISE\0UV_TEST (no mipmaps)\0UV_TEST\0METAL_GRATE (no mipmaps)\0METAL_GRATE\0DOG\0",
                ModuleAssigment1_TextureList::COUNT
            );
        }
    }
    ImGui::End();
}
