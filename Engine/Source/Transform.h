#pragma once

#include <DirectXMath.h>
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Transform
{
public:
    Transform();

    void setPosition(const Vector3& p);
    void setRotation(const Vector3& r);
    void setScale(const Vector3& s);

    const Vector3& getPosition() const { return position; }
    const Vector3& getRotation() const { return rotation; }
    const Vector3& getScale() const { return scale; }

    Matrix  getLocalMatrix() const;
    Matrix  getNormalMatrix() const;

    void setFromMatrix(Matrix& m);
    Matrix toMatrix() const;

private:
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
};
