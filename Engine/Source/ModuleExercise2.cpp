#include "Globals.h"
#include "ModuleExercise2.h"
#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleResources.h"

ModuleExercise2::ModuleExercise2() {}
ModuleExercise2::~ModuleExercise2() {}

bool ModuleExercise2::init() {
	d3d12 = app->getModuleD3D12();
	moduleResources = app->getModuleResources();
	return true;
}

bool ModuleExercise2::postInit() {
	return true;
}

void ModuleExercise2::update() {}

void ModuleExercise2::preRender() {}

void ModuleExercise2::postRender() {}

void ModuleExercise2::render() {
	ComPtr<ID3D12GraphicsCommandList> commandList = d3d12->getCommandList();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12->getRtvHandle();

	const float clearColor[4] = { 1.f, 1.f, 0.f, 1.f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

bool ModuleExercise2::cleanUp() {
	return true;
}