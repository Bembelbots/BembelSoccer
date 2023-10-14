#pragma once

#include "endpoints.h"
#include "linker.h"
#include "util/util.h"

#include <gsl/util>
#include <memory>
#include <type_traits>
#include <vector>
#include <functional>

namespace rt {

class Kernel;

class ModuleBase {
public:
    virtual ~ModuleBase() = default;

    virtual ModuleTag type() const {
        return ModuleTag::Normal;
    }

    virtual bool disabled() const {
        return false;
    }

    virtual void connect(Linker &) = 0;
    virtual void load(Kernel &) {}
    virtual void process() {}
    virtual void setup() {}
    virtual void stop() {}
};

class Module : public ModuleBase {
public:
    Module() = default;
    virtual ~Module() = default;

    RT_DISABLE_COPY(Module)

    void process() override = 0;
};

class NoThreadModule : public ModuleBase {
public:
    NoThreadModule() = default;
    virtual ~NoThreadModule() = default;
    
    RT_DISABLE_COPY(NoThreadModule)
    
    ModuleTag type() const override {
        return ModuleTag::NoThread;
    }
};

} // namespace rt
