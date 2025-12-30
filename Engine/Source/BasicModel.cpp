#include "Globals.h"
#include "BasicModel.h"

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


void BasicModel::load(const char* assetFileName)
{
	tinygltf::TinyGLTF gltfContext;
	tinygltf::Model model;
	std::string error, warning;
	bool loadOk = gltfContext.LoadASCIIFromFile(&model, &error, &warning, assetFileName);
	if (loadOk) 	{
		// Load model implementation
	}
	else LOG("Error loading %s: %s", assetFileName, error.c_str());
}