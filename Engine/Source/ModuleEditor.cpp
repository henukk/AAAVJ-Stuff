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
#include "EditorSettings.h"
#include "EditorPerformance.h"
#include "EditorAbout.h"

bool ModuleEditor::init() {
	moduleInput = app->getModuleInput();

    console = new EditorConsole();
    menuBar = new EditorMenuBar();
    editorSettings = new EditorSettings();
    editorPerformance = new EditorPerformance();
	editorAbout = new EditorAbout(*(menuBar->getAboutVisible()));

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
    delete editorSettings;

    return true;
}

void ModuleEditor::drawDockSpace() {
    static bool open = true;

    ImGuiID dockID = ImGui::GetID("MainDockspace");
    ImGui::DockSpaceOverViewport(dockID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void ModuleEditor::drawPanels() {
    if (menuBar->isConsoleVisible())
        console->draw("Console");

    if (menuBar->isSettingsVisible())
        editorSettings->draw("Settings");

    if (menuBar->isPerformanceVisible())
        editorPerformance->draw("Performance");

    if (menuBar->isAboutVisible())
		editorAbout->draw("About");
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
    }
    else if (mouseState.leftButton && (keyboardState.LeftAlt || keyboardState.RightAlt)) {
        setMode(NAVIGATION, ORBIT);
    } else if (mouseState.middleButton) {
        setMode(NAVIGATION, PAN);
    } else if (!mouseState.leftButton) {
        resetMode();
        handleQWERTYCases(keyboardState);
    }
}

void ModuleEditor::handleQWERTYCases(Keyboard::State keyboardState) {
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
