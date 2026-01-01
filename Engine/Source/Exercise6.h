#pragma once
#include "DebugDrawPass.h"
#include "Module.h"
#include "BasicModel.h"

class ModuleD3D12;
class ModuleRender;
class ModuleResources;
class ModuleCamera;
class ModuleSamplers;
class ModuleShaderDescriptors;

class BasicModel;

class Exercise6 : public Module {
private:
    ModuleD3D12* moduleD3d12;
    ModuleRender* moduleRender;
    ModuleResources* moduleResources;
    ModuleCamera* moduleCamera;
    ModuleSamplers* moduleSamplers;
    ModuleShaderDescriptors* moduleShaderDescriptors;

    ComPtr<ID3D12RootSignature>         rootSignature;
    ComPtr<ID3D12PipelineState>         pso;
    std::unique_ptr<DebugDrawPass>      debugDrawPass;
    std::vector<ComPtr<ID3D12Resource>> materialBuffers;

    BasicModel model;

public:
    Exercise6();
    ~Exercise6();

    virtual bool init() override;
    virtual bool cleanUp() override;
    virtual void render() override;

private:
    bool createRootSignature();
    bool createPSO();
    bool loadModel();

    void drawGUI();
};