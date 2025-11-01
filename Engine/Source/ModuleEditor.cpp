#include "Globals.h"
#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ImGuiPass.h"
#include "imgui.h"

ModuleEditor::ModuleEditor() {}
ModuleEditor::~ModuleEditor() {}

bool ModuleEditor::init() {
    return true;
}

bool ModuleEditor::postInit() {
    d3d12 = (ModuleD3D12*)app->GetModule<ModuleD3D12>();
    imguiPass = new ImGuiPass(d3d12->getDevice(), d3d12->getHWND());
    return true;
}

void ModuleEditor::update() {
    
}

void ModuleEditor::preRender() {
    imguiPass->startFrame();
    ImGui::ShowDemoWindow();
}

void ModuleEditor::render() {
}

void ModuleEditor::postRender() {
    ID3D12GraphicsCommandList* cmdList = d3d12->getCommandList();
    imguiPass->record(cmdList);
}

bool ModuleEditor::cleanUp() {
    delete imguiPass;
    imguiPass = nullptr;
    return true;
}
