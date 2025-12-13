#include "Globals.h"
#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleUI.h"
#include "imgui.h"

#include "ModuleInput.h"
#include "Keyboard.h"
#include "Mouse.h"

#include "EditorConsole.h"
#include "EditorMenuBar.h"

bool ModuleEditor::init() {
	moduleInput = app->getModuleInput();

    console = new EditorConsole();
    menuBar = new EditorMenuBar();

    ModuleUI* ui = app->getModuleUI();

    ui->registerWindow([this]() { menuBar->draw(); });
    ui->registerWindow([this]() { drawDockSpace(); });
    ui->registerWindow([this]() { drawPanels(); });

	//Initialize
    currentSceneTool = NAVIGATION;
	previousSceneTool = NONE;
    currentNavigationMode = PAN;

    return true;
}

bool ModuleEditor::cleanUp() {
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


void ModuleEditor::update() {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard && !io.WantCaptureMouse) {
        handleKeyboardShortcuts();
    }
}

void ModuleEditor::handleKeyboardShortcuts() {
    Keyboard::State keyboardState = Keyboard::Get().GetState();
	Mouse::State mouseState = Mouse::Get().GetState();
    
    DirectX::Mouse::ButtonStateTracker buttonStateTracker;
    buttonStateTracker.Update(mouseState);

    if (mouseState.rightButton) {
		setMode(NAVIGATION, FREE_LOOK);
    } else if (buttonStateTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED && (keyboardState.LeftAlt || keyboardState.RightAlt)) {
        setMode(NAVIGATION, PAN);
    } else if (!mouseState.leftButton) {
        resetMode();
        handleQWERTYCases(keyboardState, mouseState);
    }
}

void ModuleEditor::handleQWERTYCases(Keyboard::State keyboardState, Mouse::State mouseState) {
    if (keyboardState.IsKeyDown(Keyboard::Keys::Q)) {
        currentSceneTool = NAVIGATION;
        currentNavigationMode = PAN;
    }
    if (keyboardState.IsKeyDown(Keyboard::Keys::W)) currentSceneTool = MOVE;
    if (keyboardState.IsKeyDown(Keyboard::Keys::E)) currentSceneTool = ROTATE;
    if (keyboardState.IsKeyDown(Keyboard::Keys::R)) currentSceneTool = SCALE;
    if (keyboardState.IsKeyDown(Keyboard::Keys::T)) currentSceneTool = RECT;
    if (keyboardState.IsKeyDown(Keyboard::Keys::Y)) currentSceneTool = TRANSFORM;
}

void ModuleEditor::setMode(SCENE_TOOL sceneTool, NAVIGATION_MODE navigationMode) {
    if (previousSceneTool == NONE) {
        previousSceneTool = currentSceneTool;
        currentSceneTool = sceneTool;
        currentNavigationMode = navigationMode;
    }
}

void ModuleEditor::resetMode() {
    if (previousSceneTool != NONE) {
        currentSceneTool = previousSceneTool;
        previousSceneTool = NONE;
    }
    currentNavigationMode = PAN;
}
