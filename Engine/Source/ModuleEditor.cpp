#include "Globals.h"
#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleUI.h"
#include "imgui.h"

#include "EditorConsole.h"
#include "EditorMenuBar.h"

bool ModuleEditor::init()
{
    console = new EditorConsole();
    menuBar = new EditorMenuBar();

    ModuleUI* ui = app->getModuleUI();

    ui->registerWindow([this]() { menuBar->draw(); });
    ui->registerWindow([this]() { drawDockSpace(); });
    ui->registerWindow([this]() { drawPanels(); });

    return true;
}

bool ModuleEditor::cleanUp()
{
    delete console;
    delete menuBar;
    return true;
}

void ModuleEditor::drawDockSpace()
{
    ImGuiIO& io = ImGui::GetIO();
    static bool open = true;

    ImGuiID dockID = ImGui::GetID("MainDockspace");
    ImGui::DockSpaceOverViewport(dockID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void ModuleEditor::drawPanels()
{
    if (menuBar->isConsoleVisible())
        console->draw("Console");

    if (menuBar->isDemoVisible())
        ImGui::ShowDemoWindow();
}
