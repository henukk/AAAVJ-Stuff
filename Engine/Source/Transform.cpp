#include "Globals.h"
#include "Transform.h"
#include "ImGuizmo.h"

Transform::Transform() {
    localMatrix = Matrix::Identity;
}

static void Decompose(const Matrix& m, Vector3& t, Vector3& r, Vector3& s) {
    float mat[16];
    memcpy(mat, &m, sizeof(mat));

    float tf[3], rf[3], sf[3];
    ImGuizmo::DecomposeMatrixToComponents(mat, tf, rf, sf);

    t = Vector3(tf[0], tf[1], tf[2]);
    r = Vector3(rf[0], rf[1], rf[2]);
    s = Vector3(sf[0], sf[1], sf[2]);
}

static Matrix Recompose(const Vector3& t, const Vector3& r, const Vector3& s) {
    float mat[16];
    float tf[3] = { t.x, t.y, t.z };
    float rf[3] = { r.x, r.y, r.z };
    float sf[3] = { s.x, s.y, s.z };

    ImGuizmo::RecomposeMatrixFromComponents(tf, rf, sf, mat);
    return Matrix(mat);
}

void Transform::setPosition(const Vector3& p) {
    Vector3 t, r, s;
    Decompose(localMatrix, t, r, s);
    t = p;
    localMatrix = Recompose(t, r, s);
}

void Transform::setRotation(const Vector3& eulerDegrees) {
    Vector3 t, r, s;
    Decompose(localMatrix, t, r, s);
    r = eulerDegrees;
    localMatrix = Recompose(t, r, s);
}

void Transform::setScale(const Vector3& sc) {
    if (sc.x <= 0.f || sc.y <= 0.f || sc.z <= 0.f) return;

    Vector3 t, r, s;
    Decompose(localMatrix, t, r, s);
    s = sc;
    localMatrix = Recompose(t, r, s);
}

Vector3 Transform::getPosition() const {
    Vector3 t, r, s;
    Decompose(localMatrix, t, r, s);
    return t;
}

Vector3 Transform::getRotation() const {
    Vector3 t, r, s;
    Decompose(localMatrix, t, r, s);
    return r;
}

Vector3 Transform::getScale() const {
    Vector3 t, r, s;
    Decompose(localMatrix, t, r, s);
    return s;
}

Matrix Transform::getNormalMatrix() const {
    Matrix m = localMatrix;
    m.Translation(Vector3::Zero);
    return m.Invert().Transpose();
}

void Transform::setFromMatrix(const Matrix& m) {
    localMatrix = m;
}

void Transform::toImGuizmoMatrix(float out[16]) const {
    memcpy(out, &localMatrix, sizeof(float) * 16);
}

void Transform::fromImGuizmoMatrix(const float m[16]) {
    memcpy(&localMatrix, m, sizeof(float) * 16);
}
