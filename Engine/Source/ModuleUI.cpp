#include "Globals.h"
#include "ModuleUI.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleRender.h"
#include "ImGuiPass.h"
#include "imgui.h"

bool ModuleUI::init()
{
    d3d12 = app->getModuleD3D12();
    imguiPass = new ImGuiPass(d3d12->getDevice(), d3d12->getHWnd());
    return true;
}

void ModuleUI::preRender()
{
    imguiPass->startFrame();

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
