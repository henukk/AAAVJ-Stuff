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

struct PBRPhongMaterialData {
    XMFLOAT3 diffuseColour;
    BOOL     hasDiffuseTex;

    XMFLOAT3 specularColour;
    float    shininess;
};

class BasicMaterial {
public:
    enum Type
    {
        BASIC = 0,
        PHONG,
        PBR_PHONG
    };

private:
    std::string name;
    UINT textureTable = UINT(-1);

    union {
        BasicMaterialData basic;
        PhongMaterialData phong;
		PBRPhongMaterialData pbrPhong;
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
    const inline PBRPhongMaterialData getPBRPhongMaterial() const { return materialData.pbrPhong; }

    void inline setPhongMaterial(const PhongMaterialData newData) { materialData.phong = newData; }
    void inline setPBRPhongMaterial(const PBRPhongMaterialData newData) { materialData.pbrPhong = newData; }
};