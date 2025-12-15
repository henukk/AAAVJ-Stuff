#include "Globals.h"
#include "ModuleCamera.h"

#include "Application.h"
#include "Settings.h"
#include "ModuleD3D12.h"
#include "ModuleEditor.h"
#include "ModuleUI.h"

#include "Mouse.h"
#include "Keyboard.h"

#include <algorithm>
#include <cmath>

bool ModuleCamera::init() {
    settings = app->getSettings();

    moduleInput = app->getModuleInput();
    moduleD3D12 = app->getModuleD3D12();
    moduleEditor = app->getModuleEditor();

    syncYawPitchFromLookAt();
    syncDistanceFromLookAt();

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

    int navMode = -1;
    if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION) {
        navMode = (int)moduleEditor->getCurrentNavigationMode();
    }
    if (navMode != lastNavMode) {
        if (lastNavMode == (int)ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {

            Vector3 forward = getForwardFromYawPitch();

            if (distanceToTarget < settings->camera.minDistance|| distanceToTarget > settings->camera.maxDistance)
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
                panMode();
                break;
            case (ModuleEditor::NAVIGATION_MODE::ORBIT):
                test = 2;
                orbitMode();
                break;
            case (ModuleEditor::NAVIGATION_MODE::ZOOM):
                test = 3;
                zoomMode();
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

        handleAutoFocus();
        handleMouseWheel();
    } else {
        lastWheel = Mouse::Get().GetState().scrollWheelValue;
    }

    Keyboard::State ks = Keyboard::Get().GetState();
    if (ks.IsKeyDown(Keyboard::Keys::F)) {
        focusOnTarget();
    }

    if (position == target) target.z += 0.001f;

    Vector3 viewTarget = target;

    if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION &&
        moduleEditor->getCurrentNavigationMode() == ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {
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
    distanceToTarget = std::clamp(distanceToTarget, settings->camera.minDistance, settings->camera.maxDistance);
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

void ModuleCamera::panMode() {
    Mouse::State ms = Mouse::Get().GetState();
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

    int dx = ms.x - panLastX;
    int dy = ms.y - panLastY;
    panLastX = ms.x;
    panLastY = ms.y;

    float sx = settings->camera.panInvertX ? -1.0f : 1.0f;
    float sy = settings->camera.panInvertY ? -1.0f : 1.0f;

    Vector3 forward = target - position;
    if (forward.LengthSquared() < 1e-8f)
        return;

    forward.Normalize();

    Vector3 right = forward.Cross(Vector3::Up);
    right.Normalize();

    Vector3 camUp = right.Cross(forward);
    camUp.Normalize();

    float dist = (target - position).Length();

    Vector3 delta =
        (right * (-float(dx) * sx) +
            camUp * (float(dy) * sy)) * (settings->camera.panSpeed * dist);

    position += delta;
    target += delta;

    syncDistanceFromLookAt();
}

void ModuleCamera::orbitMode() {
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

    yaw += dx * settings->camera.orbitSpeed;
    pitch += dy * settings->camera.orbitSpeed;
    pitch = std::clamp(pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

    Vector3 forward = getForwardFromYawPitch();
    position = target - forward * distanceToTarget;
}

void ModuleCamera::zoomMode() {
    Mouse::State ms = Mouse::Get().GetState();

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

    int dy = ms.y - zoomLastY;
    zoomLastX = ms.x;
    zoomLastY = ms.y;

    float dist = (target - position).Length();
    float delta = float(dy) * settings->camera.zoomDragSpeed * dist;

    distanceToTarget = std::clamp(distanceToTarget + delta, settings->camera.minDistance, settings->camera.maxDistance);

    Vector3 fwd = (target - position);
    if (fwd.LengthSquared() < 1e-8f) return;
    fwd.Normalize();
    position = target - fwd * distanceToTarget;
}

void ModuleCamera::handleMouseWheel() {
    Mouse::State ms = Mouse::Get().GetState();

    int delta = ms.scrollWheelValue - lastWheel;
    lastWheel = ms.scrollWheelValue;

    if (delta == 0) return;

    Vector3 forward = getForwardFromYawPitch();

    if (moduleEditor->getCurrentNavigationMode() == ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {
        float d = delta * settings->camera.zoomWheelSpeed * settings->camera.flyMoveSpeed;
        position += forward * d;

        distanceToTarget = std::clamp(distanceToTarget - d, settings->camera.minDistance, settings->camera.maxDistance);
        return;
    }

    distanceToTarget -= delta * settings->camera.zoomWheelSpeed;
    distanceToTarget = std::clamp(distanceToTarget, settings->camera.minDistance, settings->camera.maxDistance);

    position = target - forward * distanceToTarget;
}

void ModuleCamera::flythroughMode(float dt) {
    Keyboard::State ks = Keyboard::Get().GetState();
    Mouse::State ms = Mouse::Get().GetState();

    float speed = settings->camera.flyMoveSpeed;
    if (ks.LeftShift || ks.RightShift)
        speed *= settings->camera.flyBoostMult;

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

    float sx = settings->camera.flyInvertX ? -1.0f : 1.0f;
    float sy = settings->camera.flyInvertY ? -1.0f : 1.0f;

    yaw += dx * settings->camera.flyRotSpeed * sx;
    pitch += dy * settings->camera.flyRotSpeed * sy;

    pitch = std::clamp(pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

    Vector3 forward = getForwardFromYawPitch();
    Vector3 right = forward.Cross(Vector3::Up);
    right.Normalize();

    if (ks.IsKeyDown(Keyboard::Keys::W)) position += forward * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::S)) position -= forward * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::A)) position -= right * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::D)) position += right * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::Q)) position -= Vector3::Up * speed * dt;
    if (ks.IsKeyDown(Keyboard::Keys::E)) position += Vector3::Up * speed * dt;
}

void ModuleCamera::handleAutoFocus() {
    Keyboard::State ks = Keyboard::Get().GetState();
    if (ks.IsKeyDown(Keyboard::Keys::F)) {
        focusOnTarget();
    }
}

void ModuleCamera::focusOnTarget() {
    Vector3 focusPoint = Vector3(0.f, 0.f, 0.f);

    distanceToTarget = 10.0f;
    Vector3 forward = getForwardFromYawPitch();

    target = focusPoint;
    position = target - forward * distanceToTarget;
}
