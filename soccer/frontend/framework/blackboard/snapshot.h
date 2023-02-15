#pragma once
#include <memory>

class SnapshotableBlackboard;

struct SnapshotTag {};

template<typename Blackboard>
class Snapshot {
public:
    using blackboard_type = Blackboard;
    using self_type = Snapshot<blackboard_type>;
    using self_ref = self_type&;

    struct SnapshotData : public blackboard_type {
        SnapshotData()
            : blackboard_type(SnapshotTag{}) {
            this->setup();
        }

        SnapshotData& operator=(const blackboard_type& other) {
            blackboard_type* bb = this;
            *bb = other;
            this->registered = false;
            return *this;
        }
    };

    Snapshot() = default;

    Snapshot<blackboard_type>& operator=(const Snapshot<blackboard_type>&) = default;

    Snapshot(const Snapshot<blackboard_type> &other) : data(other.data ){
    }

    const blackboard_type* get() const {
        return &data;
    }

    operator const blackboard_type*() const {
        return get();
    }

    self_ref operator=(const blackboard_type &other) {
        data = other;
        return *this;
    }
    
    const blackboard_type* operator->() const {
        return get();
    }

private:
    SnapshotData data;
};
