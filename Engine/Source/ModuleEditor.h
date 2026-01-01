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
};
