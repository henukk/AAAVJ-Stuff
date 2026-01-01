#include "Globals.h"
#include "ModuleEditor.h"

#include "Application.h"

#include "Settings.h"

#include "ModuleInput.h"
#include "ModuleCamera.h"
#include "ModuleUI.h"

#include "EditorConsole.h"
#include "EditorMenuBar.h"
#include "EditorSettings.h"
#include "EditorPerformance.h"
#include "EditorAbout.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include "BasicModel.h"


EditorConsole* Console = nullptr;

ModuleEditor::ModuleEditor() : selectedGameObject(nullptr) {

}

bool ModuleEditor::init() {
    settings = app->getSettings();

	moduleInput = app->getModuleInput();
    moduleCamera = app->getModuleCamera();

    Console = new EditorConsole();
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
    delete Console;
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
        Console->draw("Console");

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

void ModuleEditor::render() {
    //Grid
    if (settings->sceneEditor.showGrid) dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
    
    //Axis
    if (settings->sceneEditor.showAxis) dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);

    //Guizmo
    if (selectedGameObject != nullptr) {
        ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
        bool shouldShowGizmo = settings->sceneEditor.showGuizmo;

        switch (currentSceneTool) {
        case ModuleEditor::SCENE_TOOL::MOVE:
            gizmoOperation = ImGuizmo::TRANSLATE;
            break;
        case ModuleEditor::SCENE_TOOL::ROTATE:
            gizmoOperation = ImGuizmo::ROTATE;
            break;
        case ModuleEditor::SCENE_TOOL::SCALE:
            gizmoOperation = ImGuizmo::SCALE;
            break;
        case ModuleEditor::SCENE_TOOL::TRANSFORM:
            gizmoOperation = ImGuizmo::UNIVERSAL;
            break;
        default:
            shouldShowGizmo = false;
            break;
        }

        Matrix gameObjectMatrix = selectedGameObject->getModelMatrix();
        if (shouldShowGizmo) {
            ImGuizmo::Manipulate((const float*)&moduleCamera->getView(), (const float*)&moduleCamera->getProjection(), gizmoOperation, ImGuizmo::LOCAL, (float*)&gameObjectMatrix);
        }

        if (ImGuizmo::IsUsing()) {
            selectedGameObject->setModelMatrix(gameObjectMatrix);
        }
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
