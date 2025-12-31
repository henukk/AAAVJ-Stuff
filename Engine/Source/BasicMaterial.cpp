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

void BasicMaterial::load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath) {
    name = material.name;
    Vector4 baseColour = Vector4(float(material.pbrMetallicRoughness.baseColorFactor[0]),
        float(material.pbrMetallicRoughness.baseColorFactor[1]),
        float(material.pbrMetallicRoughness.baseColorFactor[2]),
        float(material.pbrMetallicRoughness.baseColorFactor[3]));

    BOOL hasColourTexture = FALSE;

    auto loadTexture = [](int index, const tinygltf::Model& model, const char* basePath, bool defaultSRGB, ComPtr<ID3D12Resource>& outTex) -> BOOL
        {
            if (index < 0 || index >= int(model.textures.size()))
                return FALSE;

            const tinygltf::Texture& texture = model.textures[index];
            const tinygltf::Image& image = model.images[texture.source];
            if (!image.uri.empty())
            {
                outTex = app->getModuleResources()->createTextureFromFile(std::string(basePath) + image.uri, defaultSRGB);
                return TRUE;
            }
            return FALSE;
        };

    hasColourTexture = loadTexture(material.pbrMetallicRoughness.baseColorTexture.index, model, basePath, true, baseColourTex);
    
    materialData.baseColour = baseColour;
    materialData.hasColourTexture = hasColourTexture;
    // Descriptors 

    ModuleShaderDescriptors* descriptors = app->getModuleShaderDescriptors();
    textureTable = descriptors->alloc(1);

    if (hasColourTexture) {
        descriptors->createTextureSRV(textureTable, 0, baseColourTex.Get());
    } else {
        descriptors->createNullTexture2DSRV(textureTable, 0);
    }

}
