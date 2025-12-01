#include "Globals.h"
#include "Application.h"

#include "ModuleInput.h"

#include "ModuleD3D12.h"
#include "ModuleRender.h"
#include "ModuleResources.h"

#include "ModuleUI.h"
#include "ModuleEditor.h"

#include "ModuleExercise1.h"
#include "ModuleExercise2.h"
#include "ModuleExercise3.h"


Application::Application(int argc, wchar_t** argv, void* hWnd) {
    modules.push_back(moduleInput = new ModuleInput((HWND)hWnd));
    modules.push_back(d3d12 = new ModuleD3D12((HWND)hWnd));
    modules.push_back(moduleResources = new ModuleResources());

    // Editor stuff
    //modules.push_back(moduleEditor = new ModuleEditor());
    //modules.push_back(moduleCamera = new ModuleCamera());
    
    // Ex1 - Paint the background
    //modules.push_back(new ModuleExercise1());

    // Ex2 - Paint a triangle
    //modules.push_back(new ModuleExercise2());
    
    // Ex3 - Paint triangle with transformations and grid
    modules.push_back(new ModuleExercise3());

    // Modulos wrappers de proceso
    modules.push_back(moduleUI = new ModuleUI());
    modules.push_back(moduleRender = new ModuleRender());
}

Application::~Application()
{
    cleanUp();

	for(auto it = modules.rbegin(); it != modules.rend(); ++it)
    {
        delete *it;
    }
}
 
bool Application::init()
{
	bool ret = true;

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
