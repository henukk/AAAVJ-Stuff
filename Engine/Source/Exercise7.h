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
class ModuleRingBuffer;
class ModuleEditor;

class BasicModel;

class Exercise7 : public Module {
    struct PerInstance {
        Matrix modelMat;
        Matrix normalMat;
        PBRPhongMaterialData material;
    };

    struct Light {
        Vector3 L = Vector3::One * (-0.5f);
        Vector3 Lc = Vector3::One;
        Vector3 Ac = Vector3::One * (0.1f);
    };

    struct PerFrame {
        Vector3 L = Vector3::UnitX;
        float pad0;
        Vector3 Lc = Vector3::One;
        float pad1;
        Vector3 Ac = Vector3::Zero;
        float pad2;
        Vector3 viewPos = Vector3::Zero;
        float pad3;
    };
private:
    ModuleD3D12* moduleD3d12;
    ModuleRender* moduleRender;
    ModuleResources* moduleResources;
    ModuleCamera* moduleCamera;
    ModuleSamplers* moduleSamplers;
    ModuleShaderDescriptors* moduleShaderDescriptors;
    ModuleRingBuffer* moduleRingBuffer;
	ModuleEditor* moduleEditor;

    ComPtr<ID3D12RootSignature>         rootSignature;
    ComPtr<ID3D12PipelineState>         pso;
    std::unique_ptr<DebugDrawPass>      debugDrawPass;
    std::vector<ComPtr<ID3D12Resource>> materialBuffers;

    BasicModel model;
    Light light;

public:
    Exercise7();
    ~Exercise7();

    virtual bool init() override;
    virtual bool cleanUp() override;
    virtual void render() override;

private:
    bool createRootSignature();
    bool createPSO();
    bool loadModel();

    void drawGUI();
};