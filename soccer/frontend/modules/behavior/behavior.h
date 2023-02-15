#pragma once
#include <framework/rt/module.h>

class Behavior : public rt::ModuleLoader {
public:
    void load(rt::Kernel &) override;
private:
    std::shared_ptr<rt::Module> behavior;
};
