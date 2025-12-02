#pragma once

#include "Globals.h"

class Module
{
public:
	Module() {}
    virtual ~Module() {}
	virtual bool init() { return true; }
	virtual void update() {}
    virtual void preRender() {}
    virtual void render() {}
    virtual void postRender() {}
    virtual bool cleanUp() { return true; }
};
