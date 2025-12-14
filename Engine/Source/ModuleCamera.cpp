#include "Globals.h"
#include "ModuleCamera.h"

#include "Application.h"
#include "ModuleD3D12.h"
#include "ModuleEditor.h"
#include "ModuleUI.h"

#include "Mouse.h"
#include "Keyboard.h"

#include <algorithm>
#include <cmath>

bool ModuleCamera::init() {
    moduleInput = app->getModuleInput();
    moduleD3D12 = app->getModuleD3D12();
    moduleEditor = app->getModuleEditor();

    app->getModuleUI()->registerWindow([this]() { drawGUI(); });

    syncYawPitchFromLookAt();
    syncDistanceFromLookAt();

    // init wheel baseline
    Mouse::State mouseState = Mouse::Get().GetState();
    lastWheel = mouseState.scrollWheelValue;

    rebuildViewProj();
    return true;
}

void ModuleCamera::update() {
    ImGuiIO& io = ImGui::GetIO();

    unsigned width = moduleD3D12->getWindowWidth();
    unsigned height = moduleD3D12->getWindowHeight();
    float dt = app->getElapsedMilis() * 0.001f;

    // Reset per-mode mouse state when changing tool/mode
    int navMode = -1;
    if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION) {
        navMode = (int)moduleEditor->getCurrentNavigationMode();
    }
    if (navMode != lastNavMode) {
        if (lastNavMode == (int)ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {

            Vector3 forward = getForwardFromYawPitch();

            if (distanceToTarget < minDistance || distanceToTarget > maxDistance)
                distanceToTarget = 10.0f;

            target = position + forward * distanceToTarget;
            syncDistanceFromLookAt();
        }

        panFirst = orbitFirst = zoomFirst = flyFirst = true;
        lastNavMode = navMode;
    }

    if (!io.WantCaptureKeyboard && !io.WantCaptureMouse) {
        if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION) {
            switch (moduleEditor->getCurrentNavigationMode()) {
            case (ModuleEditor::NAVIGATION_MODE::PAN):
                test = 1;
                panMode(dt);
                break;
            case (ModuleEditor::NAVIGATION_MODE::ORBIT):
                test = 2;
                orbitMode(dt);
                break;
            case (ModuleEditor::NAVIGATION_MODE::ZOOM):
                test = 3;
                zoomMode(dt);
                break;
            case (ModuleEditor::NAVIGATION_MODE::FREE_LOOK):
                test = 4;
                flythroughMode(dt);
                break;
            default:
                test = -1;
                break;
            }
        }

        handleAutoFocus(dt);
        handleMouseWheel(dt);
    } else {
        lastWheel = Mouse::Get().GetState().scrollWheelValue;
    }

    Keyboard::State ks = Keyboard::Get().GetState();
    if (ks.IsKeyDown(Keyboard::Keys::F)) {
        focusOnTarget();
    }

    // Safety
    if (position == target) target.z += 0.001f;

    Vector3 viewTarget = target;

    if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION &&
        moduleEditor->getCurrentNavigationMode() == ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {
        viewTarget = position + getForwardFromYawPitch();
    }

    // Rebuild view/projection
    view = Matrix::CreateLookAt(position, viewTarget, up);
    projection = Matrix::CreatePerspectiveFieldOfView(
        fovY,
        (height > 0) ? (float(width) / float(height)) : 1.0f,
        nearPlane,
        farPlane
    );


}

void ModuleCamera::rebuildViewProj() {
    unsigned width = moduleD3D12->getWindowWidth();
    unsigned height = moduleD3D12->getWindowHeight();

    Vector3 viewTarget = target;

    if (moduleEditor && moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION && moduleEditor->getCurrentNavigationMode() == ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {
        viewTarget = position + getForwardFromYawPitch();
    }

    view = Matrix::CreateLookAt(position, viewTarget, up);
    projection = Matrix::CreatePerspectiveFieldOfView(
        fovY,
        (height > 0) ? (float(width) / float(height)) : 1.0f,
        nearPlane,
        farPlane
    );
}

void ModuleCamera::syncYawPitchFromLookAt() {
    Vector3 dir = target - position;
    if (dir.LengthSquared() < 1e-8f) {
        yaw = 0.0f;
        pitch = 0.0f;
        return;
    }
    dir.Normalize();
    yaw = atan2f(dir.z, dir.x);
    pitch = asinf(dir.y);
}

void ModuleCamera::syncDistanceFromLookAt() {
    distanceToTarget = (target - position).Length();
    distanceToTarget = std::clamp(distanceToTarget, minDistance, maxDistance);
}

Vector3 ModuleCamera::getForwardFromYawPitch() const {
    Vector3 fwd;
    fwd.x = cosf(pitch) * cosf(yaw);
    fwd.y = sinf(pitch);
    fwd.z = cosf(pitch) * sinf(yaw);
    fwd.Normalize();
    return fwd;
}

Vector3 ModuleCamera::getRightFromForward(const Vector3& fwd) const {
    Vector3 right = fwd.Cross(Vector3::Up);
    if (right.LengthSquared() < 1e-8f) right = Vector3::Right;
    right.Normalize();
    return right;
}

// -------------------- PAN --------------------
void ModuleCamera::panMode(float dt)
{
    Mouse::State ms = Mouse::Get().GetState();

    // Botón de pan (en tu caso viene de Alt+LMB desde el Editor)
    const bool down =
        (ms.middleButton == Mouse::ButtonStateTracker::HELD) ||
        (ms.leftButton == Mouse::ButtonStateTracker::HELD);

    if (!down) {
        panFirst = true;
        return;
    }

    if (panFirst) {
        panLastX = ms.x;
        panLastY = ms.y;
        panFirst = false;
        return;
    }

    // Mouse delta
    int dx = ms.x - panLastX;
    int dy = ms.y - panLastY;
    panLastX = ms.x;
    panLastY = ms.y;

    float sx = panInvertX ? -1.0f : 1.0f;
    float sy = panInvertY ? -1.0f : 1.0f;

    // Camera axes
    Vector3 forward = target - position;
    if (forward.LengthSquared() < 1e-8f)
        return;

    forward.Normalize();

    Vector3 right = forward.Cross(Vector3::Up);
    right.Normalize();

    Vector3 camUp = right.Cross(forward);
    camUp.Normalize();

    // Scale pan by distance for editor feel
    float dist = (target - position).Length();

    Vector3 delta =
        (right * (-float(dx) * sx) +
            camUp * (float(dy) * sy)) * (panSpeed * dist);

    // Apply
    position += delta;
    target += delta;

    // Keep orbit data coherent
    syncDistanceFromLookAt();
}


// -------------------- ORBIT --------------------
void ModuleCamera::orbitMode(float dt) {
    Mouse::State ms = Mouse::Get().GetState();

    if (orbitFirst) {
        orbitLastX = ms.x;
        orbitLastY = ms.y;
        orbitFirst = false;
        syncYawPitchFromLookAt();
        syncDistanceFromLookAt();
        return;
    }

    int dx = ms.x - orbitLastX;
    int dy = ms.y - orbitLastY;
    orbitLastX = ms.x;
    orbitLastY = ms.y;

    yaw += dx * orbitSpeed;
    pitch += dy * orbitSpeed;
    pitch = std::clamp(pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

    Vector3 forward = getForwardFromYawPitch();
    position = target - forward * distanceToTarget;
}


// -------------------- ZOOM (drag) --------------------
void ModuleCamera::zoomMode(float /*dt*/) {
    Mouse::State ms = Mouse::Get().GetState();

    // Zoom por arrastre con botón derecho (o el que prefieras)
    const bool down = (ms.rightButton == Mouse::ButtonStateTracker::HELD);

    if (!down) {
        zoomFirst = true;
        return;
    }

    if (zoomFirst) {
        zoomLastX = ms.x;
        zoomLastY = ms.y;
        zoomFirst = false;
        syncDistanceFromLookAt();
        return;
    }

    int dx = ms.x - zoomLastX;
    int dy = ms.y - zoomLastY;
    zoomLastX = ms.x;
    zoomLastY = ms.y;

    // Usamos dy principalmente (arriba/abajo). dx también podría valer si quieres.
    float dist = (target - position).Length();
    float delta = float(dy) * zoomDragSpeed * dist;

    distanceToTarget = std::clamp(distanceToTarget + delta, minDistance, maxDistance);

    Vector3 fwd = (target - position);
    if (fwd.LengthSquared() < 1e-8f) return;
    fwd.Normalize();
    position = target - fwd * distanceToTarget;
}

// -------------------- WHEEL ZOOM --------------------
void ModuleCamera::handleMouseWheel(float dt) {
    Mouse::State ms = Mouse::Get().GetState();

    int delta = ms.scrollWheelValue - lastWheel;
    lastWheel = ms.scrollWheelValue;

    if (delta == 0) return;

    Vector3 forward = getForwardFromYawPitch();

    // --- FREE LOOK: dolly FPS ---
    if (moduleEditor->getCurrentNavigationMode() == ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {
        float d = delta * zoomWheelSpeed * flyMoveSpeed;
        position += forward * d;

        // Mantén distanceToTarget razonable para cuando sueltes RMB
        distanceToTarget = std::clamp(distanceToTarget - d, minDistance, maxDistance);
        return;
    }

    // --- EDITOR MODES: pivot zoom ---
    distanceToTarget -= delta * zoomWheelSpeed;
    distanceToTarget = std::clamp(distanceToTarget, minDistance, maxDistance);

    position = target - forward * distanceToTarget;
}


// -------------------- FLYTHROUGH (FPS) --------------------
void ModuleCamera::flythroughMode(float dt) {
    Keyboard::State ks = Keyboard::Get().GetState();
    Mouse::State ms = Mouse::Get().GetState();

    float speed = flyMoveSpeed;
    if (ks.LeftShift || ks.RightShift)
        speed *= flyBoostMult;

    // Mouse look
    if (flyFirst) {
        flyLastX = ms.x;
        flyLastY = ms.y;
        flyFirst = false;
        return;
    }

    int dx = ms.x - flyLastX;
    int dy = ms.y - flyLastY;
    flyLastX = ms.x;
    flyLastY = ms.y;

    float sx = flyInvertX ? -1.0f : 1.0f;
    float sy = flyInvertY ? -1.0f : 1.0f;

    yaw += dx * flyRotSpeed * sx;
    pitch += dy * flyRotSpeed * sy;

    pitch = std::clamp(pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

    Vector3 forward = getForwardFromYawPitch();
    Vector3 right = forward.Cross(Vector3::Up);
    right.Normalize();

    // Movement
    if (ks.IsKeyDown(Keyboard::Keys::W)) position += forward * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::S)) position -= forward * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::A)) position -= right * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::D)) position += right * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::Q)) position -= Vector3::Up * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::E)) position += Vector3::Up * speed * dt;
}


// -------------------- AUTOFOCUS --------------------
void ModuleCamera::handleAutoFocus(float dt) {
    Keyboard::State ks = Keyboard::Get().GetState();
    if (ks.IsKeyDown(Keyboard::Keys::F)) {
        focusOnTarget();
    }
}

void ModuleCamera::focusOnTarget() {
    // Si no tienes selección, enfoca al origen
    Vector3 focusPoint = Vector3(0.f, 0.f, 0.f);

    distanceToTarget = 10.0f; // o según bounding box
    Vector3 forward = getForwardFromYawPitch();

    target = focusPoint;
    position = target - forward * distanceToTarget;
}

// -------------------- GUI --------------------
void ModuleCamera::drawGUI() {
    if (!ImGui::Begin("Camera")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Current Mode: %d", test);
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Position###CamPos", &position.x, 0.1f);
        ImGui::DragFloat3("Target###CamTarget", &target.x, 0.1f);

        if (ImGui::Button("Sync yaw/pitch from LookAt")) syncYawPitchFromLookAt();
        ImGui::SameLine();
        if (ImGui::Button("Sync distance from LookAt")) syncDistanceFromLookAt();

        ImGui::DragFloat("Distance###CamDist", &distanceToTarget, 0.05f, minDistance, maxDistance);
        ImGui::DragFloat("Yaw###CamYaw", &yaw, 0.005f);
        ImGui::DragFloat("Pitch###CamPitch", &pitch, 0.005f);
    }

    if (ImGui::CollapsingHeader("Projection")) {
        ImGui::DragFloat("FOV (rad)", &fovY, 0.001f, 0.1f, 2.5f);
        ImGui::DragFloat("Near", &nearPlane, 0.01f, 0.01f, 10.0f);
        ImGui::DragFloat("Far", &farPlane, 1.0f, 10.0f, 100000.0f);
    }

    if (ImGui::CollapsingHeader("Pan")) {
        ImGui::DragFloat("Pan Speed", &panSpeed, 0.0001f, 0.00001f, 0.05f);
        ImGui::Checkbox("Invert X###PanInvX", &panInvertX);
        ImGui::Checkbox("Invert Y###PanInvY", &panInvertY);
    }

    if (ImGui::CollapsingHeader("Orbit")) {
        ImGui::DragFloat("Orbit Speed", &orbitSpeed, 0.0001f, 0.0001f, 0.05f);
        ImGui::Checkbox("Invert X###OrbInvX", &orbitInvertX);
        ImGui::Checkbox("Invert Y###OrbInvY", &orbitInvertY);

        ImGui::DragFloat("Min Distance", &minDistance, 0.1f, 0.01f, 1000.0f);
        ImGui::DragFloat("Max Distance", &maxDistance, 0.1f, 0.1f, 5000.0f);
    }

    if (ImGui::CollapsingHeader("Zoom")) {
        ImGui::DragFloat("Zoom Drag Speed", &zoomDragSpeed, 0.0001f, 0.00001f, 0.1f);
        ImGui::DragFloat("Zoom Wheel Speed", &zoomWheelSpeed, 0.0001f, 0.000001f, 0.1f);
    }

    if (ImGui::CollapsingHeader("Flythrough")) {
        ImGui::DragFloat("Move Speed", &flyMoveSpeed, 0.1f, 0.1f, 500.0f);
        ImGui::DragFloat("Boost Mult", &flyBoostMult, 0.1f, 1.0f, 20.0f);
        ImGui::DragFloat("Rotation Speed", &flyRotSpeed, 0.0001f, 0.0001f, 0.05f);

        ImGui::Checkbox("Invert X###FlyInvX", &flyInvertX);
        ImGui::Checkbox("Invert Y###FlyInvY", &flyInvertY);

        ImGui::DragFloat("Pitch Clamp", &flyPitchClamp, 0.01f, 0.1f, XM_PIDIV2 - 0.01f);
    }

    ImGui::End();
}

