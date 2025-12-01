#pragma once
#include "Module.h"

#include <functional>
#include <vector>

class ImGuiPass;
class ModuleD3D12;

class ModuleUI : public Module
{
public:
    using WindowCallback = std::function<void()>;

private:
    ModuleD3D12* d3d12 = nullptr;
    ImGuiPass* imguiPass = nullptr;

    std::vector<WindowCallback> windows;

public:
    bool init() override;
    void preRender() override;
    bool cleanUp() override;

    void registerWindow(const WindowCallback& callback);
};
