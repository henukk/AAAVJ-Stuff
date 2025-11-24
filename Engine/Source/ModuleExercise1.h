#pragma once
#include "Module.h"

class ModuleD3D12;
class ModuleExercise1 : public Module {
private:
	ModuleD3D12* d3d12;
public:
	bool init() override;
	void render() override;
	bool cleanUp() override;
};
