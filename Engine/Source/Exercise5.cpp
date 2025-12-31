#include "Globals.h"
#include "Exercise5.h"

#include "Application.h"

#include "ModuleD3D12.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleResources.h"
#include "ModuleSamplers.h"

#include "ModuleUI.h"

#include "BasicModel.h"
#include "BasicMesh.h"

#include "ReadData.h"

#include "DirectXTex.h"
#include <d3d12.h>
#include "d3dx12.h"

Exercise5::Exercise5() {

}

Exercise5::~Exercise5() {
	cleanUp();
}

bool Exercise5::init() {
    moduleD3d12 = app->getModuleD3D12();
    moduleRender = app->getModuleRender();
    moduleResources = app->getModuleResources();
	moduleCamera = app->getModuleCamera();
	moduleSamplers = app->getModuleSamplers();
	moduleShaderDescriptors = app->getModuleShaderDescriptors();

    ModuleUI* moduleUI = app->getModuleUI();
    moduleUI->registerWindow([this]() { drawGUI(); });

    bool ok = createRootSignature();
    ok = ok && createPSO();
    ok = ok && loadModel();

    if (ok) {
        debugDrawPass = std::make_unique<DebugDrawPass>(moduleD3d12->getDevice(), moduleD3d12->getDrawCommandQueue());
    }

    return true;
}

bool Exercise5::cleanUp() {
    return true;
}

void Exercise5::render() {
    moduleRender->registerWorldPass([this](ID3D12GraphicsCommandList* commandList) {
        unsigned width = moduleD3d12->getWindowWidth();
        unsigned height = moduleD3d12->getWindowHeight();

        const Matrix& view = moduleCamera->getView();
        Matrix proj = moduleCamera->getProjection();

        Matrix mvp = model->getModelMatrix() * view * proj;
        mvp = mvp.Transpose();

        commandList->SetPipelineState(pso.Get());
        commandList->SetGraphicsRootSignature(rootSignature.Get());
        
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // set the primitive topology
        ID3D12DescriptorHeap* descriptorHeaps[] = { moduleShaderDescriptors->getHeap(), moduleSamplers->getHeap() };
        commandList->SetDescriptorHeaps(2, descriptorHeaps);
        commandList->SetGraphicsRootDescriptorTable(3, moduleSamplers->getGPUHandle(ModuleSamplers::LINEAR_WRAP));
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

        BEGIN_EVENT(commandList, "Model Render Pass");

        for (const BasicMesh& mesh : model->getMeshes()) {
            if (UINT(mesh.getMaterialIndex()) < model->getNumMaterials()) {
                const BasicMaterial& material = model->getMaterials()[mesh.getMaterialIndex()];

                commandList->SetGraphicsRootConstantBufferView(1, materialBuffers[mesh.getMaterialIndex()]->GetGPUVirtualAddress());
                commandList->SetGraphicsRootDescriptorTable(2, moduleShaderDescriptors->getGPUHandle(material.getTextureTable()));

                mesh.draw(commandList);
            }
        }

        END_EVENT(commandList);

        if (showGrid) dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
        if (showAxis) dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

        Matrix objectMatrix = model->getModelMatrix();
        if (showGuizmo) {
            ImGuizmo::Manipulate((const float*)&view, (const float*)&proj, gizmoOperation, ImGuizmo::LOCAL, (float*)&objectMatrix);
        }

        if (ImGuizmo::IsUsing()) {
            model->setModelMatrix(objectMatrix);
        }

        debugDrawPass->record(commandList, width, height, view, proj);
    });
}

bool Exercise5::createRootSignature() {
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    CD3DX12_ROOT_PARAMETER rootParameters[4] = {};
    CD3DX12_DESCRIPTOR_RANGE tableRanges;
    CD3DX12_DESCRIPTOR_RANGE sampRange;

    tableRanges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, ModuleSamplers::COUNT, 0);

    rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsDescriptorTable(1, &tableRanges, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[3].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

    rootSignatureDesc.Init(4, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool Exercise5::loadModel() {
    model = std::make_unique<BasicModel>();
    //model->load("Assets/Models/BoxInterleaved/BoxInterleaved.gltf", "Assets/Models/BoxInterleaved/");
    //model->load("Assets/Models/BoxTextured/BoxTextured.gltf", "Assets/Models/BoxTextured/");
    model->load("Assets/Models/Duck/duck.gltf", "Assets/Models/Duck/");
    model->setModelMatrix(Matrix::CreateScale(0.01f, 0.01f, 0.01f));


    for (int i = 0, count = model->getNumMaterials(); i < count; ++i)
    {
        const BasicMaterial& material = model->getMaterials()[i];
        const BasicMaterialData& data = material.getBasicMaterial();

        materialBuffers.push_back(moduleResources->createDefaultBuffer(&data, alignUp(sizeof(BasicMaterialData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), material.getName()));
    }

    return true;
}

bool Exercise5::createPSO() {
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = { 
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    auto dataVS = DX::ReadData(L"Exercise5VS.cso");
    auto dataPS = DX::ReadData(L"Exercise5PS.cso");

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
    psoDesc.RasterizerState.FrontCounterClockwise = TRUE;                                           // our models are counter clock wise
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);                                         // a default blend state.
    psoDesc.NumRenderTargets = 1;                                                                   // we are only binding one render target

    // create the pso
    return SUCCEEDED(moduleD3d12->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void Exercise5::drawGUI() {
    Matrix objectMatrix = model->getModelMatrix();

    if (ImGui::Begin("Geometry Viewer Options")) {
        ImGui::Checkbox("Show grid", &showGrid);
        ImGui::Checkbox("Show axis", &showAxis);
        ImGui::Checkbox("Show guizmo", &showGuizmo);
        ImGui::Text("Model loaded %s with %d meshes and %d materials", model->getSrcFile().c_str(), model->getNumMeshes(), model->getNumMaterials());

        for (const BasicMesh& mesh : model->getMeshes())
        {
            ImGui::Text("Mesh %s with %d vertices and %d triangles", mesh.getName().c_str(), mesh.getNumVertices(), mesh.getNumIndices() / 3);
        }

        ImGui::Separator();
        if (ImGui::IsKeyPressed(ImGuiKey_T)) gizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) gizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_S)) gizmoOperation = ImGuizmo::SCALE;

        ImGui::RadioButton("Translate", (int*)&gizmoOperation, (int)ImGuizmo::TRANSLATE);
        ImGui::SameLine();
        ImGui::RadioButton("Rotate", (int*)&gizmoOperation, ImGuizmo::ROTATE);
        ImGui::SameLine();
        ImGui::RadioButton("Scale", (int*)&gizmoOperation, ImGuizmo::SCALE);

        float translation[3], rotation[3], scale[3];
        ImGuizmo::DecomposeMatrixToComponents((float*)&objectMatrix, translation, rotation, scale);
        bool transform_changed = ImGui::DragFloat3("Tr", translation, 0.1f);
        transform_changed = transform_changed || ImGui::DragFloat3("Rt", rotation, 0.1f);
        transform_changed = transform_changed || ImGui::DragFloat3("Sc", scale, 0.1f);

        if (transform_changed) {
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, (float*)&objectMatrix);

            model->setModelMatrix(objectMatrix);
        }
    }
    ImGui::End();
}

