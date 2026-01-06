#include "Globals.h"
#include "ModuleUI.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleRender.h"
#include "ModuleShaderDescriptors.h"
#include "ImGuiPass.h"
#include "imgui.h"
#include "ImGuizmo.h"

bool ModuleUI::init() {
    moduleD3d12 = app->getModuleD3D12();
    auto* shaderDesc = app->getModuleShaderDescriptors();

    imguiPass = new ImGuiPass(moduleD3d12->getDevice(), moduleD3d12->getHWnd(), shaderDesc->getCPUHandle(0), shaderDesc->getGPUHandle(0));
    return true;
}

void ModuleUI::preRender() {
    imguiPass->startFrame();

    ImGuizmo::BeginFrame();

    for (auto& current : windows)
        current();

    ModuleRender* render = app->getModuleRender();
    render->registerUIPass(
        [this](ID3D12GraphicsCommandList* cmd) {
            imguiPass->record(cmd);
        }
    );
}

bool ModuleUI::cleanUp() {
    delete imguiPass;
    return true;
}

void ModuleUI::registerWindow(const WindowCallback& callback)
{
    windows.push_back(callback);
}
