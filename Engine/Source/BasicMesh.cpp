#include "Globals.h"
#include "BasicMesh.h"

#include "gltf_utils.h"

void BasicMesh::load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive)
{
	const auto& itPos = primitive.attributes.find("POSITION");
	if (itPos != primitive.attributes.end()) // If no position no geometry data
	{
		uint32_t numVertices = uint32_t(model.accessors[itPos->second].count);
		Vertex* vertices = new Vertex[numVertices];
		uint8_t* vertexData = (uint8_t*)vertices; // Casts Vertex Buffer to Bytes (uint8_t*) buffer
		loadAccessorData(vertexData + offsetof(Vertex, position), sizeof(Vector3), sizeof(Vertex),
			numVertices, model, itPos->second);
		loadAccessorData(vertexData + offsetof(Vertex, texCoord0), sizeof(Vector2), sizeof(Vertex),
			numVertices, model, primitive.attributes, "TEXCOORD_0");
	}
}