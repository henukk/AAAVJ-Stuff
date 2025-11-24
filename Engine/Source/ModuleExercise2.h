#pragma once
#include "Module.h"

class ModuleD3D12;
class ModuleResources;

class ModuleExercise2 : public Module {
private:
	ModuleD3D12* d3d12 = nullptr;
	ModuleResources* moduleResources = nullptr;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;
public:
	ModuleExercise2();
	~ModuleExercise2();
	bool init();
	bool postInit();
	void update();
	void preRender();
	void postRender();
	void render();
	bool cleanUp();
};
