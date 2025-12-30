#pragma once

namespace tinygltf { class Model;  struct Mesh; struct Primitive; }

class BasicMesh {
public:
    struct Vertex
    {
        Vector3 position = Vector3::Zero;
        Vector2 texCoord0 = Vector2::Zero;
    };

public:
	void load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive);
};

