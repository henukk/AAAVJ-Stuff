#pragma once

#include "DebugDrawPass.h"
#include "ImGuiPass.h"
#include "Module.h"
#include <imgui.h>
#include "ImGuizmo.h"

class ModuleD3D12;
class ModuleResources;
class ModuleCamera;
class ModuleSamplers;
class ModuleShaderDescriptors;

class BasicModel;

class Exercise5 : public Module {
    ModuleD3D12* moduleD3d12;
    ModuleResources* moduleResources;
    ModuleCamera* moduleCamera;
    ModuleSamplers* moduleSamplers;
    ModuleShaderDescriptors* moduleDescriptors;

    ComPtr<ID3D12RootSignature>         rootSignature;
    ComPtr<ID3D12PipelineState>         pso;
    std::unique_ptr<DebugDrawPass>      debugDrawPass;
    std::unique_ptr<ImGuiPass>          imguiPass;
    std::vector<ComPtr<ID3D12Resource>> materialBuffers;
    bool                                showAxis = false;
    bool                                showGrid = true;
    bool                                showGuizmo = true;
    ImGuizmo::OPERATION                 gizmoOperation = ImGuizmo::TRANSLATE;
    std::unique_ptr<BasicModel>         model;

public:
    Exercise5();
    ~Exercise5();

    virtual bool init() override;
    virtual bool cleanUp() override;
    virtual void preRender() override;
    virtual void render() override;

private:
    void imGuiCommands();
    bool createRootSignature();
    bool createPSO();
    bool loadModel();
};