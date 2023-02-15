#pragma once
#include <framework/rt/module.h>

class RoboCup : public rt::ModuleLoader {
public:
    void load(rt::Kernel &) final;
private:
    std::vector<std::shared_ptr<rt::Module>> modules;
    std::vector<std::shared_ptr<rt::ModuleLoader>> loaders;
};
