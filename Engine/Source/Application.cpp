#include "Globals.h"
#include "Application.h"
#include "Settings.h"
#include "ModuleInput.h"

#include "ModuleD3D12.h"
#include "ModuleRender.h"
#include "ModuleResources.h"
#include "ModuleSamplers.h"
#include "ModuleShaderDescriptors.h"
#include "ModuleRingBuffer.h"

#include "ModuleRTDescriptors.h"
#include "ModuleDSDescriptors.h"

#include "ModuleUI.h"
#include "ModuleEditor.h"
#include "ModuleCamera.h"

#include "Exercise1.h"
#include "Exercise2.h"
#include "Exercise3.h"
#include "Exercise4.h"
#include "Assigment1.h"
#include "Exercise5.h"
#include "Exercise6.h"
#include "Exercise7.h"
#include "Assigment2.h"

Application::Application([[maybe_unused]] int argc, [[maybe_unused]] wchar_t** argv, void* hWnd) {
    modules.push_back(moduleInput = new ModuleInput((HWND)hWnd));
    modules.push_back(d3d12 = new ModuleD3D12((HWND)hWnd));
    modules.push_back(moduleResources = new ModuleResources());
	modules.push_back(moduleSamplers = new ModuleSamplers());
    modules.push_back(moduleShaderDescriptors = new ModuleShaderDescriptors());
    modules.push_back(moduleRingBuffer = new ModuleRingBuffer());

    modules.push_back(moduleRTDescriptors = new ModuleRTDescriptors());
    modules.push_back(moduleDSDescriptors = new ModuleDSDescriptors());

    // Editor stuff
    modules.push_back(moduleEditor = new ModuleEditor());
    modules.push_back(moduleCamera = new ModuleCamera());
    
    // Ex1 - Paint the background
    //modules.push_back(new Exercise1());

    // Ex2 - Paint a triangle
    //modules.push_back(new Exercise2());
    
    // Ex3 - Paint triangle with transformations and grid
    //modules.push_back(new Exercise3());

    // Ex4 - Paint quad with texture
    //modules.push_back(new Exercise4());

	// Assigment 1
	//modules.push_back(new Assigment1());

    // Ex5 - Paint duck model
    //modules.push_back(new Exercise5());

    // Ex6 - Implement Phong Shading
    //modules.push_back(new Exercise6());

    // Ex7 - Implement Physically Based Phong Shading (Changing -> Render to texture)
    //modules.push_back(new Exercise7());

    // Assigment 2
    modules.push_back(new Assigment2());

    // Modulos wrappers de proceso
    modules.push_back(moduleUI = new ModuleUI());
    modules.push_back(moduleRender = new ModuleRender());

    settings = new Settings();
}

Application::~Application() {
    cleanUp();

	for(auto it = modules.rbegin(); it != modules.rend(); ++it)
    {
        delete *it;
    }

    delete settings;
}
 
bool Application::init() {
	bool ret = true;

    settings->LoadSettings();

    for (auto it = modules.begin(); it != modules.end() && ret; ++it)
        ret = (*it)->init();

    lastMilis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	return ret;
}

void Application::update()
{
    using namespace std::chrono_literals;

    // Update milis
    uint64_t currentMilis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    elapsedMilis = currentMilis - lastMilis;
    lastMilis = currentMilis;
    tickSum -= tickList[tickIndex];
    tickSum += elapsedMilis;
    tickList[tickIndex] = elapsedMilis;
    tickIndex = (tickIndex + 1) % MAX_FPS_TICKS;

    if (!app->paused)
    {
        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->update();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->preRender();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->render();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->postRender();
    }
}

bool Application::cleanUp()
{
	bool ret = true;

	for(auto it = modules.rbegin(); it != modules.rend() && ret; ++it)
		ret = (*it)->cleanUp();

	return ret;
}
