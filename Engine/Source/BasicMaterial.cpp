#include "Globals.h"
#include "BasicMaterial.h"

#include "gltf_utils.h"

BasicMaterial::BasicMaterial() : colour(Vector4::One), colourTex(nullptr) { }

BasicMaterial::~BasicMaterial() { }

void BasicMaterial::load(const tinygltf::Model& model, const tinygltf::MaterPbrMetallicRoughnessial& material, const char* basePath) {
	colour = Vector4(float(material.baseColorFactor[0]), float(material.baseColorFactor[1]),
		float(material.baseColorFactor[2]), float(material.baseColorFactor[3]));
	if (material.baseColorTexture.index >= 0)
	{
		const tinygltf::Texture& texture = model.textures[material.baseColorTexture.index];
		const tinygltf::Image& image = model.images[texture.source];
		if (!image.uri.empty())
		{
			colourTex = app->getResources()->createTextureFromFile(std::string(basePath) + image.uri);
		}
	}
}