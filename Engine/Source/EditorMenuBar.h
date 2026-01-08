#pragma once
#include <string>

class EditorMenuBar {
private:
    bool showConsole;
    bool showSettings;
    bool showPerformance;
    bool showAbout;
public:
    EditorMenuBar();
    ~EditorMenuBar();

    void draw();

	bool* getAboutVisible() { return &showAbout; }

    bool isConsoleVisible() const { return showConsole; }
    bool isSettingsVisible() const { return showSettings; }
    bool isPerformanceVisible() const { return showPerformance; }
	bool isAboutVisible() const { return showAbout; }
};
