#pragma once
#include <framework/rt/module.h>

class RoboCup : public rt::NoThreadModule {
public:
    void load(rt::Kernel &) final;
    void connect(rt::Linker &) final;
private:
    std::vector<std::shared_ptr<rt::ModuleBase>> modules;
};
