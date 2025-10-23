#include "Globals.h"
#include "ModuleD3D12.h"

constexpr int N = 3;

ModuleD3D12::ModuleD3D12(HWND hWnd) : hWnd(hWnd) {

}

ModuleD3D12::~ModuleD3D12()
{

}

bool ModuleD3D12::init() {
	#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugInterface;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
		debugInterface->EnableDebugLayer();
	#endif
	
	//factory???
	#if defined(_DEBUG)
		CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
	#else
		CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
	#endif

	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

	//info queue???
	ComPtr<ID3D12InfoQueue> infoQueue;
	device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
		
	//swap chain
	createCommandQueue();
	createSwapChain();
	return true;
}

void ModuleD3D12::update() {

}

void ModuleD3D12::preRender() {
	//currentIndex = swapChain->GetCurrentBackBufferIndex();
	//WaitForFence(fenceValues[currentIndex]); // SetEventOnCompletion + WaitForSingleObject
	//commandAllocators[currentIndex]->Reset();
}
void ModuleD3D12::postRender() {
	//commandQueue->Signal(fence, ++fenceValue);
	//currentIndex = swapChain->GetCurrentBackBufferIndex();
	//fenceValues[currentIndex] = fenceValue; // Store the fence for this buffer
}

void ModuleD3D12::render() {

}

bool ModuleD3D12::cleanUp() {
	return true;
}

void ModuleD3D12::createCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
}

void ModuleD3D12::createSwapChain() {
	//windows size
	RECT rect = {};
	GetWindowRect(hWnd, &rect);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = UINT(rect.right - rect.left); // Width of the back buffer in pixels
	swapChainDesc.Height = UINT(rect.bottom - rect.top); // Height of the back buffer in pixels
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit RGBA format (8 bits per channel)
	// UNORM = Unsigned normalized integer (0-255 mapped to 0.0-1.0)
	swapChainDesc.Stereo = FALSE; // Set to TRUE for stereoscopic 3D rendering (VR/3D Vision)
	swapChainDesc.SampleDesc = { 1, 0 }; // Multisampling { Count, Quality } // Count=1: No multisampling (1 sample per pixel)
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // This buffer will be used as a render target
	swapChainDesc.BufferCount = N; // Double buffering:
	// - 1 front buffer (displayed)
   // - 1 back buffer (rendering)
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // How to scale when window size doesn't match buffer size:
	// STRETCH = Stretch the image to fit the window
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Modern efficient swap method:
	// - FLIP: Uses page flipping (no copying)
   // - DISCARD: Discard previous back buffer contents
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Alpha channel behavior for window blending UNSPECIFIED = Use default behavior
	swapChainDesc.Flags = 0; // Additional swap chain options: 0 = No special flags
	// DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: Allow full-screen mode switches
   // DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING: Allow tearing in windowed mode (VSync off)


	ComPtr<IDXGISwapChain1> swapChain1;
	factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1);
	swapChain1.As(&swapChain);
}