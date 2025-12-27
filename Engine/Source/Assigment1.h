#pragma once

#include "Module.h"
#include "DebugDrawPass.h"

#include<vector>

class Settings;

class ModuleD3D12;
class ModuleResources;
class ModuleSamplers;
class ModuleRender;
class ModuleCamera;

class ModuleUI;

class Assigment1 : public Module {
private:
    enum Assigment1_TextureList {
        CHECKERBOARD_NO_MIPMAPS,
        CHECKERBOARD,
        THIN_GRID_NO_MIPMAPS,
        THIN_GRID,
        SMALL_TEXT_NO_MIPMAPS,
        SMALL_TEXT,
        NOISE_NO_MIPMAPS,
        NOISE,
        UV_TEST_NO_MIPMAPS,
        UV_TEST,
        METAL_GRATE_NO_MIPMAPS,
        METAL_GRATE,
        DOG,
        COUNT
    };

private:
    Settings* settings;
    ModuleD3D12* moduleD3d12;
    ModuleResources* moduleResources;
	ModuleSamplers* moduleSamplers;
    ModuleRender* moduleRender;
    ModuleCamera* moduleCamera;

    ModuleUI* moduleUI;

    std::vector<ComPtr<ID3D12Resource>> textureVector;

    ComPtr<ID3D12Resource>          vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW        vertexBufferView;
    ComPtr<ID3D12RootSignature>     rootSignature;
    ComPtr<ID3D12PipelineState>     pso;
    std::unique_ptr<DebugDrawPass>  debugDrawPass;

    Matrix mvp;
    Vector3 textPos = { 0.0f, 0.0f, 0.0f };
    Vector3 textRot = { 0.0f, 0.0f, 0.0f };
    Vector3 textScale = { 1.0f, 1.0f, 1.0f };
    int sampler;
    int selectedTexture;

    ComPtr<ID3D12DescriptorHeap> srvHeap;
    D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle{};


public:
    Assigment1();
    virtual bool init() override;
    virtual bool cleanUp() override;
    virtual void render() override;

private:
    bool loadTextures();

    bool createVertexBuffer(void* bufferData, unsigned bufferSize, unsigned stride);
    bool createRootSignature();
    bool createPSO();

    void drawGUI();
};