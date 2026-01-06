#include "Globals.h"
#include "ModuleRender.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleEditor.h"

bool ModuleRender::init() {
    d3d12 = app->getModuleD3D12();
    return true;
}

void ModuleRender::preRender() {
    auto* cmd = d3d12->getCommandList();
    auto* editor = app->getModuleEditor();

    cmd->Reset(d3d12->getCommandAllocator(), nullptr);

    ID3D12Resource* sceneColor = editor->getSceneColor();
    ImVec2 size = editor->getSceneSize();

    if (sceneColor && size.x > 0.0f && size.y > 0.0f) {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(sceneColor, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmd->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv = editor->getSceneRTV();
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = editor->getSceneDSV();
        cmd->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

        const float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        cmd->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
        cmd->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        D3D12_VIEWPORT vp{ 0.0f, 0.0f, size.x, size.y, 0.0f, 1.0f };
        D3D12_RECT sc{ 0, 0, LONG(size.x), LONG(size.y) };
        cmd->RSSetViewports(1, &vp);
        cmd->RSSetScissorRects(1, &sc);
    }
}


void ModuleRender::render() {
    auto* cmd = d3d12->getCommandList();
    auto* editor = app->getModuleEditor();

    ID3D12Resource* sceneColor = editor->getSceneColor();

    if (sceneColor) {
        for (auto& pass : worldPasses)
            pass(cmd);

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(sceneColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        cmd->ResourceBarrier(1, &barrier);
    }

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmd->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = d3d12->getDepthStencilDescriptor();
    cmd->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    const float bbClear[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    cmd->ClearRenderTargetView(rtv, bbClear, 0, nullptr);
    cmd->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    unsigned w = d3d12->getWindowWidth();
    unsigned h = d3d12->getWindowHeight();
    D3D12_VIEWPORT vp{ 0.0f, 0.0f, float(w), float(h), 0.0f, 1.0f };
    D3D12_RECT sc{ 0, 0, LONG(w), LONG(h) };
    cmd->RSSetViewports(1, &vp);
    cmd->RSSetScissorRects(1, &sc);

    for (auto& ui : uiPasses)
        ui(cmd);

    // Backbuffer -> Present
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmd->ResourceBarrier(1, &barrier);

    cmd->Close();
    ID3D12CommandList* lists[] = { cmd };
    d3d12->getDrawCommandQueue()->ExecuteCommandLists(1, lists);

    worldPasses.clear();
    uiPasses.clear();
}



bool ModuleRender::cleanUp() {
    worldPasses.clear();
    uiPasses.clear();
    return true;
}

void ModuleRender::registerWorldPass(const RenderCallback& cb) {
    worldPasses.push_back(cb);
}

void ModuleRender::registerUIPass(const RenderCallback& cb) {
    uiPasses.push_back(cb);
}
