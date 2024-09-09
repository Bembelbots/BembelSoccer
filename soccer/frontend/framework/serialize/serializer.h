#pragma once
#include <string>
#include <vector>
#include <cstring>
#include "../util/converter.h"

std::string base64_encode(unsigned char const *, unsigned int len);
std::string base64_decode(std::string const &encoded_string);

using SerializedT = std::pair<std::string, std::string>;

template<typename T>
struct Serializer {
    SerializedT operator()(const std::string &key,  const T &value) {
        auto _key = key;
        unsigned char *tmp = new unsigned char[sizeof(T)];
        std::memcpy(tmp, &value, sizeof(T));
        auto _value = base64_encode(tmp, sizeof(T));
        delete[] tmp;
        return {_key, _value};
    }
};

template<typename T>
SerializedT serialize(const std::string &key, const T &value){
    static Serializer<T> serializer;
    return serializer(key, value);
}

template<typename T>
struct Serializer<std::vector<T>> {
    SerializedT operator()(const std::string &key,  const std::vector<T> &values) const {
        const std::string delim = ";";
        std::string _key = key;
        std::string _value = "";
        for (auto &value : values) {
            // cppcheck-suppress useStlAlgorithm
            _value += serialize("", value).second + delim;
        }
        if(!_value.empty()) {
            _value = _value.substr(0, _value.length() - 1);
        }
        return {_key, _value};
    }
};

#define SERIALIZE(TYPE, ...) \
template<> \
struct Serializer<TYPE> { \
    SerializedT operator()(const std::string &key,  const TYPE &value) const \
    __VA_ARGS__ \
};

SERIALIZE(bool, {
    auto _key = key;
    std::string _value = "";
    if (value) {
        _value = "yes";
    } else {
        _value = "no";
    }

    return {_key, _value};
});

SERIALIZE(int, {
    return {key, str(value)};
});

SERIALIZE(float, {
    return {key, str(value, 5)};
});

SERIALIZE(std::string, {
    return {key, value};
});
