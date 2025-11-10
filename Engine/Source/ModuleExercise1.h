#pragma once
#include "Module.h"

class ModuleD3D12;
class ModuleExercise1 : public Module {
private:
	ModuleD3D12* d3d12;
public:
	ModuleExercise1();	~ModuleExercise1();
	bool init();
	bool postInit();
	void update();	void preRender();	void postRender();	void render();	bool cleanUp();
};

