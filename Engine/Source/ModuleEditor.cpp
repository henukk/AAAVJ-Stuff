#include "Globals.h"
#include "ModuleEditor.h"

#include "Application.h"

#include "Settings.h"

#include "ModuleInput.h"
#include "ModuleCamera.h"
#include "ModuleUI.h"

#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "ModuleRTDescriptors.h"
#include "ModuleDSDescriptors.h"
#include "ModuleShaderDescriptors.h"

#include "EditorConsole.h"
#include "EditorMenuBar.h"
#include "EditorSettings.h"
#include "EditorPerformance.h"
#include "EditorAbout.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include "BasicModel.h"
#include "Transform.h"

EditorConsole* Console = nullptr;

ModuleEditor::ModuleEditor() : selectedGameObject(nullptr) {

}

bool ModuleEditor::init() {
    settings = app->getSettings();

	moduleInput = app->getModuleInput();
    moduleCamera = app->getModuleCamera();


    moduleD3d12 = app->getModuleD3D12();
    moduleResources = app->getModuleResources();
    moduleRTDescriptors = app->getModuleRTDescriptors();
    moduleDSDescriptors = app->getModuleDSDescriptors();
    moduleShaderDescriptors = app->getModuleShaderDescriptors();

    Console = new EditorConsole();
    menuBar = new EditorMenuBar();
    editorSettings = new EditorSettings();
    editorPerformance = new EditorPerformance();
	editorAbout = new EditorAbout(*(menuBar->getAboutVisible()));


    ModuleUI* ui = app->getModuleUI();


    ui->registerWindow([this]() { menuBar->draw(); });
    ui->registerWindow([this]() { drawDockSpace(); });
    ui->registerWindow([this]() { drawPanels(); }); 

    ui->registerWindow([this]() { drawSceneWindow(); });

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
    bool allowCameraInput = isSceneHovered() && !ImGuizmo::IsUsing();
    if (allowCameraInput) {
        handleKeyboardShortcuts();
    }
}

void ModuleEditor::render() {
    //Grid
    if (settings->sceneEditor.showGrid) dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
    
    //Axis
    if (settings->sceneEditor.showAxis) dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);
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

void ModuleEditor::drawSceneWindow()
{
    ImGui::Begin("Scene");
    sceneHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    sceneSize = ImGui::GetContentRegionAvail();

    if ((int)sceneSize.x != (int)prevSceneSize.x || (int)sceneSize.y != (int)prevSceneSize.y) {
        prevSceneSize = sceneSize;
        recreateSceneRenderTarget();
    }

    ImVec2 sceneMin = ImGui::GetCursorScreenPos();
    sceneScreenMin = sceneMin;

    if (sceneSRV != UINT(-1) && sceneSize.x > 0 && sceneSize.y > 0) {
        ImGui::Image((ImTextureID)getSceneSRV().ptr, sceneSize, ImVec2(0, 0), ImVec2(1, 1));
    }

    // --- ImGuizmo ---
    if (selectedGameObject) {
        ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
		bool shouldShowGizmo = settings->sceneEditor.showGuizmo;

        switch (currentSceneTool) {
            case MOVE:          op = ImGuizmo::TRANSLATE; break;
            case ROTATE:        op = ImGuizmo::ROTATE; break;
            case SCALE:         op = ImGuizmo::SCALE; break;
            case TRANSFORM:     op = ImGuizmo::UNIVERSAL; break;
            default: shouldShowGizmo = false; break;
        }

        if (shouldShowGizmo) {
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(sceneMin.x, sceneMin.y, sceneSize.x, sceneSize.y);

            Transform* t = selectedGameObject->getTransform();
            Matrix m = t->toMatrix();
            ImGuizmo::Manipulate((float*)&moduleCamera->getView(), (float*)&moduleCamera->getProjection(), op, ImGuizmo::LOCAL, (float*)&m);
            
            if (ImGuizmo::IsUsing()) {
                t->setFromMatrix(m);
            }
        }
    }
    ImGui::End();
}



void ModuleEditor::recreateSceneRenderTarget() {

    uint32_t width = (uint32_t)sceneSize.x;
    uint32_t height = (uint32_t)sceneSize.y;

    if (width == 0 || height == 0)
        return;

    moduleD3d12->flush();

    if (sceneSRV != UINT(-1)) moduleShaderDescriptors->free(sceneSRV);
    if (sceneRTV != UINT(-1)) moduleRTDescriptors->free(sceneRTV);
    if (sceneDSV != UINT(-1)) moduleDSDescriptors->free(sceneDSV);

    sceneColor.Reset();
    sceneDepth.Reset();

    const float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    sceneColor = moduleResources->createRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, clearColor);

    sceneRTV = moduleRTDescriptors->alloc();
    moduleRTDescriptors->create(sceneRTV, sceneColor.Get());

    sceneSRV = moduleShaderDescriptors->alloc(1);
    moduleShaderDescriptors->createTextureSRV(sceneSRV, 0, sceneColor.Get());

    // ---------- Depth ----------
    sceneDepth = moduleResources->createDepthStencil(DXGI_FORMAT_D32_FLOAT, width, height, 1.0f);

    sceneDSV = moduleDSDescriptors->alloc();
    moduleDSDescriptors->create(sceneDSV, sceneDepth.Get());
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleEditor::getSceneRTV() const {
    return app->getModuleRTDescriptors()->getCPUHandle(sceneRTV);
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleEditor::getSceneDSV() const {
    return app->getModuleDSDescriptors()->getCPUHandle(sceneDSV);
}

D3D12_GPU_DESCRIPTOR_HANDLE ModuleEditor::getSceneSRV() const {
    return app->getModuleShaderDescriptors()->getGPUHandle(sceneSRV, 0);
}
