#pragma once
#include "Module.h"

class ModuleD3D12 : Module {
public:
	ModuleD3D12();
	~ModuleD3D12();

	bool init();
	void update();
	void preRender();
	void postRender();
	void render();
	bool cleanUp();
};
