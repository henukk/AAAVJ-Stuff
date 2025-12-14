#pragma once
#include "Globals.h"
#include "Module.h"

class ModuleInput;
class ModuleD3D12;
class ModuleEditor;

class ModuleCamera : public Module {
private:
    ModuleInput* moduleInput = nullptr;
    ModuleD3D12* moduleD3D12 = nullptr;
    ModuleEditor* moduleEditor = nullptr;

public:
    bool init() override;
    void update() override;

    // --- Getters ---
    const Matrix& getView() const { return view; }
    const Matrix& getProjection() const { return projection; }

private:
    // Modes
    void panMode(float dt);
    void orbitMode(float dt);
    void zoomMode(float dt);
    void flythroughMode(float dt);

    // Helpers
    void handleMouseWheel(float dt);
    void handleAutoFocus(float dt);
    void rebuildViewProj();

    void syncYawPitchFromLookAt();
    void syncDistanceFromLookAt();
    Vector3 getForwardFromYawPitch() const;
    Vector3 getRightFromForward(const Vector3& fwd) const;
    void focusOnTarget();

    void drawGUI();

private:
    // Camera state
    Vector3 position{ 0.0f, 10.0f, 10.0f };
    Vector3 target{ 0.0f, 0.0f, 0.0f };
    Vector3 up = Vector3::Up;

    Matrix view;
    Matrix projection;

    // FPS angles (persistentes)
    float yaw = 0.0f;
    float pitch = 0.0f;

    // Orbit distance
    float distanceToTarget = 10.0f;

    // Debug / UI
    int test = -1;

    // --------- Tunables (UI) ----------
    // General
    float fovY = XM_PIDIV4;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    // Pan
    float panSpeed = 0.0025f;     // multiplicador por pixel * distance
    bool  panInvertX = false;
    bool  panInvertY = false;

    // Orbit
    float orbitSpeed = 0.005f;    // radianes por pixel
    bool  orbitInvertX = false;
    bool  orbitInvertY = true;

    // Zoom (drag + wheel)
    float zoomDragSpeed = 0.01f;  // unidades por pixel * distance
    float zoomWheelSpeed = 0.01f; // unidades por "wheel delta"
    float minDistance = 0.5f;
    float maxDistance = 200.0f;

    // Flythrough
    float flyMoveSpeed = 10.0f;   // units/sec
    float flyBoostMult = 4.0f;    // SHIFT
    float flyRotSpeed = 0.0035f; // radianes por pixel
    bool  flyInvertX = false;
    bool  flyInvertY = true;
    float flyPitchClamp = XM_PIDIV2 - 0.01f;

    // Mouse state per-mode
    bool panFirst = true;
    int  panLastX = 0, panLastY = 0;

    bool orbitFirst = true;
    int  orbitLastX = 0, orbitLastY = 0;

    bool zoomFirst = true;
    int  zoomLastX = 0, zoomLastY = 0;

    bool flyFirst = true;
    int  flyLastX = 0, flyLastY = 0;

    int lastWheel = 0;
    int lastNavMode = -999;
};
