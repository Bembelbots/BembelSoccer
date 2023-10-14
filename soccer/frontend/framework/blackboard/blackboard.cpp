#include "blackboard.h"
#include "blackboardregistry.h"

BlackboardBase::BlackboardBase(const std::string &name) :
    Introspection(name),
    _myName(name),
    context(std::make_shared<BlackboardContext>()) {
    BlackboardRegistry::subscribe(*this);
    registered = true;
}

BlackboardBase::~BlackboardBase() {
    if(registered) {
        BlackboardRegistry::unsubscribe(*this);
    }
}

std::string BlackboardBase::getBlackboardName() const {
    return _myName;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
