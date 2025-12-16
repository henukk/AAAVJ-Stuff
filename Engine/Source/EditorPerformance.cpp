#include "Globals.h"
#include "EditorPerformance.h"

EditorPerformance::EditorPerformance() {}
EditorPerformance::~EditorPerformance() {}

void EditorPerformance::draw(const char* title, bool* p_open) {
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    ImGui::Separator();

    ImGui::End();
}
