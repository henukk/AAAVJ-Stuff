#pragma once
#include "Module.h"

class EditorConsole;
class EditorMenuBar;

class ModuleEditor : public Module
{
    EditorConsole* console = nullptr;
    EditorMenuBar* menuBar = nullptr;

public:
    bool init() override;
    bool cleanUp() override;

private:
    void drawDockSpace();
    void drawPanels();
};
