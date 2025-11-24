#pragma once
#include "Module.h"

class ModuleD3D12;
class ModuleResources;

class ModuleExercise2 : public Module {
private:
	ModuleD3D12* d3d12 = nullptr;
	ModuleResources* moduleResources = nullptr;
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
