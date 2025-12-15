#pragma once
#include <string>

#include "EditorToolbar.h"

class EditorMenuBar {
private:
    bool showConsole;
    bool showSettings;

    EditorToolbar toolbar;
public:
    EditorMenuBar();
    ~EditorMenuBar();

    void draw();

    bool isConsoleVisible() const { return showConsole; }
    bool isSettingsVisible() const { return showSettings; }
};
