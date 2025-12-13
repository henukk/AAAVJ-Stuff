#pragma once
#include "imgui.h"

class ModuleEditor;

class EditorToolbar {
private:
	ModuleEditor* moduleEditor;

    float buttonWidth = 100.0f;
    float buttonHeight = 28.0f;

public:
    EditorToolbar();
    ~EditorToolbar();

    void DrawCentered(float menuWidth);

private:
    void ManageNavigationButton(int selectedIndex, int selectedNavIndex);
    void ManagePositionButton(int selectedIndex);
    void CreateButton(int selectedIndex, const char* text, int index);
};
