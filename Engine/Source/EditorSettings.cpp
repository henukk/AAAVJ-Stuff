#include "Globals.h"
#include "EditorSettings.h"
#include "Application.h"
#include "Settings.h"

EditorSettings::EditorSettings() {
    settings = app->getSettings();
}

EditorSettings::~EditorSettings() {

}

void EditorSettings::draw(const char* title, bool* p_open) {
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Camera")) {
        if (ImGui::CollapsingHeader("Pan")) {
            ImGui::DragFloat("Pan Speed", &settings->camera.panSpeed, 0.0001f, 0.00001f, 0.05f);
            ImGui::Checkbox("Invert X###PanInvX", &settings->camera.panInvertX);
            ImGui::Checkbox("Invert Y###PanInvY", &settings->camera.panInvertY);
        }

        if (ImGui::CollapsingHeader("Orbit")) {
            ImGui::DragFloat("Orbit Speed", &settings->camera.orbitSpeed, 0.0001f, 0.0001f, 0.05f);
            ImGui::Checkbox("Invert X###OrbInvX", &settings->camera.orbitInvertX);
            ImGui::Checkbox("Invert Y###OrbInvY", &settings->camera.orbitInvertY);

            ImGui::DragFloat("Min Distance", &settings->camera.minDistance, 0.1f, 0.01f, 1000.0f);
            ImGui::DragFloat("Max Distance", &settings->camera.maxDistance, 0.1f, 0.1f, 5000.0f);
        }

        if (ImGui::CollapsingHeader("Zoom")) {
            ImGui::DragFloat("Zoom Drag Speed", &settings->camera.zoomDragSpeed, 0.0001f, 0.00001f, 0.1f);
            ImGui::DragFloat("Zoom Wheel Speed", &settings->camera.zoomWheelSpeed, 0.0001f, 0.000001f, 0.1f);
        }

        if (ImGui::CollapsingHeader("Flythrough")) {
            ImGui::DragFloat("Move Speed", &settings->camera.flyMoveSpeed, 0.1f, 0.1f, 500.0f);
            ImGui::DragFloat("Boost Mult", &settings->camera.flyBoostMult, 0.1f, 1.0f, 20.0f);
            ImGui::DragFloat("Rotation Speed", &settings->camera.flyRotSpeed, 0.0001f, 0.0001f, 0.05f);

            ImGui::Checkbox("Invert X###FlyInvX", &settings->camera.flyInvertX);
            ImGui::Checkbox("Invert Y###FlyInvY", &settings->camera.flyInvertY);

            ImGui::DragFloat("Pitch Clamp", &settings->camera.flyPitchClamp, 0.01f, 0.1f, XM_PIDIV2 - 0.01f);
        }
    }

    ImGui::End();
}
