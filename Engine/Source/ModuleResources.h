#pragma once
#include "Module.h"

#include <filesystem>
#include <vector>

namespace DirectX { class ScratchImage;  struct TexMetadata; }

class ModuleD3D12;

class ModuleResources : public Module {
private:
    ModuleD3D12* d3d12;
    ID3D12Device5* device;

    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;
public:
    bool init() override;
    bool cleanUp() override;

public:
    ComPtr<ID3D12Resource> createUploadBuffer(const void* data, size_t size, const char* name);
    ComPtr<ID3D12Resource> createDefaultBuffer(const void* data, size_t size, const char* name);
    ComPtr<ID3D12Resource> createTextureFromFile(const std::filesystem::path& path, bool defaultSRGB = false, bool createMipmaps = true);
    ComPtr<ID3D12Resource> createRenderTarget(DXGI_FORMAT format, uint32_t width, uint32_t height, const float clearColor[4]);
    ComPtr<ID3D12Resource> createDepthStencil(DXGI_FORMAT format, uint32_t width, uint32_t height, float clearDepth);

    ComPtr<ID3D12Resource> getUploadHeap(size_t size);

private:
    ComPtr<ID3D12Resource> createTextureFromImage(const ScratchImage& image, const char* name);
};
