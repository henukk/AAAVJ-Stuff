#pragma once

#include <span>
#include "BasicMesh.h"

namespace tinygltf { class Model; }

class BasicModel {
private:
    struct TextureInfo {
        ComPtr<ID3D12Resource> resource;
        UINT desc = 0;
        Vector4 colour;
    };

    struct Transforms {
        XMFLOAT4X4 model;
        XMFLOAT3X3 normal;
    };

    Matrix matrix = Matrix::Identity;
    std::unique_ptr<BasicMesh[]> meshes;
    uint32_t numMeshes = 0;
    uint32_t numMaterials = 0;
    std::string srcFile;

    ComPtr<ID3D12Resource> transformBuffer;

public:
    BasicModel();
    ~BasicModel();

    void load(const char* fileName);

    uint32_t getNumMeshes() const { return numMeshes; }
    uint32_t getNumMaterials() const { return numMaterials; }

    std::span<const BasicMesh> getMeshes() const { return std::span<const BasicMesh>(meshes.get(), numMeshes); }

    const Matrix& getModelMatrix() const { return matrix; }
    void setModelMatrix(const Matrix& m) { matrix = m; }
    Matrix getNormalMatrix() const
    {
        Matrix normal = matrix;
        normal.Translation(Vector3::Zero);
        normal.Invert();
        normal.Transpose();

        return normal;
    }

    const std::string& getSrcFile() const { return srcFile; }

private:
    void loadMeshes(const tinygltf::Model& model);
};