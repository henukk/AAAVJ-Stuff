#include "Globals.h"
#include "ModuleExercise2.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ReadData.h"

bool ModuleExercise2::init() {
	//get dependencies
	d3d12 = app->getModuleD3D12();
	moduleResources = app->getModuleResources();

	//create vertex buffer
	struct Vertex { float x, y, z; };

	Vertex vertices[3] = {
		{ -1.0f, -1.0f, 0.0f },
		{  0.0f,  1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f }
	};

	UINT vertexBufferSize = sizeof(vertices);

	vertexBuffer = moduleResources->createDefaultBuffer(vertices, vertexBufferSize, "Exercise2");

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;

	//empty root signatur
	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.Init(
		0, nullptr,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;

	D3D12SerializeRootSignature(
		&rootDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob
	);

	d3d12->getDevice()->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	);

	//create the pso
	auto dataVS = DX::ReadData(L"Exercise2VS.cso");
	auto dataPS = DX::ReadData(L"Exercise2PS.cso");

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	{ "MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
	  0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.VS = { dataVS.data(), dataVS.size() };
	psoDesc.PS = { dataPS.data(), dataPS.size() };
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

void ModuleExercise2::render()
{
	ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();

	// Pipeline
	commandList->SetPipelineState(pso.Get());
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	// Input Assembler
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// Draw
	commandList->DrawInstanced(3, 1, 0, 0);
}