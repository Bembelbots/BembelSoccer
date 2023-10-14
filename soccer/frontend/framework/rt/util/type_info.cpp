#include "type_info.h"

#include <cxxabi.h>

std::string rt::prettyTypeName(TypeID id) {
    char *demangled = abi::__cxa_demangle(id.name(), 0, 0, nullptr);
    std::string name{demangled};
    free(demangled);
    return name;
}