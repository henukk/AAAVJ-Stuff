#pragma once
#include "Module.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public Module {
private:
	ComPtr<IDXGIFactory6> factory;

public:
	ModuleD3D12();
	~ModuleD3D12();

	bool init() override;
	void update() override;
	void preRender() override;
	void postRender() override;
	void render() override;
	bool cleanUp() override;
};
