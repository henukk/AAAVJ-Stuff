#pragma once

#include "Module.h"

class ImGuiPass;
class ModuleD3D12;

class ModuleEditor : public Module {
public:
    ModuleEditor();
    ~ModuleEditor();

    bool init() override;
    bool postInit() override;
    void update() override;
    void preRender() override;
    void render() override;
    void postRender() override;
    bool cleanUp() override;

private:
    ImGuiPass* imguiPass = nullptr;
    ModuleD3D12* d3d12 = nullptr;
};
