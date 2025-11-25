#pragma once
#include "Module.h"

class ModuleD3D12;
class ModuleUI;

class ModuleExercise1 : public Module {
private:
    ModuleD3D12* d3d12 = nullptr;
    ModuleUI* ui = nullptr;

    float clearColor[4] = { 1.f, 0.f, 0.f, 1.f };

public:
    bool init() override;
    void render() override;

private:
    void drawWindow();
};
