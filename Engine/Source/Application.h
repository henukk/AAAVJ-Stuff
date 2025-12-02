#pragma once

#include "Globals.h"

#include <array>
#include <vector>
#include <chrono>

class Module;
class ModuleInput;
class ModuleD3D12;
class ModuleRender;
class ModuleResources;
class ModuleUI;
class ModuleEditor;
class ModuleCamera;

class Application
{
public:

	Application(int argc, wchar_t** argv, void* hWnd);
	~Application();

	bool         init();
	void         update();
	bool         cleanUp();

    
    float                       getFPS() const { return 1000.0f * float(MAX_FPS_TICKS) / tickSum; }
    float                       getAvgElapsedMs() const { return tickSum / float(MAX_FPS_TICKS); }
    uint64_t                    getElapsedMilis() const { return elapsedMilis; }

    bool                        isPaused() const { return paused; }
    bool                        setPaused(bool p) { paused = p; return paused; }

    inline ModuleInput* getModuleInput() const noexcept { return moduleInput; }
    inline ModuleD3D12* getModuleD3D12() const noexcept { return d3d12; }
    inline ModuleRender* getModuleRender() const noexcept { return moduleRender; }
    inline ModuleResources* getModuleResources() const noexcept { return moduleResources; }
    inline ModuleUI* getModuleUI() const noexcept { return moduleUI; }
    inline ModuleEditor* getModuleEditor() const noexcept { return moduleEditor; }
    inline ModuleCamera* getModuleCamera() const noexcept { return moduleCamera; }

private:
    enum { MAX_FPS_TICKS = 30 };
    typedef std::array<uint64_t, MAX_FPS_TICKS> TickList;

    std::vector<Module*> modules;
    ModuleInput* moduleInput;
    ModuleD3D12* d3d12;
    ModuleRender* moduleRender;
    ModuleResources* moduleResources;
    ModuleUI* moduleUI;
    ModuleEditor* moduleEditor;
    ModuleCamera* moduleCamera;

    uint64_t  lastMilis = 0;
    TickList  tickList;
    uint64_t  tickIndex;
    uint64_t  tickSum = 0;
    uint64_t  elapsedMilis = 0;
    bool      paused = false;
};

extern Application* app;
