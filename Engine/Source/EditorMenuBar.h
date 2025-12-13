#pragma once
#include <string>

#include "EditorToolbar.h"

class EditorMenuBar {
public:
    EditorMenuBar();
    ~EditorMenuBar();

    void draw();

    bool isConsoleVisible() const { return showConsole; }
    bool isDemoVisible() const { return showDemo; }

private:
    bool showConsole = true;
    bool showDemo = true;

    EditorToolbar toolbar;
};
