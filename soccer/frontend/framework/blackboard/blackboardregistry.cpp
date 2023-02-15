#include "blackboardregistry.h"
#include "blackboard.h"

#include "../logger/logger.h"
#include "../util/assert.h"

BlackboardStorage& BlackboardRegistry::GetBlackboards() {
    static BlackboardStorage storage;
    return storage;
}

void BlackboardRegistry::subscribe(BlackboardBase &instance) {

    std::string bbname = instance.getBlackboardName();

    LOG_INFO << "subscribing blackboard: " << bbname;

    // Assert that no blackboard is registered twice.
    jsassert(getBlackboardInstance(bbname) == nullptr);
    GetBlackboards().insert(&instance);
}

void BlackboardRegistry::unsubscribe(BlackboardBase &instance) {

    std::string bbname = instance.getBlackboardName();

    LOG_INFO << "unsubscribing blackboard: " << bbname;
    GetBlackboards().erase(&instance);
}


BlackboardBase* BlackboardRegistry::getBlackboardInstance(std::string_view identifier) {
    for (auto *i: GetBlackboards()) {
        // cppcheck-suppress useStlAlgorithm
        if (identifier == i->getBlackboardName()) {
            return (i);
        }
    }
    return nullptr;
}
