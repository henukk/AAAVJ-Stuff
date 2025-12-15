#include "Globals.h"
#include "EditorMenuBar.h"
#include "imgui.h"

EditorMenuBar::EditorMenuBar() {
    showConsole = true;
    showSettings = true;
}

EditorMenuBar::~EditorMenuBar() {}

void EditorMenuBar::draw() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                LOG("\n\n------------------------------------------------------------------------------------\nNOT IMPLEMENTED FILE>EXIT\n------------------------------------------------------------------------------------\n\n")
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {

            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {

            }
            ImGui::Separator();
            if (ImGui::MenuItem("Settings")) {

            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Console", nullptr, &showConsole);
            ImGui::MenuItem("Settings", nullptr, &showSettings);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About...")) {

            }
            ImGui::EndMenu();
        }

        float menuWidth = ImGui::GetWindowSize().x;

        toolbar.DrawCentered(menuWidth);

        ImGui::EndMainMenuBar();
    }
}
