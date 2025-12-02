#pragma once
#include "Globals.h"
#include "Module.h"

class ModuleInput;
class ModuleD3D12;

class ModuleCamera : public Module {
private:
    ModuleInput* moduleInput;
    ModuleD3D12* moduleD3D12;
public:
    bool init() override;
    void update() override;

    // --- Getters ---
    const Matrix& getView()     const { return view; }
    const Matrix& getProjection() const { return projection; }

private:
    void drawGUI();

private:
    // Camera state
    Vector3 position{ 0.0f, 10.0f, 10.0f };
    Vector3 target{ 0.0f, 0.0f, 0.0f };
    Vector3 up{ 0.0f, 1.0f, 0.0f };

    Matrix view;
    Matrix projection;

};
