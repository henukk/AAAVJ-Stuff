#include "Globals.h"
#include "EditorMenuBar.h"
#include "imgui.h"

#include "EditorConsole.h"

EditorMenuBar::EditorMenuBar() {
    showConsole = true;
    showSettings = true;
    showPerformance = true;
    showAbout = false;
}

EditorMenuBar::~EditorMenuBar() {}

void EditorMenuBar::draw() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "(Not implemented)")) {
                Console->PrintWarn("Not implemented yet!!!");
            }
			ImGui::Separator();
            if (ImGui::MenuItem("Load", "(Not implemented)")) {
                Console->PrintWarn("Not implemented yet!!!");
            }
            if (ImGui::MenuItem("Save", "(Not implemented)")) {
                Console->PrintWarn("Not implemented yet!!!");
            }
            if (ImGui::MenuItem("Save as", "(Not implemented)")) {
                Console->PrintWarn("Not implemented yet!!!");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("To do...", "(Not implemented)")) {
                Console->PrintWarn("Not implemented yet!!!");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Console", nullptr, &showConsole);
            ImGui::MenuItem("Settings", nullptr, &showSettings);
            ImGui::MenuItem("Performance", nullptr, &showPerformance);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("About", nullptr, &showAbout);
            ImGui::EndMenu();
        }

        float menuWidth = ImGui::GetWindowSize().x;

        toolbar.DrawCentered(menuWidth);

        ImGui::EndMainMenuBar();
    }
}
