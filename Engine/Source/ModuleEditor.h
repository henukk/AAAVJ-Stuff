#pragma once
#include "Module.h"
#include "Keyboard.h"
#include "Mouse.h"

class Settings;

class ModuleInput;
class ModuleCamera;

//class EditorConsole;
class EditorMenuBar;
class EditorSettings;
class EditorPerformance;
class EditorAbout;

class BasicModel;

class ModuleEditor : public Module {
public:
    enum SCENE_TOOL {
        NONE = -1,
        NAVIGATION = 0,
        MOVE = 1,
        ROTATE = 2,
        SCALE = 3,
        RECT = 4,
        TRANSFORM = 5
    };
    enum NAVIGATION_MODE {
        PAN = 0,
        ORBIT = 1,
        ZOOM = 2,
        FREE_LOOK = 3
    };

private:
    Settings* settings;

    ModuleInput* moduleInput;
    ModuleCamera* moduleCamera;

#pragma region SCENE_RENDER
    bool sceneHovered = false;
    ImVec2 sceneScreenMin;

    ComPtr<ID3D12Resource> sceneColor;
    ComPtr<ID3D12Resource> sceneDepth;

    // Descriptor handles
    UINT sceneRTV = UINT(-1);
    UINT sceneDSV = UINT(-1);
    UINT sceneSRV = UINT(-1);

    // Size tracking
    ImVec2 sceneSize = { 0, 0 };
    ImVec2 prevSceneSize = { 0, 0 };
#pragma endregion

    //EditorConsole* console;
    EditorMenuBar* menuBar;
    EditorSettings* editorSettings;
	EditorPerformance* editorPerformance;
	EditorAbout* editorAbout;

    //EDITOR MANAGEMENT
	SCENE_TOOL currentSceneTool;
	NAVIGATION_MODE currentNavigationMode;
	SCENE_TOOL previousSceneTool;

    //SELECTED GAMEOBJECT
    BasicModel* selectedGameObject;

public:
    ModuleEditor();

    bool init() override;
    bool cleanUp() override;
    void update() override;
    void render() override;

private:
    void drawDockSpace();
    void drawPanels();

    void setMode(SCENE_TOOL sceneTool, NAVIGATION_MODE navigationMode);
    void resetMode();

    void handleKeyboardShortcuts();
    void handleQWERTYCases(Keyboard::State keyboardState);

    void drawSceneWindow();
    void recreateSceneRenderTarget();
public:
    SCENE_TOOL getCurrentSceneTool() const { return currentSceneTool; }
	NAVIGATION_MODE getCurrentNavigationMode() const { return currentNavigationMode; }
	void setCurrentSceneTool(int tool) { currentSceneTool = static_cast<SCENE_TOOL>(tool); }

    constexpr inline void setSelectedGameObject(BasicModel* gameObject) {
        selectedGameObject = gameObject;
    };
    constexpr inline void releaseSelected() {
        selectedGameObject = nullptr;
    }
    inline const BasicModel* getSelectedGameObject() const {
        return selectedGameObject;
    }

#pragma region SCENE_RENDER
    bool isSceneHovered() const { return sceneHovered; }
    ImVec2 getSceneScreenMin() const { return sceneScreenMin; }

    ImVec2 getSceneSize() const { return sceneSize; }
    ID3D12Resource* getSceneColor() const { return sceneColor.Get(); }

    D3D12_CPU_DESCRIPTOR_HANDLE getSceneRTV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE getSceneDSV() const;
    D3D12_GPU_DESCRIPTOR_HANDLE getSceneSRV() const;
#pragma endregion

};
