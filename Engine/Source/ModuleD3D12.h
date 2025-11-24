#pragma once
#include "Module.h"

//#include <dxgi1_6.h> //No necesario y ahorramos mediante forward declaration

constexpr int N = 2;

class IDXGIFactory6;
class IDXGIAdapter4;
class IDXGISwapChain3;

class ImGuiPass;
//class ModuleD3D12;

class ModuleD3D12 : public Module {
private:
	HWND hWnd;
	ComPtr<IDXGIFactory6> factory;
	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device5> device;

	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12CommandAllocator> commandAllocators[N];
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	UINT rtvDescriptorSize = 0;
	ComPtr<ID3D12Resource> renderTargets[N];
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<IDXGISwapChain3> swapChain;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValues[N] = {};
    UINT64 fenceValue = 0;
    HANDLE fenceEvent = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

	UINT currentIndex;

    unsigned windowWidth = 0;
    unsigned windowHeight = 0;
    bool fullscreen = false;

public:
	ModuleD3D12(HWND hWnd);
	~ModuleD3D12();

    bool init() override;
    void preRender() override;
    void postRender() override;
    bool cleanUp() override;

private:
    void initDXGIFactoryAndDevice();

    void createCommandQueue();
    void createCommandAllocators();
    void createCommandList();

    void createSwapChain();
    void createRTVDescriptorHeap();
    void createRenderTargets();

    void initSynchronization();
    void waitForFence(UINT64 value);

    void getWindowSize(unsigned& width, unsigned& height);

public:
    HWND getHWND() const { return hWnd; }
    ID3D12Device5* getDevice() const { return device.Get(); }
    ID3D12GraphicsCommandList* getCommandList() const { return commandList.Get(); }
    IDXGISwapChain3* getSwapChain() const { return swapChain.Get(); }
    ID3D12CommandQueue* getCommandQueue() const { return commandQueue.Get(); }

    D3D12_CPU_DESCRIPTOR_HANDLE getRtvHandle() const { return rtvHandle; }

    unsigned getWindowWidth() const { return windowWidth; }
    unsigned getWindowHeight() const { return windowHeight; }
   
    void resize();
    void flush();
};
