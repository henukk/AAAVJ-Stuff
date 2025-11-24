#pragma once
#include "Module.h"

class ImGuiPass;
class ModuleD3D12;
class EditorConsole;
class EditorMenuBar;

class ModuleEditor : public Module {
private:
    ModuleD3D12* d3d12 = nullptr;
    ImGuiPass* imguiPass = nullptr;
    EditorConsole* console = nullptr;
    EditorMenuBar* menuBar = nullptr;

public:
    bool init() override;
    void preRender() override;
    void render() override;
    bool cleanUp() override;

private:
    void drawMenuBar();
    void drawDockSpace();
    void drawPanels();
    void logFrameInfo();
};
