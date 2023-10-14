#pragma once

#include <string>
#include <string_view>

namespace stab {

struct BlackboardEntry {
    std::string name;
    std::string value;
    bool editable;
};

struct BBSetRequest {
    std::string name;
    std::string value;
};

} // namespace stab
