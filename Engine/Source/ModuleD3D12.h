#pragma once
#include "Module.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public Module {
private:
	HWND hWnd;
	ComPtr<IDXGIFactory6> factory;
	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device5> device;

	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<IDXGISwapChain1> swapChain;

public:
	ModuleD3D12(HWND hWnd);
	~ModuleD3D12();

	bool init() override;
	void update() override;
	void preRender() override;
	void postRender() override;
	void render() override;
	bool cleanUp() override;

private:
	void createCommandQueue();
	void createSwapChain();
};
