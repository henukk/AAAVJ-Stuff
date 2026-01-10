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

#include "ImGuizmo.h"
#include "BasicModel.h"

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
    ImVec2 sceneSize = moduleEditor->getSceneSize();
    if (sceneSize.x <= 1.0f || sceneSize.y <= 1.0f)
        return;

    unsigned width = (unsigned)sceneSize.x;
    unsigned height = (unsigned)sceneSize.y;

    float dt = app->getElapsedMilis() * 0.001f;

    int navMode = -1;
    if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION)
        navMode = (int)moduleEditor->getCurrentNavigationMode();

    if (navMode != lastNavMode) {
        if (lastNavMode == (int)ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {
            Vector3 forward = getForwardFromYawPitch();
            distanceToTarget = std::clamp(distanceToTarget,
                settings->camera.minDistance,
                settings->camera.maxDistance);

            target = position + forward * distanceToTarget;
            syncDistanceFromLookAt();
        }

        panFirst = orbitFirst = zoomFirst = flyFirst = true;
        lastNavMode = navMode;
    }

    bool allowCameraInput = moduleEditor->isSceneHovered() && !ImGuizmo::IsUsing();

    if (allowCameraInput) {
        if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION) {
            switch (moduleEditor->getCurrentNavigationMode()) {
            case ModuleEditor::NAVIGATION_MODE::PAN:        panMode(); break;
            case ModuleEditor::NAVIGATION_MODE::ORBIT:      orbitMode(); break;
            case ModuleEditor::NAVIGATION_MODE::ZOOM:       zoomMode(); break;
            case ModuleEditor::NAVIGATION_MODE::FREE_LOOK:  flythroughMode(dt); break;
            }
        }

        handleAutoFocus();
        handleMouseWheel();
    }
    else {
        lastWheel = Mouse::Get().GetState().scrollWheelValue;
    }

    Vector3 viewTarget = target;
    if (moduleEditor->getCurrentNavigationMode() == ModuleEditor::NAVIGATION_MODE::FREE_LOOK)
        viewTarget = position + getForwardFromYawPitch();

    view = Matrix::CreateLookAt(position, viewTarget, up);
    projection = Matrix::CreatePerspectiveFieldOfView(fovY, (float)width / (float)height, nearPlane, farPlane);
}


void ModuleCamera::rebuildViewProj() {
    ImVec2 sceneSize = moduleEditor->getSceneSize();
    if (sceneSize.x <= 1.0f || sceneSize.y <= 1.0f)
        return;

    unsigned width = static_cast<unsigned>(sceneSize.x);
    unsigned height = static_cast<unsigned>(sceneSize.y);

    Vector3 viewTarget = target;

    if (moduleEditor && moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION && moduleEditor->getCurrentNavigationMode() == ModuleEditor::NAVIGATION_MODE::FREE_LOOK) {
        viewTarget = position + getForwardFromYawPitch();
    }

    view = Matrix::CreateLookAt(position, viewTarget, up);
    projection = Matrix::CreatePerspectiveFieldOfView(
        fovY,
        (height > 0.0f) ? ((float)width / (float)height) : 1.0f,
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
    if (!ms.middleButton && !ms.leftButton) {
        panFirst = true;
        return;
    }

    ImVec2 sceneMin = moduleEditor->getSceneScreenMin();
    int mx = ms.x - (int)sceneMin.x;
    int my = ms.y - (int)sceneMin.y;

    if (panFirst) {
        panLastX = mx;
        panLastY = my;
        panFirst = false;
        return;
    }

    int dx = mx - panLastX;
    int dy = my - panLastY;
    panLastX = mx;
    panLastY = my;

    Vector3 forward = target - position;
    if (forward.LengthSquared() < 1e-8f) return;
    forward.Normalize();

    Vector3 right = forward.Cross(Vector3::Up);
    right.Normalize();
    Vector3 camUp = right.Cross(forward);

    float dist = (target - position).Length();

    Vector3 delta = (-right * (float)dx + camUp * (float)dy) * (settings->camera.panSpeed * dist);

    position += delta;
    target += delta;
}


void ModuleCamera::orbitMode() {
    Mouse::State ms = Mouse::Get().GetState();
    ImVec2 sceneMin = moduleEditor->getSceneScreenMin();

    int mx = ms.x - (int)sceneMin.x;
    int my = ms.y - (int)sceneMin.y;

    if (orbitFirst) {
        orbitLastX = mx;
        orbitLastY = my;
        orbitFirst = false;
        syncYawPitchFromLookAt();
        syncDistanceFromLookAt();
        return;
    }

    int dx = mx - orbitLastX;
    int dy = my - orbitLastY;
    orbitLastX = mx;
    orbitLastY = my;

    yaw += dx * settings->camera.orbitSpeed;
    pitch += dy * settings->camera.orbitSpeed;
    pitch = std::clamp(pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

    position = target - getForwardFromYawPitch() * distanceToTarget;
}


void ModuleCamera::zoomMode() {
    Mouse::State ms = Mouse::Get().GetState();
    if (!ms.rightButton) {
        zoomFirst = true;
        return;
    }

    ImVec2 sceneMin = moduleEditor->getSceneScreenMin();
    int my = ms.y - (int)sceneMin.y;

    if (zoomFirst) {
        zoomLastY = my;
        zoomFirst = false;
        syncDistanceFromLookAt();
        return;
    }

    int dy = my - zoomLastY;
    zoomLastY = my;

    distanceToTarget = std::clamp(
        distanceToTarget + dy * settings->camera.zoomDragSpeed,
        settings->camera.minDistance,
        settings->camera.maxDistance
    );

    position = target - getForwardFromYawPitch() * distanceToTarget;
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
    Mouse::State ms = Mouse::Get().GetState();
    Keyboard::State ks = Keyboard::Get().GetState();

    ImVec2 sceneMin = moduleEditor->getSceneScreenMin();
    int mx = ms.x - (int)sceneMin.x;
    int my = ms.y - (int)sceneMin.y;

    if (flyFirst) {
        flyLastX = mx;
        flyLastY = my;
        flyFirst = false;
        return;
    }

    int dx = mx - flyLastX;
    int dy = my - flyLastY;
    flyLastX = mx;
    flyLastY = my;

    yaw += dx * settings->camera.flyRotSpeed;
    pitch += dy * settings->camera.flyRotSpeed;
    pitch = std::clamp(pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

    Vector3 forward = getForwardFromYawPitch();
    Vector3 right = forward.Cross(Vector3::Up);
    right.Normalize();

    float speed = settings->camera.flyMoveSpeed * dt;
    if (ks.LeftShift) speed *= settings->camera.flyBoostMult;

    if (ks.IsKeyDown(Keyboard::Keys::W)) position += forward * speed;
    if (ks.IsKeyDown(Keyboard::Keys::S)) position -= forward * speed;
    if (ks.IsKeyDown(Keyboard::Keys::A)) position -= right * speed;
    if (ks.IsKeyDown(Keyboard::Keys::D)) position += right * speed;
}

void ModuleCamera::handleAutoFocus() {
    Keyboard::State ks = Keyboard::Get().GetState();
    if (ks.IsKeyDown(Keyboard::Keys::F)) {
        focusOnTarget();
    }
}

void ModuleCamera::focusOnTarget() {
    Vector3 focusPoint = Vector3::Zero;

    const BasicModel* selectedModel = moduleEditor->getSelectedGameObject();

    if (selectedModel) {
        focusPoint = selectedModel->getModelMatrix().Translation();
    }

    distanceToTarget = 10.0f;

    Vector3 forward = getForwardFromYawPitch();

    target = focusPoint;
    position = target - forward * distanceToTarget;
}
