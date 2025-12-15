#pragma once
#include "imgui.h"

class Settings;

class EditorSettings {
private:
    Settings* settings;

public:
    EditorSettings();
    ~EditorSettings();

    void draw(const char* title, bool* p_open = nullptr);

    void drawCameraSettings();
    void drawSceneSettings();
};
