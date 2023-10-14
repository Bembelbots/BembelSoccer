#pragma once

#ifdef STANDALONE_BEHAVIOR

#include <blackboard/introspection.h>

class Blackboard;

namespace stab {

class BlackboardBaseShim : public Introspection {
    
public:
    BlackboardBaseShim(const std::string &name) 
        : Introspection(name) {}

};

} // namespace stab

#else

#include <framework/blackboard/blackboard.h>

namespace stab {

using BlackboardBaseShim = Blackboard;

} // namespace stab

#endif

// vim: set ts=4 sw=4 sts=4 expandtab:
