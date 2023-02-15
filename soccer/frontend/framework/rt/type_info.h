#pragma once

#include <string>
#include <typeinfo>
#include <typeindex>

namespace rt {

using TypeID = std::type_index;

// TODO this class is useless..
template<typename T>
struct TypeInfo {

    static TypeID id() { return std::type_index(typeid(T)); }
    static const char *name() { return typeid(T).name(); }
};

std::string prettyTypeName(TypeID id);

} // namespace rt