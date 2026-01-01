#pragma once

namespace tinygltf { class Model; struct Material; }

struct BasicMaterialData {
    XMFLOAT4    baseColour;
    BOOL        hasColourTexture;
};

struct PhongMaterialData {
    XMFLOAT4    diffuseColour;
    float       Kd;
    float       Ks;
    float       shininess;
    BOOL        hasDiffuseTex;
};

class BasicMaterial {
public:
    enum Type
    {
        BASIC = 0,
        PHONG
    };

private:
    std::string name;
    UINT textureTable = UINT(-1);

    union {
        BasicMaterialData basic;
        PhongMaterialData phong;
    } materialData;

    Type materialType = BASIC;

    ComPtr<ID3D12Resource>  baseColourTex;

public:
    BasicMaterial();
    ~BasicMaterial();

    void load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath, BasicMaterial::Type materialType);

public:
    const inline char* getName() const { return name.c_str(); }
    UINT inline getTextureTable() const { return textureTable; }
    Type inline getMaterialType() const { return materialType; }

    const inline BasicMaterialData getBasicMaterial() const { return materialData.basic; }
    const inline PhongMaterialData getPhongMaterial() const { return materialData.phong; }
    void inline setPhongMaterial(PhongMaterialData newData) { materialData.phong = newData; }

};