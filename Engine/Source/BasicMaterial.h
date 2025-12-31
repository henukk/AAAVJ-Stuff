#pragma once

namespace tinygltf { class Model; struct Material; }

struct BasicMaterialData
{
    XMFLOAT4 baseColour;
    BOOL     hasColourTexture;
};

class BasicMaterial
{
public:
    void load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath);

    UINT getTextureTable() const { return textureTable; }
    const BasicMaterialData getBasicMaterial() const { return materialData; }

    const char* getName() const { return name.c_str(); }

private:
    UINT textureTable = UINT(-1);
    BasicMaterialData materialData;

    ComPtr<ID3D12Resource>  baseColourTex;
    ComPtr<ID3D12Resource>  metallicRoughnessTex;
    ComPtr<ID3D12Resource>  occlusionTex;
    ComPtr<ID3D12Resource>  normalTex;
    ComPtr<ID3D12Resource>  emissiveTex;

    std::string             name;
};