#include "Globals.h"
#include "Exercise7.h"

#include "Application.h"

#include "ModuleD3D12.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleResources.h"
#include "ModuleSamplers.h"
#include "ModuleRingBuffer.h"

#include "ModuleUI.h"
#include "ModuleEditor.h"

#include "ReadData.h"

#include "DirectXTex.h"
#include <d3d12.h>
#include "d3dx12.h"

#include "ImGuizmo.h"

Exercise7::Exercise7() {

}

Exercise7::~Exercise7() {
	cleanUp();
}

bool Exercise7::init() {
	moduleD3d12 = app->getModuleD3D12();
	moduleRender = app->getModuleRender();
	moduleResources = app->getModuleResources();
	moduleCamera = app->getModuleCamera();
	moduleSamplers = app->getModuleSamplers();
	moduleShaderDescriptors = app->getModuleShaderDescriptors();
	moduleRingBuffer = app->getModuleRingBuffer();

	ModuleUI* moduleUI = app->getModuleUI();
	moduleUI->registerWindow([this]() { drawGUI(); });

	bool ok = createRootSignature();
	ok = ok && createPSO();
	ok = ok && loadModel();

	app->getModuleEditor()->setSelectedGameObject(&model);

	if (ok) {
		debugDrawPass = std::make_unique<DebugDrawPass>(moduleD3d12->getDevice(), moduleD3d12->getDrawCommandQueue());
	}

	return true;
}

bool Exercise7::cleanUp() {
	return true;
}

void Exercise7::render() {
	moduleRender->registerWorldPass([this](ID3D12GraphicsCommandList* commandList) {
		ImVec2 sceneSize = app->getModuleEditor()->getSceneSize();
		unsigned width = static_cast<unsigned>(sceneSize.x);
		unsigned height = static_cast<unsigned>(sceneSize.y);

		const Matrix& view = moduleCamera->getView();
		Matrix proj = moduleCamera->getProjection();

		Matrix mvp = model.getModelMatrix() * view * proj;
		mvp = mvp.Transpose();

		PerFrame perFrame;
		perFrame.L = light.L;
		perFrame.Lc = light.Lc;
		perFrame.Ac = light.Ac;
		perFrame.viewPos = moduleCamera->getPosition();
		perFrame.L.Normalize();

		commandList->SetPipelineState(pso.Get());
		commandList->SetGraphicsRootSignature(rootSignature.Get());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // set the primitive topology
		ID3D12DescriptorHeap* descriptorHeaps[] = { moduleShaderDescriptors->getHeap(), moduleSamplers->getHeap() };
		commandList->SetDescriptorHeaps(2, descriptorHeaps);
		commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

		PerFrame* perFramePtr = nullptr;
		auto perFrameGPU = moduleRingBuffer->allocBuffer(sizeof(PerFrame), (void**)&perFramePtr);
		*perFramePtr = perFrame;

		commandList->SetGraphicsRootConstantBufferView(1, perFrameGPU);
		commandList->SetGraphicsRootDescriptorTable(4, moduleSamplers->getGPUHandle(ModuleSamplers::LINEAR_WRAP));

		for (const BasicMesh& mesh : model.getMeshes()) {
			if (UINT(mesh.getMaterialIndex()) < model.getNumMaterials()) {
				const BasicMaterial& material = model.getMaterials()[mesh.getMaterialIndex()];

				PerInstance perInstance = { model.getModelMatrix().Transpose(), model.getNormalMatrix().Transpose(), material.getPBRPhongMaterial() };
				PerInstance* perInstancePtr = nullptr;
				auto perInstanceGPU = moduleRingBuffer->allocBuffer(sizeof(PerInstance), (void**)&perInstancePtr);
				*perInstancePtr = perInstance;

				commandList->SetGraphicsRootConstantBufferView(2, perInstanceGPU);
				commandList->SetGraphicsRootDescriptorTable(3, moduleShaderDescriptors->getGPUHandle(material.getTextureTable()));

				mesh.draw(commandList);
			}
		}


		debugDrawPass->record(commandList, width, height, view, proj);
		});
}

bool Exercise7::createRootSignature() {
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	CD3DX12_ROOT_PARAMETER rootParameters[5] = {};
	CD3DX12_DESCRIPTOR_RANGE tableRanges;
	CD3DX12_DESCRIPTOR_RANGE sampRange;

	tableRanges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, ModuleSamplers::COUNT, 0);

	rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsDescriptorTable(1, &tableRanges, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[4].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

	rootSignatureDesc.Init(5, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSignatureBlob;

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, nullptr))) {
		return false;
	}

	if (FAILED(moduleD3d12->getDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) {
		return false;
	}

	return true;
}

bool Exercise7::createPSO() {
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};


	auto dataVS = DX::ReadData(L"Exercise7VS.cso");
	auto dataPS = DX::ReadData(L"Exercise7PS.cso");

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

bool Exercise7::loadModel() {
	model = BasicModel();
	model.load("Assets/Models/Duck/duck.gltf", "Assets/Models/Duck/", BasicMaterial::Type::PBR_PHONG);

	transform.setScale(Vector3(0.01f));
	transform.setRotation(Vector3(0.f, -90.f, 0.f));
	transform.setPosition(Vector3(0.f, 0.f, 0.f));
	model.setTransform(&transform);

	return true;
}

void Exercise7::drawGUI() {
	Matrix objectMatrix = model.getModelMatrix();

	if (ImGui::Begin("Exericise 7 Controls")) {
		ImGui::Text("Model loaded %s with %d meshes and %d materials", model.getSrcFile().c_str(), model.getNumMeshes(), model.getNumMaterials());

		for (const BasicMesh& mesh : model.getMeshes()) {
			ImGui::Text("Mesh %s with %d vertices and %d triangles", mesh.getName().c_str(), mesh.getNumVertices(), mesh.getNumIndices() / 3);
		}

		ImGui::Separator();

#pragma region Transform
		Transform& t = transform;

		float mat[16];
		t.toImGuizmoMatrix(mat);

		float tr[3], rt[3], sc[3];
		ImGuizmo::DecomposeMatrixToComponents(mat, tr, rt, sc);

		bool changed = false;
		changed |= ImGui::DragFloat3("Position", tr, 0.1f);
		changed |= ImGui::DragFloat3("Rotation", rt, 0.1f);
		changed |= ImGui::DragFloat3("Scale", sc, 0.1f);

		if (changed) {
			ImGuizmo::RecomposeMatrixFromComponents(tr, rt, sc, mat);
			t.fromImGuizmoMatrix(mat);
		}
#pragma endregion

		if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat3("Light Direction", reinterpret_cast<float*>(&light.L), 0.1f, -1.0f, 1.0f);
			ImGui::SameLine();
			if (ImGui::SmallButton("Normalize")) {
				light.L.Normalize();
			}
			ImGui::ColorEdit3("Light Colour", reinterpret_cast<float*>(&light.Lc), ImGuiColorEditFlags_NoAlpha);
			ImGui::ColorEdit3("Ambient Colour", reinterpret_cast<float*>(&light.Ac), ImGuiColorEditFlags_NoAlpha);
		}

		for (BasicMaterial& material : model.getMaterials()) {
			if (material.getMaterialType() == BasicMaterial::PBR_PHONG) {
				char tmp[256];
				_snprintf_s(tmp, 255, "Material %s", material.getName());

				if (ImGui::CollapsingHeader(tmp, ImGuiTreeNodeFlags_DefaultOpen)) {
					PBRPhongMaterialData pbr = material.getPBRPhongMaterial();
					if (ImGui::ColorEdit3("Diffuse Colour", reinterpret_cast<float*>(&pbr.diffuseColour))) {
						material.setPBRPhongMaterial(pbr);
					}

					bool hasTexture = pbr.hasDiffuseTex;
					if (ImGui::Checkbox("Use Texture", &hasTexture)) {
						pbr.hasDiffuseTex = hasTexture;
						material.setPBRPhongMaterial(pbr);
					}

					if (ImGui::ColorEdit3("Specular Colour", reinterpret_cast<float*>(&pbr.specularColour))) {
						material.setPBRPhongMaterial(pbr);
					}

					if (ImGui::DragFloat("shininess", &pbr.shininess)) {
						material.setPBRPhongMaterial(pbr);
					}
				}
			}
		}
	}
	ImGui::End();
}

