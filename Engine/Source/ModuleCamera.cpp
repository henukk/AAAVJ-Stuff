    #include "Globals.h"
    #include "ModuleCamera.h"

    #include "Application.h"
    #include "ModuleD3D12.h"
    #include "ModuleEditor.h"
    #include "ModuleUI.h"

    #include "Mouse.h"
    #include "Keyboard.h"

    #include <algorithm>

    bool ModuleCamera::init() {
        moduleInput = app->getModuleInput();
        moduleD3D12 = app->getModuleD3D12();
        moduleEditor = app->getModuleEditor();

        app->getModuleUI()->registerWindow([this]() { drawGUI(); });

        Vector3 dir = target - position;
        distanceToTarget = dir.Length();
        dir.Normalize();

        float yawRad = atan2f(dir.z, dir.x);
        float pitchRad = asinf(dir.y);

        return true;
    }


    void ModuleCamera::update() {
        ImGuiIO& io = ImGui::GetIO();

        unsigned width = moduleD3D12->getWindowWidth();
        unsigned height = moduleD3D12->getWindowHeight();
        float dt = app->getElapsedMilis() * 0.001f;
    
        if (!io.WantCaptureKeyboard && !io.WantCaptureMouse) {
            if (moduleEditor->getCurrentSceneTool() == ModuleEditor::SCENE_TOOL::NAVIGATION) {
                switch (moduleEditor->getCurrentNavigationMode()) {
                    case (ModuleEditor::NAVIGATION_MODE::PAN):
                        test = 1;
                        panMode(dt);
                        break;
                    case (ModuleEditor::NAVIGATION_MODE::ORBIT):
                        test = 2;
                        //orbitMode(dt);
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
                }
            }

            handleAutoFocus(dt);
            handleMouseWheel(dt);
        }

        if (position == target)
            target.z += 0.001f;

        view = Matrix::CreateLookAt(position, target, up);
        projection = Matrix::CreatePerspectiveFieldOfView(XM_PIDIV4, float(width) / float(height), 0.1f, 1000.0f);
    }

    void ModuleCamera::panMode(float dt) {
        Mouse::State mouseState = Mouse::Get().GetState();

        static bool isFirstClick = true;
        static int initialMouseX = 0;
        static int initialMouseY = 0;

        // Velocidad de movimiento ajustada
        float mouseSpeed = 0.1f;

        // Si el botón del ratón está presionado (izquierdo o medio)
        if (mouseState.middleButton == Mouse::ButtonStateTracker::HELD || mouseState.leftButton == Mouse::ButtonStateTracker::HELD) {
            if (isFirstClick) {
                initialMouseX = mouseState.x;
                initialMouseY = mouseState.y;
                isFirstClick = false;
            }

            // Calcular el desplazamiento en los ejes X y Y
            int deltaX = mouseState.x - initialMouseX;
            int deltaY = mouseState.y - initialMouseY;

            // La distancia de la cámara al objetivo o al origen (dependiendo de cómo lo quieras)
            float distance = position.Length();  // Esto es la distancia desde la cámara al origen (0, 0, 0)

            // Calcular las direcciones de movimiento
            Vector3 forward = target - position;  // Dirección hacia el objetivo
            forward.Normalize();

            Vector3 right = forward.Cross(Vector3::Up);  // Dirección a la derecha (perpendicular a 'forward')
            right.Normalize();

            // Mover la cámara proporcional a la distancia al origen
            position += right * -deltaX * mouseSpeed * distance;  // Movimiento en el eje X (izquierda/derecha)
            position += Vector3::Up * deltaY * mouseSpeed * distance;  // Movimiento en el eje Y (arriba/abajo)

            // Mantener el target sincronizado con la cámara
            target += right * -deltaX * mouseSpeed * distance;
            target += Vector3::Up * deltaY * mouseSpeed * distance;

            // Actualizar la posición del ratón para el siguiente frame
            initialMouseX = mouseState.x;
            initialMouseY = mouseState.y;
        }
        else {
            isFirstClick = true;
        }
    }



    void ModuleCamera::handleMouseWheel(float dt) {
        // Obtener el estado del ratón
        Mouse::State mouseState = Mouse::Get().GetState();

        float increment = 0.05f; // Mayor velocidad de cambio para el zoom

        if (mouseState.scrollWheelValue != 0) {
            float targetDistance = distanceToTarget + (mouseState.scrollWheelValue > 0 ? -increment : increment);

            // Limitar la distancia de zoom
            targetDistance = std::clamp(targetDistance, 5.0f, 50.0f);

            // Interpolación suave entre la distancia actual y la nueva
            distanceToTarget = std::lerp(distanceToTarget, targetDistance, 0.1f);

            Vector3 forward = target - position;
            forward.Normalize();

            position = target - forward * distanceToTarget;
        }
    }



    void ModuleCamera::zoomMode(float dt) {

    }

    void ModuleCamera::handleAutoFocus(float dt) {

    }


    void ModuleCamera::drawGUI() {
        if (ImGui::Begin("Camera")) {
            if (ImGui::CollapsingHeader("Position"))
            {
                ImGui::DragFloat3("Position###CamPos", &position.x, 0.1f);
		    }
            if (ImGui::CollapsingHeader("Target"))
            {
                ImGui::DragFloat3("Target###CamTarget", &target.x, 0.1f);
		    }
            ImGui::DragFloat("Distance", &distanceToTarget, 0.1f);
			ImGui::Text("Current Mode: %d", test);
        }
        ImGui::End();
    }

    void ModuleCamera::flythroughMode(float dt) {
        Keyboard::State keyboardState = Keyboard::Get().GetState();

        float movementSpeed = 10.0f; // Ajusta esto según el comportamiento que desees
        float rotationSpeed = 0.01f;  // Ajusta la velocidad de rotación de la cámara

        Vector3 forward = target - position;
        forward.Normalize();

        Vector3 right = forward.Cross(Vector3::Up);
        right.Normalize();

        Vector3 up = Vector3::Up;

        // Movimiento hacia adelante y hacia atrás (W, S)
        if (keyboardState.IsKeyDown(Keyboard::Keys::W)) {
            position += forward * movementSpeed * dt;
            target += forward * movementSpeed * dt;
        }
        if (keyboardState.IsKeyDown(Keyboard::Keys::S)) {
            position -= forward * movementSpeed * dt;
            target -= forward * movementSpeed * dt;
        }

        // Movimiento lateral (A, D)
        if (keyboardState.IsKeyDown(Keyboard::Keys::A)) {
            position -= right * movementSpeed * dt;
            target -= right * movementSpeed * dt;  // Mantener el target sincronizado
        }
        if (keyboardState.IsKeyDown(Keyboard::Keys::D)) {
            position += right * movementSpeed * dt;
            target += right * movementSpeed * dt;  // Mantener el target sincronizado
        }

        // Movimiento hacia arriba y hacia abajo (Q, E)
        if (keyboardState.IsKeyDown(Keyboard::Keys::Q)) {
            position += up * movementSpeed * dt;
            target += up * movementSpeed * dt;  // Mantener el target sincronizado
        }
        if (keyboardState.IsKeyDown(Keyboard::Keys::E)) {
            position -= up * movementSpeed * dt;
            target -= up * movementSpeed * dt;  // Mantener el target sincronizado
        }

        // Rotación: movimiento del ratón para rotar la cámara
        Mouse::State mouseState = Mouse::Get().GetState();

        static bool isFirstClick = true;
        static float lastX = 0.0f;
        static float lastY = 0.0f;

        // Verificar si es el primer clic del ratón
        if (isFirstClick) {
            // Guardamos la posición inicial del ratón para calcular el delta cuando se mueva por primera vez
            lastX = mouseState.x;
            lastY = mouseState.y;
            isFirstClick = false; // Después del primer movimiento, desactivamos el primer clic
        }

        // Obtener la diferencia en los movimientos del ratón
        float deltaX = -float(mouseState.x - lastX);
        float deltaY = -float(mouseState.y - lastY);

        // Ajustar la rotación en función del movimiento del ratón
        float yaw = deltaX * rotationSpeed;
        float pitch = deltaY * rotationSpeed;

        // Rotar la dirección del "target"
        Vector3 direction = target - position;
        Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0.0f);

        direction = Vector3::TransformNormal(direction, rotationMatrix);
        target = position + direction;

        // Actualizar la posición del ratón para el siguiente frame
        lastX = mouseState.x;
        lastY = mouseState.y;
    }

