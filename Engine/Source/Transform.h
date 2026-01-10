#pragma once

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Transform {
private:
    Matrix localMatrix;

public:
    Transform();

    void setPosition(const Vector3& p);
    void setRotation(const Vector3& eulerDegrees);
    void setScale(const Vector3& s);

    Vector3 getPosition() const;
    Vector3 getRotation() const;
    Vector3 getScale() const;

    const Matrix& getLocalMatrix() const { return localMatrix; }
    Matrix getNormalMatrix() const;

    void setFromMatrix(const Matrix& m);
    Matrix toMatrix() const { return localMatrix; }

    // --- ImGuizmo ---
    void toImGuizmoMatrix(float out[16]) const;
    void fromImGuizmoMatrix(const float m[16]);
};
