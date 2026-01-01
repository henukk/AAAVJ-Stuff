#include "Globals.h"
#include "BasicMaterial.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModuleShaderDescriptors.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE 

#include "tiny_gltf.h"
#include <imgui.h>


BasicMaterial::BasicMaterial() {

}

BasicMaterial::~BasicMaterial() {

}

void BasicMaterial::load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath, BasicMaterial::Type materialType) {
    ModuleResources* moduleResources = app->getModuleResources();
    ModuleShaderDescriptors* moduleShaderDescriptors = app->getModuleShaderDescriptors();

    name = material.name;
    this->materialType = materialType;

    Vector4 baseColour = Vector4(
        float(material.pbrMetallicRoughness.baseColorFactor[0]),
        float(material.pbrMetallicRoughness.baseColorFactor[1]),
        float(material.pbrMetallicRoughness.baseColorFactor[2]),
        float(material.pbrMetallicRoughness.baseColorFactor[3])
    );

    BOOL hasColourTexture = FALSE;

    auto loadTexture = [moduleResources] (int index, const tinygltf::Model& model, const char* basePath, bool defaultSRGB, ComPtr<ID3D12Resource>& outTex) -> BOOL
        {
            if (index < 0 || index >= int(model.textures.size()))
                return FALSE;

            const tinygltf::Texture& texture = model.textures[index];
            const tinygltf::Image& image = model.images[texture.source];
            if (!image.uri.empty())
            {
                outTex = moduleResources->createTextureFromFile(std::string(basePath) + image.uri, defaultSRGB);
                return TRUE;
            }
            return FALSE;
        };

    hasColourTexture = loadTexture(material.pbrMetallicRoughness.baseColorTexture.index, model, basePath, true, baseColourTex);
    
    if (materialType == BASIC) {
        materialData.basic.baseColour = baseColour;
        materialData.basic.hasColourTexture = hasColourTexture;
    } else if (materialType == PHONG) {
        materialData.phong.diffuseColour = baseColour;
        materialData.phong.hasDiffuseTex = hasColourTexture;
        materialData.phong.Kd = 0.85f;
        materialData.phong.Ks = 0.35f;
        materialData.phong.shininess = 32.0f;
    }

    textureTable = moduleShaderDescriptors->alloc(1);

    if (hasColourTexture) {
        moduleShaderDescriptors->createTextureSRV(textureTable, 0, baseColourTex.Get());
    } else {
        moduleShaderDescriptors->createNullTexture2DSRV(textureTable, 0);
    }

}
