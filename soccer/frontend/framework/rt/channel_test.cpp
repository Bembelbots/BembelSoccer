#include "channel.h"

#include <iostream>

int main() {
    Emitter<int> e;
    Listener<int> l;

    e.connect(&l);

    e.set(4);
    std::cout << l.get() << std::endl;

    e.set(5);
    e.set(6);
    std::cout << l.get() << std::endl;

    Listener<float> x;
    std::cout << "Emitter<int> connectable to Listener<float>? " << e.connectable(&x) << std::endl;

    return 0;
}
