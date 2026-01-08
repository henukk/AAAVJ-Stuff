#include "Globals.h"

#include "Transform.h"

Transform::Transform() : position(0.f, 0.f, 0.f), rotation(0.f, 0.f, 0.f), scale(1.f, 1.f, 1.f) { }

void Transform::setPosition(const Vector3& p) {
    position = p;
}

void Transform::setRotation(const Vector3& r) {
    rotation = r;
}

void Transform::setScale(const Vector3& s) {
    scale = s;
}

Matrix Transform::getLocalMatrix() const {
    Matrix S = Matrix::CreateScale(scale);
    Matrix R =
        Matrix::CreateRotationX(XMConvertToRadians(rotation.x)) *
        Matrix::CreateRotationY(XMConvertToRadians(rotation.y)) *
        Matrix::CreateRotationZ(XMConvertToRadians(rotation.z));
    Matrix T = Matrix::CreateTranslation(position);

    return S * R * T;
}

Matrix Transform::getNormalMatrix() const {
    Matrix m = getLocalMatrix();
    m.Translation(Vector3::Zero);
    return m.Invert().Transpose();
}

Matrix Transform::toMatrix() const {
    return getLocalMatrix();
}

void Transform::setFromMatrix(Matrix& m) {
    Vector3 scl, pos;
    Quaternion q;
    m.Decompose(scl, q, pos);

    position = pos;
    scale = scl;

    Vector3 euler;
    euler = q.ToEuler();

    rotation = Vector3(
        XMConvertToDegrees(euler.x),
        XMConvertToDegrees(euler.y),
        XMConvertToDegrees(euler.z)
    );
}
