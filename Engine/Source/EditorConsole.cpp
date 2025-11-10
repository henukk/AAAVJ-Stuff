#include "Globals.h"
#include "EditorConsole.h"
#include <cstdarg>

EditorConsole::EditorConsole() {}
EditorConsole::~EditorConsole() {}

void EditorConsole::clear() {
    items.clear();
}

void EditorConsole::PrintLog(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    addMessage(LogType::CONSOLE_LOG, fmt, args);
    va_end(args);
}

void EditorConsole::PrintWarn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    addMessage(LogType::CONSOLE_WARNING, fmt, args);
    va_end(args);
}

void EditorConsole::PrintError(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    addMessage(LogType::CONSOLE_ERROR, fmt, args);
    va_end(args);
}

void EditorConsole::draw(const char* title, bool* p_open) {
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    drawHeader();
    ImGui::Separator();
    drawMessages();

    ImGui::End();
}

void EditorConsole::drawHeader() {
    static const size_t maxItems = CONSOLE_MAX_ITEMS;

    if (ImGui::Button("Clear")) clear();
    ImGui::SameLine();

    static bool copyPressed = false;
    if (ImGui::Button("Copy")) copyPressed = true;

    ImGui::SameLine();

    if (ImGui::BeginTable("ConsoleHeaderTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
        ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthFixed, 220.0f);

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Filter");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(180.0f);
        filter.Draw("##FilterInput");

        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Messages: %zu (Max: %zu)", items.size(), maxItems);

        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Checkbox("Logs", &showLogs);
    ImGui::SameLine();
    ImGui::Checkbox("Warnings", &showWarnings);
    ImGui::SameLine();
    ImGui::Checkbox("Errors", &showErrors);
    ImGui::Separator();

    checkCopy(copyPressed);
    copyPressed = false;
}

void EditorConsole::checkCopy(bool copyPressed) {
    if (!copyPressed) return;

    std::string clipboard;
    clipboard.reserve(4096);

    if (!selectedIndices.empty()) {
        for (auto index : selectedIndices) {
            if (index < items.size()) {
                clipboard += items[index].message + "\n";
            }
        }
    }
    else {
        for (auto& item : items) {
            if (!filter.PassFilter(item.message.c_str())) continue;
            if ((item.type == LogType::CONSOLE_LOG && !showLogs) ||
                (item.type == LogType::CONSOLE_WARNING && !showWarnings) ||
                (item.type == LogType::CONSOLE_ERROR && !showErrors)) continue;
            clipboard += item.message + "\n";
        }
    }

    ImGui::SetClipboardText(clipboard.c_str());
    this->PrintLog("Copied %zu line(s) to clipboard.", selectedIndices.empty() ? items.size() : selectedIndices.size());
}


void EditorConsole::drawMessages() {
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false,
        ImGuiWindowFlags_HorizontalScrollbar |
        ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (const auto& item : items) {
        if (!filter.PassFilter(item.message.c_str()))
            continue;

        if ((item.type == LogType::CONSOLE_LOG && !showLogs) ||
            (item.type == LogType::CONSOLE_WARNING && !showWarnings) ||
            (item.type == LogType::CONSOLE_ERROR && !showErrors))
            continue;

        drawMessage(item);
    }

    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
}

void EditorConsole::drawMessage(const ConsoleItem& item) {
    ImVec4 color;
    const char* prefix;
    switch (item.type) {
    case LogType::CONSOLE_LOG:     color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); prefix = "[LOG] "; break;
    case LogType::CONSOLE_WARNING: color = ImVec4(1.0f, 1.0f, 0.4f, 1.0f); prefix = "[WARN] "; break;
    case LogType::CONSOLE_ERROR:   color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); prefix = "[ERROR] "; break;
    default:                       color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); prefix = ""; break;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    std::string fullLine = std::string(prefix) + item.message;

    size_t index = &item - &items[0];
    bool isSelected = selectedIndices.count(index) > 0;

    if (ImGui::Selectable(fullLine.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::GetIO().KeyCtrl) {
            if (isSelected)
                selectedIndices.erase(index);
            else
                selectedIndices.insert(index);
        }
        else {
            selectedIndices.clear();
            selectedIndices.insert(index);
        }
    }

    ImGui::PopStyleColor();
}


void EditorConsole::addMessage(LogType type, const char* fmt, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);

    ConsoleItem item;
    item.message = std::string(buf);
    item.type = type;
    items.push_back(item);

    const size_t maxItems = CONSOLE_MAX_ITEMS;
    if (items.size() > maxItems) {
        items.erase(items.begin());
    }
}
