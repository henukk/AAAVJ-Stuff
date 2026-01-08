#include "Globals.h"
#include "BasicModel.h"

#include "Application.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleResources.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE 
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include "EditorConsole.h"

#include "Transform.h"

void BasicModel::load(const char* fileName, const char* basePath, BasicMaterial::Type materialType) {
    tinygltf::TinyGLTF gltfContext;
    tinygltf::Model model;
    std::string error, warning;

    bool loadOk = gltfContext.LoadASCIIFromFile(&model, &error, &warning, fileName);

    if (loadOk) {
        srcFile = fileName;
        loadMaterials(model, basePath, materialType);
        loadMeshes(model);
    } else {
        LOG("Error loading %s: %s", fileName, error.c_str());
        Console->PrintError("Error loading %s: %s", fileName, error.c_str());
    }
}

void BasicModel::loadMeshes(const tinygltf::Model& model) {
    auto countPrimitves = [](const tinygltf::Model& m) -> size_t {
        size_t count = 0;
        for (const tinygltf::Mesh& mesh : m.meshes) {
            count += mesh.primitives.size();
        }
        return count;
        };

    numMeshes = uint32_t(countPrimitves(model));

    meshes = std::make_unique<BasicMesh[]>(numMeshes);
    int meshIndex = 0;

    for (const tinygltf::Mesh& mesh : model.meshes)
    {
        for (const tinygltf::Primitive& primitive : mesh.primitives)
        {
            meshes[meshIndex++].load(model, mesh, primitive);
        }
    }
}

void BasicModel::loadMaterials(const tinygltf::Model& model, const char* basePath, BasicMaterial::Type materialType) {
    ModuleShaderDescriptors* descriptors = app->getModuleShaderDescriptors();
    ModuleResources* resources = app->getModuleResources();

    numMaterials = uint32_t(model.materials.size());
    materials = std::make_unique<BasicMaterial[]>(numMaterials);
    int materialIndex = 0;

    for (const tinygltf::Material& material : model.materials)
    {
        materials[materialIndex++].load(model, material, basePath, materialType);
    }
}

Matrix BasicModel::getModelMatrix() const {
    return transform ? transform->getLocalMatrix() : Matrix::Identity;
}

Matrix BasicModel::getNormalMatrix() const {
    return transform ? transform->getNormalMatrix() : Matrix::Identity;
}