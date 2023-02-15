#pragma once

#include <string>
#include "../serialize/serializer.h"

class BlackboardDataContainer {
public:

    template<typename T>
    BlackboardDataContainer(const std::string &key,  const T &value){
        auto result = serialize(key, value);
        _key = result.first;
        _value = result.second;
    }

    std::string getKey() const;
    std::string getValue() const;

private:
    std::string _key;
    std::string _value;
};

// vim: set ts=4 sw=4 sts=4 expandtab:
