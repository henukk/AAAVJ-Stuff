#include "Globals.h"
#include "ModuleCamera.h"

#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleUI.h"

bool ModuleCamera::init() {
    moduleInput = app->getModuleInput();
    moduleD3D12 = app->getModuleD3D12();

    app->getModuleUI()->registerWindow([this]() { drawGUI(); });

    return true;
}

void ModuleCamera::update() {
    unsigned width = moduleD3D12->getWindowWidth();
    unsigned height = moduleD3D12->getWindowHeight();

    app->getElapsedMilis();

    if (position == target)
        target.z += 0.001f;

    view = Matrix::CreateLookAt(position, target, up);
    projection = Matrix::CreatePerspectiveFieldOfView(XM_PIDIV4, float(width) / float(height), 0.1f, 1000.0f);
}

void ModuleCamera::drawGUI()
{
    if (ImGui::Begin("Camera"))
    {
        if (ImGui::CollapsingHeader("Transforms", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("Position", &position.x, 0.1f);
            ImGui::DragFloat3("Target", &target.x, 0.1f);
        }

        if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("View Matrix:");
            ImGui::Text("%f %f %f %f", view._11, view._12, view._13, view._14);
            ImGui::Text("%f %f %f %f", view._21, view._22, view._23, view._24);
            ImGui::Text("%f %f %f %f", view._31, view._32, view._33, view._34);
            ImGui::Text("%f %f %f %f", view._41, view._42, view._43, view._44);
        }
    }
    ImGui::End();
}
