#pragma once
#include <set>
#include <string_view>

class BlackboardBase;

using BlackboardStorage = std::set<BlackboardBase*>;

class BlackboardRegistry {
public:
    // subscribe an derived blackboard to the global blackboard instance.
    static void subscribe(BlackboardBase &instance);

    // unsubscribe an blackboard from the global instance.
    static void unsubscribe(BlackboardBase &instance);

    static BlackboardBase* getBlackboardInstance(std::string_view);

    static BlackboardStorage& GetBlackboards();
};
