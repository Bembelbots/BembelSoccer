#pragma once
#include <framework/rt/module.h>

class Behavior : public rt::NoThreadModule {
public:
    void load(rt::Kernel &) override;
    void connect(rt::Linker &) override;
private:
    std::shared_ptr<rt::Module> behavior;
};
