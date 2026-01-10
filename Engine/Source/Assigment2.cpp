#include "Globals.h"
#include "Assigment2.h"

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

Assigment2::Assigment2() {

}

Assigment2::~Assigment2() {
	cleanUp();
}

bool Assigment2::init() {
	moduleD3d12 = app->getModuleD3D12();
	moduleRender = app->getModuleRender();
	moduleResources = app->getModuleResources();
	moduleCamera = app->getModuleCamera();
	moduleSamplers = app->getModuleSamplers();
	moduleShaderDescriptors = app->getModuleShaderDescriptors();
	moduleRingBuffer = app->getModuleRingBuffer();

	ModuleUI* moduleUI = app->getModuleUI();
	moduleUI->registerWindow([this]() { drawGUI(); });

	bool ok = createRootSignatureBasic();
	ok = ok && createRootSignatureLit();
	ok = ok && createPSOBasic();
	ok = ok && createPSOPhong();
	ok = ok && createPSOPbr();
	ok = ok && loadModels();

	app->getModuleEditor()->setSelectedGameObject(&getActiveModelState().model);

	if (ok) {
		debugDrawPass = std::make_unique<DebugDrawPass>(moduleD3d12->getDevice(), moduleD3d12->getDrawCommandQueue());
	}

	return true;
}

bool Assigment2::cleanUp() {
	return true;
}

void Assigment2::render() {
	moduleRender->registerWorldPass([this](ID3D12GraphicsCommandList* commandList) {
		ImVec2 sceneSize = app->getModuleEditor()->getSceneSize();
		unsigned width = static_cast<unsigned>(sceneSize.x);
		unsigned height = static_cast<unsigned>(sceneSize.y);

		const Matrix& view = moduleCamera->getView();
		Matrix proj = moduleCamera->getProjection();

		ModelState& activeModel = getActiveModelState();
		Matrix mvp = activeModel.model.getModelMatrix() * view * proj;
		mvp = mvp.Transpose();

		PerFrame perFrame;
		perFrame.L = light.L;
		perFrame.Lc = light.Lc;
		perFrame.Ac = light.Ac;
		perFrame.viewPos = moduleCamera->getPosition();
		perFrame.L.Normalize();

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // set the primitive topology
		ID3D12DescriptorHeap* descriptorHeaps[] = { moduleShaderDescriptors->getHeap(), moduleSamplers->getHeap() };
		commandList->SetDescriptorHeaps(2, descriptorHeaps);

		switch (selectedModel) {
		case ModelType::BASIC: {
			commandList->SetPipelineState(psoBasic.Get());
			commandList->SetGraphicsRootSignature(rootSignatureBasic.Get());
			commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);
			commandList->SetGraphicsRootDescriptorTable(3, moduleSamplers->getGPUHandle(ModuleSamplers::LINEAR_WRAP));

			for (const BasicMesh& mesh : activeModel.model.getMeshes()) {
				if (UINT(mesh.getMaterialIndex()) < activeModel.model.getNumMaterials()) {
					const BasicMaterial& material = activeModel.model.getMaterials()[mesh.getMaterialIndex()];

					commandList->SetGraphicsRootConstantBufferView(1, activeModel.materialBuffers[mesh.getMaterialIndex()]->GetGPUVirtualAddress());
					commandList->SetGraphicsRootDescriptorTable(2, moduleShaderDescriptors->getGPUHandle(material.getTextureTable()));

					mesh.draw(commandList);
				}
			}
			break;
		}
		case ModelType::PHONG:
		case ModelType::PBR_PHONG: {
			commandList->SetPipelineState(selectedModel == ModelType::PHONG ? psoPhong.Get() : psoPbr.Get());
			commandList->SetGraphicsRootSignature(rootSignatureLit.Get());
			commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

			PerFrame* perFramePtr = nullptr;
			auto perFrameGPU = moduleRingBuffer->allocBuffer(sizeof(PerFrame), (void**)&perFramePtr);
			*perFramePtr = perFrame;

			commandList->SetGraphicsRootConstantBufferView(1, perFrameGPU);
			commandList->SetGraphicsRootDescriptorTable(4, moduleSamplers->getGPUHandle(ModuleSamplers::LINEAR_WRAP));

			for (const BasicMesh& mesh : activeModel.model.getMeshes()) {
				if (UINT(mesh.getMaterialIndex()) < activeModel.model.getNumMaterials()) {
					const BasicMaterial& material = activeModel.model.getMaterials()[mesh.getMaterialIndex()];

					if (selectedModel == ModelType::PHONG) {
						PerInstancePhong perInstance = { activeModel.model.getModelMatrix().Transpose(), activeModel.model.getNormalMatrix().Transpose(), material.getPhongMaterial() };
						PerInstancePhong* perInstancePtr = nullptr;
						auto perInstanceGPU = moduleRingBuffer->allocBuffer(sizeof(PerInstancePhong), (void**)&perInstancePtr);
						*perInstancePtr = perInstance;

						commandList->SetGraphicsRootConstantBufferView(2, perInstanceGPU);
					}
					else {
						PerInstancePbr perInstance = { activeModel.model.getModelMatrix().Transpose(), activeModel.model.getNormalMatrix().Transpose(), material.getPBRPhongMaterial() };
						PerInstancePbr* perInstancePtr = nullptr;
						auto perInstanceGPU = moduleRingBuffer->allocBuffer(sizeof(PerInstancePbr), (void**)&perInstancePtr);
						*perInstancePtr = perInstance;

						commandList->SetGraphicsRootConstantBufferView(2, perInstanceGPU);
					}

					commandList->SetGraphicsRootDescriptorTable(3, moduleShaderDescriptors->getGPUHandle(material.getTextureTable()));

					mesh.draw(commandList);
				}
			}
			break;
		}
		default:
			break;
		}


		debugDrawPass->record(commandList, width, height, view, proj);
		});
}

bool Assigment2::createRootSignatureBasic() {
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

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, nullptr))) {
		return false;
	}

	if (FAILED(moduleD3d12->getDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatureBasic)))) {
		return false;
	}

	return true;
}

bool Assigment2::createRootSignatureLit() {
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

	if (FAILED(moduleD3d12->getDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatureLit)))) {
		return false;
	}

	return true;
}

bool Assigment2::createPSOBasic() {
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	auto dataVS = DX::ReadData(L"Exercise5VS.cso");
	auto dataPS = DX::ReadData(L"Exercise5PS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };  // the structure describing our input layout
	psoDesc.pRootSignature = rootSignatureBasic.Get();                                                   // the root signature that describes the input data this pso needs
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
	return SUCCEEDED(moduleD3d12->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoBasic)));
}

bool Assigment2::createPSOPhong() {
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};


	auto dataVS = DX::ReadData(L"Exercise6VS.cso");
	auto dataPS = DX::ReadData(L"Exercise6PS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };  // the structure describing our input layout
	psoDesc.pRootSignature = rootSignatureLit.Get();                                                   // the root signature that describes the input data this pso needs
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
	return SUCCEEDED(moduleD3d12->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoPhong)));
}

bool Assigment2::createPSOPbr() {
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};


	auto dataVS = DX::ReadData(L"Exercise7VS.cso");
	auto dataPS = DX::ReadData(L"Exercise7PS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };  // the structure describing our input layout
	psoDesc.pRootSignature = rootSignatureLit.Get();                                                   // the root signature that describes the input data this pso needs
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
	return SUCCEEDED(moduleD3d12->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoPbr)));
}

bool Assigment2::loadModels() {
	models[ModelType::BASIC].model = BasicModel();
	models[ModelType::BASIC].model.load("Assets/Models/Duck/duck.gltf", "Assets/Models/Duck/", BasicMaterial::Type::BASIC);

	models[ModelType::PHONG].model = BasicModel();
	models[ModelType::PHONG].model.load("Assets/Models/Duck/duck.gltf", "Assets/Models/Duck/", BasicMaterial::Type::PHONG);

	models[ModelType::PBR_PHONG].model = BasicModel();
	models[ModelType::PBR_PHONG].model.load("Assets/Models/Duck/duck.gltf", "Assets/Models/Duck/", BasicMaterial::Type::PBR_PHONG);

	sharedTransform.setScale(Vector3(0.01f));
	sharedTransform.setRotation(Vector3(0.f, -90.f, 0.f));
	sharedTransform.setPosition(Vector3(0.f, 0.f, 0.f));

	models[ModelType::BASIC].model.setTransform(&sharedTransform);
	models[ModelType::PHONG].model.setTransform(&sharedTransform);
	models[ModelType::PBR_PHONG].model.setTransform(&sharedTransform);

	ModelState& basicModel = models[ModelType::BASIC];
	for (int i = 0, count = basicModel.model.getNumMaterials(); i < count; ++i) {
		const BasicMaterial& material = basicModel.model.getMaterials()[i];
		const BasicMaterialData& data = material.getBasicMaterial();

		basicModel.materialBuffers.push_back(moduleResources->createDefaultBuffer(&data, alignUp(sizeof(BasicMaterialData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), material.getName()));
	}

	return true;
}

Assigment2::ModelState& Assigment2::getActiveModelState() {
	return models[selectedModel];
}

void Assigment2::drawGUI() {
	ModelState& activeModel = getActiveModelState();

	if (ImGui::Begin("Assigment 2 Controls")) {
		int selection = selectedModel;
		if (ImGui::Combo("Model", &selection, "Basic\0Phong\0PBR Phong\0")) {
			selectedModel = static_cast<ModelType>(selection);
			app->getModuleEditor()->setSelectedGameObject(&getActiveModelState().model);
		}

		ImGui::Text("Model loaded %s with %d meshes and %d materials", activeModel.model.getSrcFile().c_str(), activeModel.model.getNumMeshes(), activeModel.model.getNumMaterials());

		for (const BasicMesh& mesh : activeModel.model.getMeshes()) {
			ImGui::Text("Mesh %s with %d vertices and %d triangles", mesh.getName().c_str(), mesh.getNumVertices(), mesh.getNumIndices() / 3);
		}

		ImGui::Separator();

		Transform& t = sharedTransform;
		bool transform_changed = false;
		transform_changed |= ImGui::DragFloat3("Position", (float*)&t.getPosition(), 0.1f);
		transform_changed |= ImGui::DragFloat3("Rotation", (float*)&t.getRotation(), 0.1f);
		transform_changed |= ImGui::DragFloat3("Scale", (float*)&t.getScale(), 0.1f);

		if (selectedModel != ModelType::BASIC) {
			if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::DragFloat3("Light Direction", reinterpret_cast<float*>(&light.L), 0.1f, -1.0f, 1.0f);
				ImGui::SameLine();
				if (ImGui::SmallButton("Normalize")) {
					light.L.Normalize();
				}
				ImGui::ColorEdit3("Light Colour", reinterpret_cast<float*>(&light.Lc), ImGuiColorEditFlags_NoAlpha);
				ImGui::ColorEdit3("Ambient Colour", reinterpret_cast<float*>(&light.Ac), ImGuiColorEditFlags_NoAlpha);
			}
		}

		for (BasicMaterial& material : activeModel.model.getMaterials()) {
			if (selectedModel == ModelType::PHONG && material.getMaterialType() == BasicMaterial::PHONG) {
				char tmp[256];
				_snprintf_s(tmp, 255, "Material %s", material.getName());

				if (ImGui::CollapsingHeader(tmp, ImGuiTreeNodeFlags_DefaultOpen)) {
					PhongMaterialData phong = material.getPhongMaterial();
					if (ImGui::ColorEdit3("Diffuse Colour", reinterpret_cast<float*>(&phong.diffuseColour))) {
						material.setPhongMaterial(phong);
					}

					bool hasTexture = phong.hasDiffuseTex;
					if (ImGui::Checkbox("Use Texture", &hasTexture)) {
						phong.hasDiffuseTex = hasTexture;
						material.setPhongMaterial(phong);
					}

					if (ImGui::DragFloat("Kd", &phong.Kd, 0.01f)) {
						material.setPhongMaterial(phong);
					}

					if (ImGui::DragFloat("Ks", &phong.Ks, 0.01f)) {
						material.setPhongMaterial(phong);
					}

					if (ImGui::DragFloat("shininess", &phong.shininess)) {
						material.setPhongMaterial(phong);
					}
				}
			}

			if (selectedModel == ModelType::PBR_PHONG && material.getMaterialType() == BasicMaterial::PBR_PHONG) {
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