#include "Globals.h"
#include "ModuleRender.h"
#include "Application.h"
#include "ModuleD3D12.h"

bool ModuleRender::init()
{
    d3d12 = app->getModuleD3D12();
    return true;
}

void ModuleRender::preRender()
{
    auto commandList = d3d12->getCommandList();

    commandList->Reset(d3d12->getCommandAllocator(), nullptr);

    // 4) Transición PRESENT -> RENDER_TARGET
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        d3d12->getBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(1, &barrier);

    // 5) Configurar RTV + DSV
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = d3d12->getDepthStencilDescriptor();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    // 6) Clear base (por defecto gris oscuro)
    const float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(
        dsv,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f, 0, 0, nullptr
    );

    // 7) Viewport + Scissor
    unsigned int windowWidth = d3d12->getWindowWidth();
    unsigned int windowHeight = d3d12->getWindowHeight();
    D3D12_VIEWPORT viewport{ 0.0f, 0.0f, float(windowWidth), float(windowHeight), 0.0f, 1.0f };
    D3D12_RECT scissor{ 0, 0, LONG(windowWidth), LONG(windowHeight) };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);
}

void ModuleRender::render()
{
    ID3D12GraphicsCommandList* cmd = d3d12->getCommandList();

    for (auto& pass : worldPasses)
        pass(cmd);

    for (auto& ui : uiPasses)
        ui(cmd);

    //Antes de d3d12 post render
    // Transición RENDER_TARGET -> PRESENT
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        d3d12->getBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    auto commandList = d3d12->getCommandList();
    commandList->ResourceBarrier(1, &barrier);

    // Cerrar y ejecutar la command list
    commandList->Close();
    ID3D12CommandList* commandLists[] = { commandList };
    d3d12->getDrawCommandQueue()->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

    worldPasses.clear();
    uiPasses.clear();
}

bool ModuleRender::cleanUp()
{
    worldPasses.clear();
    uiPasses.clear();
    return true;
}

void ModuleRender::registerWorldPass(const RenderCallback& cb)
{
    worldPasses.push_back(cb);
}

void ModuleRender::registerUIPass(const RenderCallback& cb)
{
    uiPasses.push_back(cb);
}
