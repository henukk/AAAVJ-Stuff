#pragma once
#include <vector>
#include <string>
#include "imgui.h"

constexpr UINT CONSOLE_MAX_ITEMS = 2048;

enum class LogType {
    CONSOLE_LOG,
    CONSOLE_WARNING,
    CONSOLE_ERROR
};

struct ConsoleItem {
    std::string message;
    LogType type;
    bool selected = false;
};

class EditorConsole {
public:
    EditorConsole();
    ~EditorConsole();

    void clear();

    void PrintLog(const char* fmt, ...);
    void PrintWarn(const char* fmt, ...);
    void PrintError(const char* fmt, ...);

    void draw(const char* title, bool* p_open = nullptr);

private:
    void drawHeader();
    void drawMessages();
    void drawMessage(ConsoleItem& item, size_t index);
    void addMessage(LogType type, const char* fmt, va_list args);
    void checkCopy(bool copyPressed);
    void clearSelection();

private:
    ImGuiTextFilter filter;
    std::vector<ConsoleItem> items;

    bool autoScroll = true;

    bool showLogs = true;
    bool showWarnings = true;
    bool showErrors = true;
};

extern EditorConsole* Console;