#include "Globals.h"
#include "BasicModel.h"

#include "Application.h"
#include "ModuleResources.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE 
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

BasicModel::BasicModel()
{
}

BasicModel::~BasicModel()
{
}


void BasicModel::load(const char* assetFileName) {
	tinygltf::TinyGLTF gltfContext;
	tinygltf::Model model;
	std::string error, warning;
	bool loadOk = gltfContext.LoadASCIIFromFile(&model, &error, &warning, assetFileName);
	if (loadOk)
	{
		srcFile = assetFileName;
		loadMeshes(model);
	}
	else LOG("Error loading %s: %s", assetFileName, error.c_str());
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

    for (const tinygltf::Mesh& mesh : model.meshes) {
        for (const tinygltf::Primitive& primitive : mesh.primitives) {
            meshes[meshIndex++].load(model, mesh, primitive);
        }
    }
}