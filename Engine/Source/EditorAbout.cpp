#include "Globals.h"
#include "EditorAbout.h"
#include "EditorConsole.h"

#include <windows.h>
#include <shellapi.h>

EditorAbout::EditorAbout(bool& imOppened) : imOpened(imOppened) {}
EditorAbout::~EditorAbout() {}

void EditorAbout::draw(const char* title, bool* p_open) {
    if (!ImGui::Begin(title, &imOpened)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Software Name: %s", "ClangClin");
    ImGui::Text("Version: %s", "v0.1.0 (Assignment 1)");

    ImGui::Separator();

    ImGui::TextWrapped(
        "This project is a DirectX 12-based game engine developed as part of a learning process. "
        "Its main goal is to demonstrate the knowledge and skills acquired during the course, "
        "covering core concepts of real-time rendering, engine architecture, and modern graphics APIs."
    );

    ImGui::Separator();

    if (ImGui::Button("Author GitHub (Click to copy link)")) {
        ImGui::SetClipboardText("https://github.com/henukk");
		Console->PrintLog("Author GitHub link copied to clipboard.");
    }

    if (ImGui::Button("Project Repository (Click to copy link)")) {
        ImGui::SetClipboardText("https://github.com/henukk/AAAVJ-Stuff");
        Console->PrintLog("Project Repository link copied to clipboard.");
    }

    if (ImGui::Button("Documentation (Click to copy link)")) {
        ImGui::SetClipboardText("https://github.com/henukk/AAAVJ-Stuff/wiki");
        Console->PrintLog("Documentation link copied to clipboard.");
    }

    if (ImGui::Button("License (Click to copy link)")) {
        ImGui::SetClipboardText("https://github.com/henukk/AAAVJ-Stuff/blob/main/LICENSE");
        Console->PrintLog("License link copied to clipboard.");
    }

    ImGui::Separator();

    ImGui::SetCursorPosX(
        ImGui::GetWindowWidth() - ImGui::CalcTextSize("Close").x - 20
    );

    if (ImGui::Button("Close")) {
        closeMyself();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        closeMyself();
    }

    ImGui::End();
}


void EditorAbout::closeMyself() {
	imOpened = false;
}
