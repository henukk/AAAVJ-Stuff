#include "Globals.h"
#include "EditorToolbar.h"
#include "imgui.h"

#include "Application.h"
#include "ModuleEditor.h"

constexpr const char* PAN_TEXT = "Pan";
constexpr const char* ORBIT_TEXT = "Orbit";
constexpr const char* ZOOM_TEXT = "Zoom";
constexpr const char* FLY_MODE_TEXT = "Fly mode";

constexpr const char* VIEW_NAV_MODES[4] = { PAN_TEXT, ORBIT_TEXT, ZOOM_TEXT, FLY_MODE_TEXT };

constexpr const char* MOVE_TEXT = "Move";
constexpr const char* ROTATE_TEXT = "Rotate";
constexpr const char* SCALE_TEXT = "Scale";
constexpr const char* RECT_TRANSFORM_TEXT = "RectTransform";
constexpr const char* TRANSFORM_TEXT = "Transform";

constexpr const char* BAR_FUNCTIONS[5] = { MOVE_TEXT, ROTATE_TEXT, SCALE_TEXT, RECT_TRANSFORM_TEXT, TRANSFORM_TEXT };

EditorToolbar::EditorToolbar() {
	moduleEditor = app->getModuleEditor();
}

EditorToolbar::~EditorToolbar() {}

void EditorToolbar::DrawCentered(float menuWidth)
{
    float toolbarWidth = 6 * buttonWidth;
    float centerPos = (menuWidth - toolbarWidth) * 0.5f;
    ImGui::SetCursorPosX(centerPos);


	int selectedIndex = static_cast<int>(moduleEditor->getCurrentSceneTool());
	int selectedNavIndex = static_cast<int>(moduleEditor->getCurrentNavigationMode());

    ManageNavigationButton(selectedIndex, selectedNavIndex);
    ManagePositionButton(selectedIndex);
    
    ImGui::SameLine();
}

void EditorToolbar::ManageNavigationButton(int selectedIndex, int selectedNavIndex) {    
    CreateButton(selectedIndex, VIEW_NAV_MODES[selectedNavIndex], 0);
}

void EditorToolbar::ManagePositionButton(int selectedIndex) {
    for (int i = 1; i < 6; i++) {
        CreateButton(selectedIndex, BAR_FUNCTIONS[i-1], i);
    }
}

void EditorToolbar::CreateButton(int selectedIndex, const char* text, int index) {
    if (selectedIndex == index) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.55f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.65f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.45f, 1.0f, 1.0f));
    }
    if (ImGui::Button(text, ImVec2(buttonWidth, buttonHeight))) {
        moduleEditor->setCurrentSceneTool(index);
    }

    if (selectedIndex == index) {
        ImGui::PopStyleColor(3);
    }

    ImGui::SameLine();
}
