#include "kernel.h"
#include "module.h"

#include <iostream>
#include <thread>

struct FooMessage {
    int x;
    int y;

    bool event1;
};

struct BarMessage {
    float a;
    float b;
};

struct QuxMessage {
    float x = 0;
    float y = 0;
    float z = 0;
};

struct FooCommand {};
struct DoFoo {
    float x;
};
REGISTER_COMMAND(DoFoo, FooCommand);

class Bar : public Module {

public:
    BarMessage bar;

    void info(ModuleMeta &meta) override {
        meta.name = "Bar";
        meta.provides(&bar);
    }

    void process() override {
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
        std::cout << "Bar::process(): " << bar.a << std::endl;
        bar.a++;
    }
};

class Qux : public Module {

public:
    FooMessage foo;
    BarMessage bar;
    QuxMessage qux;

    Dispatcher<FooCommand> cmds;

    Qux() = default;

    void info(ModuleMeta &meta) override {
        meta.name = "Qux";
        meta.requires(&foo);
        meta.listens(&bar);
        meta.provides(&qux);
        meta.issues(&cmds);
    }

    void process() override {
        cmds.enqueue(DoFoo{qux.x});
        qux.x += 1.5f;
        std::this_thread::sleep_for(std::chrono::seconds{1});
        // ...
        std::cout << "Qux: foo.x = " << foo.x << ", foo.y = " << foo.y << std::endl;
    }
};

class Foo : public Module {

public:
    BarMessage bar;
    FooMessage foo;
    Handler<FooCommand> cmds;

    Foo() = default;

    void info(ModuleMeta &meta) override {
        meta.name = "Foo";
        meta.listens(&bar);
        meta.provides(&foo);
        meta.handles(&cmds);
        cmds.connect<DoFoo, &Foo::doFoo>(this);
    }

    void process() override {
        cmds.update();
        std::this_thread::sleep_for(std::chrono::seconds{2});
        std::cout << "Foo::process(): bar.a = " << bar.a << std::endl;
        foo.x += 1;
        foo.y -= 1;
    }

    void doFoo(DoFoo f) { std::cout << "Received command DoFoo with f.x = " << f.x << std::endl; }
};

int main() {
    Kernel f;
    f.load<Qux>();
    f.load<Bar>();
    f.load<Foo>();

    // f.execSequential(10);
    f.exec();
}
