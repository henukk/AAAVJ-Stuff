#pragma once

namespace tinygltf { class Model;  struct MaterPbrMetallicRoughnessial; }

struct BasicMaterialData {
    XMFLOAT4 baseColour;
    BOOL     hasColourTexture;
};

struct MetallicRoughnessMaterialData {
    XMFLOAT4 baseColour;
    float    metallicFactor;
    float    roughnessFactor;
    float    occlusionStrength;
    float    normalScale;
    XMFLOAT3 emissiveFactor;
    BOOL     hasBaseColourTex;
    BOOL     hasMetallicRoughnessTex;
    BOOL     hasOcclusionTex;
    BOOL     hasNormalMap;
    BOOL     hasEmissive;
};

class BasicMaterial {
private:
	Vector4 colour;
	std::shared_ptr<class Texture> colourTex;

public:
	BasicMaterial();
	~BasicMaterial();

	void load(const tinygltf::Model& model, const tinygltf::MaterPbrMetallicRoughnessial& material, const char* basePath);
};

