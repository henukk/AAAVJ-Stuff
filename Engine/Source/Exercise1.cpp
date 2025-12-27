#include "Globals.h"
#include "Exercise1.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleUI.h"
#include "imgui.h"

bool Exercise1::init()
{
    d3d12 = app->getModuleD3D12();
    ui = app->getModuleUI();

    ui->registerWindow([this]() { drawWindow(); });

    return true;
}

void Exercise1::render()
{
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12->getRenderTargetDescriptor();

    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void Exercise1::drawWindow()
{
    if (ImGui::Begin("Exercise 1 Settings"))
    {
        ImGui::ColorEdit4("Background Color", clearColor);
    }
    ImGui::End();
}
