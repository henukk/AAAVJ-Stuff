#include "Globals.h"
#include "ModuleD3D12.h"


ModuleD3D12::ModuleD3D12() {

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

	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device5> device;
	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

	//info queue???
	ComPtr<ID3D12InfoQueue> infoQueue;
	device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
	return true;
}

void ModuleD3D12::update() {

}

void ModuleD3D12::preRender() {

}

void ModuleD3D12::postRender() {

}

void ModuleD3D12::render() {

}

bool ModuleD3D12::cleanUp() {
	return true;
}
