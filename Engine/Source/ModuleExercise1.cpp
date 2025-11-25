#include "Globals.h"
#include "ModuleExercise1.h"
#include "Application.h"
#include "ModuleD3D12.h"

bool ModuleExercise1::init(){
	d3d12 = app->getModuleD3D12();
	return true;
}

void ModuleExercise1::render() {
	ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12->getRenderTargetDescriptor();

    const float clearColor[4] = { 1.f, 0.f, 0.f, 1.f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}