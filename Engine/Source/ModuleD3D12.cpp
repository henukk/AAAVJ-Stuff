#include "Globals.h"
#include "ModuleD3D12.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"

#include "Application.h"
#include "ImGuiPass.h"
#include "imgui.h"

ModuleD3D12::ModuleD3D12(HWND hWnd) : hWnd(hWnd), currentIndex(0), fenceValue(0), fenceEvent(nullptr) {}
ModuleD3D12::~ModuleD3D12() {
    flush();
}

bool ModuleD3D12::init() {
    #if defined(_DEBUG)
        ComPtr<ID3D12Debug> debugInterface;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
        debugInterface->EnableDebugLayer();
    #endif
    initDXGIFactoryAndDevice();

    getWindowSize(windowWidth, windowHeight);

    createCommandQueue();
    createCommandAllocators();
    createCommandList();

    createSwapChain();
    createRTVDescriptorHeap();
    createRenderTargets();

    initSynchronization();

    return true;
}

void ModuleD3D12::update() {}

void ModuleD3D12::preRender() {
    currentIndex = swapChain->GetCurrentBackBufferIndex();
    waitForFence(fenceValues[currentIndex]);
    commandAllocators[currentIndex]->Reset();

    commandList->Reset(commandAllocators[currentIndex].Get(), nullptr);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[currentIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    commandList->ResourceBarrier(1, &barrier);

    rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        currentIndex, rtvDescriptorSize
    );
}

void ModuleD3D12::render() {    
    const float clearColor[4] = { 1.f, 0.f, 0.f, 1.f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void ModuleD3D12::postRender() {
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[currentIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    commandList->ResourceBarrier(1, &barrier);

    commandList->Close();

    ID3D12CommandList* cmdLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, cmdLists);
    
    swapChain->Present(1, 0);

    commandQueue->Signal(fence.Get(), ++fenceValue);
    currentIndex = swapChain->GetCurrentBackBufferIndex();
    fenceValues[currentIndex] = fenceValue;
}

bool ModuleD3D12::cleanUp() {
    if (fenceEvent) {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }
    return true;
}

void ModuleD3D12::initDXGIFactoryAndDevice() {
#if defined(_DEBUG)
    CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
    CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
#endif
    factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
}

void ModuleD3D12::createCommandQueue() {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue));
}

void ModuleD3D12::createCommandAllocators() {
    for (int i = 0; i < N; ++i)
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
}

void ModuleD3D12::createCommandList() {
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList));
    commandList->Close();
}

void ModuleD3D12::createSwapChain() {
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = windowHeight;
    desc.Height = windowHeight;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = N;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.SampleDesc = { 1, 0 };
    desc.Scaling = DXGI_SCALING_STRETCH;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1);
    swapChain1.As(&swapChain);
}

void ModuleD3D12::createRTVDescriptorHeap() {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = N;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap));
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void ModuleD3D12::createRenderTargets() {
    auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < N; ++i) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, handle);
        handle.Offset(1, rtvDescriptorSize);
    }
}

void ModuleD3D12::initSynchronization() {
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void ModuleD3D12::waitForFence(UINT64 value) {
    if (fence->GetCompletedValue() < value) {
        fence->SetEventOnCompletion(value, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void ModuleD3D12::resize()
{
    unsigned width, height;
    getWindowSize(width, height);

    if (width == 0 || height == 0)
        return;

    if (width == windowWidth && height == windowHeight)
        return;

    windowWidth = width;
    windowHeight = height;

    flush();

    for (unsigned i = 0; i < N; ++i)
    {
        renderTargets[i].Reset();
        fenceValues[i] = 0;
    }

    // redimensionar el swap chain
    DXGI_SWAP_CHAIN_DESC desc = {};
    swapChain->GetDesc(&desc);

    HRESULT hr = swapChain->ResizeBuffers(
        N,
        windowWidth,
        windowHeight,
        desc.BufferDesc.Format,
        desc.Flags
    );

    if (FAILED(hr))
    {
        LOG("Failed to resize swap chain buffers");
        return;
    }

    currentIndex = swapChain->GetCurrentBackBufferIndex();

    // recrear RTVs
    createRenderTargets();

    LOG("SwapChain resized to %ux%u", windowWidth, windowHeight);
}



void ModuleD3D12::getWindowSize(unsigned& width, unsigned& height)
{
    RECT clientRect = {};
    GetClientRect(hWnd, &clientRect);

    width = unsigned(clientRect.right - clientRect.left);
    height = unsigned(clientRect.bottom - clientRect.top);
}

void ModuleD3D12::flush()
{
    commandQueue->Signal(fence.Get(), ++fenceValue);

    fence->SetEventOnCompletion(fenceValue, fenceEvent);
    WaitForSingleObject(fenceEvent, INFINITE);
}