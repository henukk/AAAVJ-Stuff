#pragma once
#include "Globals.h"
#include "Module.h"

class ModuleInput;
class ModuleD3D12;
class ModuleEditor;

class ModuleCamera : public Module {
private:
    ModuleInput* moduleInput;
    ModuleD3D12* moduleD3D12;
    ModuleEditor* moduleEditor;
public:
    bool init() override;
    void update() override;

    // --- Getters ---
    const Matrix& getView()     const { return view; }
    const Matrix& getProjection() const { return projection; }

private:
    void panMode(float dt);

    void handleAutoFocus(float dt);
    void handleMouseWheel(float dt);
    void zoomMode(float dt);
    void flythroughMode(float dt);

    void drawGUI();

private:
    Vector3 position{ 0.0f, 10.0f, 10.0f };
    Vector3 target{ 0.0f, 0.0f, 0.0f };

    Vector3 up = Vector3::Up;

    Matrix view;
    Matrix projection;

    // Camera control
    float distanceToTarget;
    int test;

};
