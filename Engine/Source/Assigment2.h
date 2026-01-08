#pragma once
#include "DebugDrawPass.h"
#include "Module.h"
#include "BasicModel.h"
#include "Transform.h"

class ModuleD3D12;
class ModuleRender;
class ModuleResources;
class ModuleCamera;
class ModuleSamplers;
class ModuleShaderDescriptors;
class ModuleRingBuffer;
class ModuleEditor;

class BasicModel;

class Assigment2 : public Module {
    struct PerInstancePbr {
        Matrix modelMat;
        Matrix normalMat;
        PBRPhongMaterialData material;
    };

    struct PerInstancePhong {
        Matrix modelMat;
        Matrix normalMat;
        PhongMaterialData material;
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

    struct ModelState {
        BasicModel model;
        std::vector<ComPtr<ID3D12Resource>> materialBuffers;
    };

    enum ModelType {
        BASIC = 0,
        PHONG = 1,
        PBR_PHONG = 2,
        MODEL_COUNT
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


    ComPtr<ID3D12RootSignature>         rootSignatureBasic;
    ComPtr<ID3D12RootSignature>         rootSignatureLit;
    ComPtr<ID3D12PipelineState>         psoBasic;
    ComPtr<ID3D12PipelineState>         psoPhong;
    ComPtr<ID3D12PipelineState>         psoPbr;
    std::unique_ptr<DebugDrawPass>      debugDrawPass;

    Transform sharedTransform;
    ModelState models[ModelType::MODEL_COUNT];
    Light light;
    ModelType selectedModel = ModelType::PBR_PHONG;

public:
    Assigment2();
    ~Assigment2();

    virtual bool init() override;
    virtual bool cleanUp() override;
    virtual void render() override;

private:
    bool createRootSignatureBasic();
    bool createRootSignatureLit();
    bool createPSOBasic();
    bool createPSOPhong();
    bool createPSOPbr();
    bool loadModels();
    ModelState& getActiveModelState();

    void drawGUI();
};