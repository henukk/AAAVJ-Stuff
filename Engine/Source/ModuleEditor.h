#pragma once
#include "Module.h"
#include "Keyboard.h"
#include "Mouse.h"

class ModuleInput;
//class EditorConsole;
class EditorMenuBar;
class EditorSettings;
class EditorPerformance;
class EditorAbout;

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
    ModuleInput* moduleInput;

    //EditorConsole* console;
    EditorMenuBar* menuBar;
    EditorSettings* editorSettings;
	EditorPerformance* editorPerformance;
	EditorAbout* editorAbout;

	SCENE_TOOL currentSceneTool;
	NAVIGATION_MODE currentNavigationMode;
	SCENE_TOOL previousSceneTool;

public:
    bool init() override;
    bool cleanUp() override;
    void update() override;

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
};
