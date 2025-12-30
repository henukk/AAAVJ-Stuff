#include "Globals.h"
#include "BasicMesh.h"

#include "Application.h"
#include "ModuleResources.h"

#include "gltf_utils.h"

BasicMesh::BasicMesh() {

}

BasicMesh::~BasicMesh() {
	clean();
}

void BasicMesh::load(const tinygltf::Model& model,
    const tinygltf::Mesh& mesh,
    const tinygltf::Primitive& primitive)
{
    name = mesh.name;
    materialIndex = primitive.material;

    const auto itPos = primitive.attributes.find("POSITION");
    if (itPos == primitive.attributes.end())
        return;

    ModuleResources* resources = app->getModuleResources();

    const tinygltf::Accessor& posAcc = model.accessors[itPos->second];
    numVertices = uint32_t(posAcc.count);

    vertices = std::make_unique<Vertex[]>(numVertices);
    uint8_t* vertexData = reinterpret_cast<uint8_t*>(vertices.get());

    loadAccessorData(vertexData + offsetof(Vertex, position),
        sizeof(Vector3), sizeof(Vertex),
        numVertices, model, itPos->second);

    loadAccessorData(vertexData + offsetof(Vertex, texCoord0),
        sizeof(Vector2), sizeof(Vertex),
        numVertices, model,
        primitive.attributes, "TEXCOORD_0");

    loadAccessorData(vertexData + offsetof(Vertex, normal),
        sizeof(Vector3), sizeof(Vertex),
        numVertices, model,
        primitive.attributes, "NORMAL");

    vertexBuffer = resources->createDefaultBuffer(
        vertices.get(),
        numVertices * sizeof(Vertex),
        name.c_str());

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = numVertices * sizeof(Vertex);

    // --- Índices ---
    if (primitive.indices >= 0)
    {
        const tinygltf::Accessor& indAcc = model.accessors[primitive.indices];

        indexElementSize = tinygltf::GetComponentSizeInBytes(indAcc.componentType);
        numIndices = uint32_t(indAcc.count);

        indices = std::make_unique<uint8_t[]>(numIndices * indexElementSize);

        loadAccessorData(indices.get(),
            indexElementSize,
            indexElementSize,
            numIndices,
            model,
            primitive.indices);

        static const DXGI_FORMAT formats[3] =
        {
            DXGI_FORMAT_R8_UINT,
            DXGI_FORMAT_R16_UINT,
            DXGI_FORMAT_R32_UINT
        };

        indexBuffer = resources->createDefaultBuffer(
            indices.get(),
            numIndices * indexElementSize,
            name.c_str());

        indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        indexBufferView.Format = formats[indexElementSize >> 1];
        indexBufferView.SizeInBytes = numIndices * indexElementSize;
    }
}


void BasicMesh::draw (ID3D12GraphicsCommandList* commandList) const {
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	if (indexBuffer)
	{
		commandList->IASetIndexBuffer(&indexBufferView);
		commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
	}
	else
	{
		commandList->DrawInstanced(numVertices, 1, 0, 0);
	}
}

void BasicMesh::clean() {

}