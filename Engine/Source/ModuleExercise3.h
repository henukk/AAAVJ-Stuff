#pragma once
#include "Module.h"
#include "DebugDrawPass.h"

class ModuleD3D12;
class ModuleResources;

class ModuleExercise3 : public Module {
private:
	ModuleD3D12* d3d12;
	ModuleResources* moduleResources;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;

	std::unique_ptr<DebugDrawPass>  debugDrawPass;
	Matrix                      mvp;
public:
	bool init() override;
	void render() override;

private:
	bool createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride);
	bool createRootSignature();
	bool createPSO();
};

