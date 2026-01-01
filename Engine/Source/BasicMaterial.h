#pragma once

namespace tinygltf { class Model; struct Material; }

struct BasicMaterialData
{
    Vector4 baseColour;
    BOOL    hasColourTexture;
};

class BasicMaterial
{
public:
    void load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath);

    const BasicMaterialData getBasicMaterial() const { return materialData; }

    const char* getName() const { return name.c_str(); }
    UINT getTextureTable() const { return textureTable; }

private:
    std::string name;
    BasicMaterialData materialData;
    UINT textureTable = UINT(-1);

    ComPtr<ID3D12Resource>  baseColourTex;

};