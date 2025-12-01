#pragma once
#include "Module.h"
#include <vector>
#include <functional>

class ModuleD3D12;

class ModuleRender : public Module
{
public:
    using RenderCallback = std::function<void(ID3D12GraphicsCommandList*)>;

private:
    ModuleD3D12* d3d12 = nullptr;

    std::vector<RenderCallback> worldPasses;
    std::vector<RenderCallback> uiPasses;

public:
    bool init() override;
    void preRender() override;
    void render() override;
    bool cleanUp() override;

    void registerWorldPass(const RenderCallback& cb);
    void registerUIPass(const RenderCallback& cb);
};
