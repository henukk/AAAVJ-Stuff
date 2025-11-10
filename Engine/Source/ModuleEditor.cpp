#include "Globals.h"
#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ImGuiPass.h"
#include "imgui.h"

#include "EditorConsole.h"
#include "EditorMenuBar.h"

ModuleEditor::ModuleEditor() {}
ModuleEditor::~ModuleEditor() {}

bool ModuleEditor::init() {
    d3d12 = app->getModuleD3D12();
    imguiPass = new ImGuiPass(d3d12->getDevice(), d3d12->getHWND());
    console = new EditorConsole();
    menuBar = new EditorMenuBar();
    return true;
}

bool ModuleEditor::postInit() {
    return true;
}

void ModuleEditor::update() {}

void ModuleEditor::preRender() {
    imguiPass->startFrame();

    drawMenuBar();
    drawDockSpace();
    drawPanels();
    logFrameInfo();
}

void ModuleEditor::render() {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = d3d12->getRtvHandle();

    d3d12->getCommandList()->OMSetRenderTargets(1, &handle, FALSE, nullptr);

    ID3D12GraphicsCommandList* cmdList = d3d12->getCommandList();
    imguiPass->record(cmdList);

}

void ModuleEditor::postRender() {
}

bool ModuleEditor::cleanUp() {
    delete imguiPass;
    delete console;
    delete menuBar;

    imguiPass = nullptr;
    console = nullptr;
    menuBar = nullptr;

    return true;
}

void ModuleEditor::drawMenuBar() {
    if (menuBar)
        menuBar->draw();
}

void ModuleEditor::drawDockSpace() {
    ImGuiIO& io = ImGui::GetIO();
    static bool dockspaceOpen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpaceWindow", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar(3);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
}

void ModuleEditor::drawPanels() {
    if (menuBar->isConsoleVisible())
        console->draw("Console");

    if (menuBar->isDemoVisible())
        ImGui::ShowDemoWindow();
}

void ModuleEditor::logFrameInfo() {
    //console->PrintWarn("This is a warning about missing asset.");
    //console->PrintError("Failed to load shader: %s", "skybox.hlsl");
    //console->PrintLog("FPS: %.2f", app->getFPS());
}
