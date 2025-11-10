#include "Globals.h"
#include "ModuleExercise1.h"
#include "Application.h"
#include "ModuleD3D12.h"

ModuleExercise1::ModuleExercise1() {

}

ModuleExercise1::~ModuleExercise1(){}

bool ModuleExercise1::init(){
	d3d12 = app->getModuleD3D12();
	return true;
}

bool ModuleExercise1::postInit(){
	return true;
}

void ModuleExercise1::update(){}

void ModuleExercise1::preRender(){}

void ModuleExercise1::postRender(){}

void ModuleExercise1::render() {
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12->getCommandList();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12->getRtvHandle();

	const float clearColor[4] = { 1.f, 0.f, 0.f, 1.f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

bool ModuleExercise1::cleanUp(){
	return true;
}