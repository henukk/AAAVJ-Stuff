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
    ComPtr<ID3D12Resource> createTextureFromFile(const std::filesystem::path& path, bool defaultSRGB = false);

    ComPtr<ID3D12Resource> getUploadHeap(size_t size);

private:
    ComPtr<ID3D12Resource> createTextureFromImage(const ScratchImage& image, const char* name);
};
