#include "Globals.h"
#include "ModuleUI.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleRender.h"
#include "ImGuiPass.h"
#include "imgui.h"
#include "ImGuizmo.h"

bool ModuleUI::init() {
    moduleD3d12 = app->getModuleD3D12();
    imguiPass = new ImGuiPass(moduleD3d12->getDevice(), moduleD3d12->getHWnd());
    return true;
}

void ModuleUI::preRender() {
    imguiPass->startFrame();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, float(moduleD3d12->getWindowWidth()), float(moduleD3d12->getWindowHeight()));

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
