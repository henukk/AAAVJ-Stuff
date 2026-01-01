#pragma once
#include "Globals.h"
#include "Module.h"

class ModuleInput;
class ModuleD3D12;
class ModuleEditor;

class Settings;

class ModuleCamera : public Module {
private:
    ModuleInput* moduleInput;
    ModuleD3D12* moduleD3D12;
    ModuleEditor* moduleEditor;

    Settings* settings;

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

public:
    bool init() override;
    void update() override;

    // --- Getters ---
    const inline Matrix& getView() const { return view; }
    const inline Matrix& getProjection() const { return projection; }
    const inline Vector3& getPosition() const { return position; }

private:
    // Modes
    void panMode();
    void orbitMode();
    void zoomMode();
    void flythroughMode(float dt);

    // Helpers
    void handleMouseWheel();
    void handleAutoFocus();
    void rebuildViewProj();

    void syncYawPitchFromLookAt();
    void syncDistanceFromLookAt();
    Vector3 getForwardFromYawPitch() const;
    Vector3 getRightFromForward(const Vector3& fwd) const;
    void focusOnTarget();
};
