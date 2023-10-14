#pragma once

#include "endpoints.h"
#include "linker.h"

#include <gsl/util>
#include <memory>
#include <type_traits>
#include <vector>
#include <functional>

namespace rt {

class Kernel;

class ModuleLoader {
public:
    ModuleLoader() = default;
    virtual ~ModuleLoader() = default;

    ModuleLoader(const ModuleLoader &) = delete;
    ModuleLoader(ModuleLoader &&) = delete;
    ModuleLoader &operator=(const ModuleLoader &) = delete;
    ModuleLoader &operator=(ModuleLoader &&) = delete;
    
    virtual void connect(Linker &) {};
    virtual void load(Kernel &) = 0;
    virtual void setup() {}
    virtual void stop() {}
};

class Module : public ModuleLoader {
public:
    Module() = default;
    virtual ~Module() = default;

    Module(const Module &) = delete;
    Module(Module &&) = delete;
    Module &operator=(const Module &) = delete;
    Module &operator=(Module &&) = delete;

    void connect(Linker &) override = 0;
    void load(Kernel &) override {}
    virtual void process() = 0;
};



} // namespace rt
