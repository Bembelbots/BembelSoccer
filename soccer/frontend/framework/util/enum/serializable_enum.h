/**
 * This header lets you define enums that can be converted to strings and vice versa.
 * The following macros are available:
 *
 *  - SERIALIZABLE_ENUM(name, ...): Defines a normal c enum.
 *
 *  - SERIALIZABLE_ENUM_I(name, baseClass, ...): Defines an enum with a custom
 *      base class.
 *
 *  - SERIALIZABLE_ENUM_CLASS(name, ...): Defines a scoped enum.
 *
 *  - SERIALIZABLE_ENUM_CLASS_I(name, baseClass, ...): Defines a scoped enum 
 *      with a custom base class.
 *
 *
 *  All these macros create template specialisations for your enum E of the
 *  following functions:
 *
 *      template<class E> 
 *      static const std::vector<E> enumValues:
 *        Vector that contains all elements of E in the order they
 *        were defined.
 *        
 *      template<class E> 
 *      static std::optional<E> strToEnum(std::string_view):
 *        Tries to convert a string to its respective element of E.
 *        Returns std::nullopt iff the conversion failed.
 *
 *      template<class E>
 *      static std::string_view enumToStr(E e):
 *        Converts the element to a string.
 *
 *  Additionally the stream operator << is overloaded to print the name of
 *  each element.
 *
 *
 * Usage example:
 *
 *     SERIALIZABLE_ENUM(Animals,
 *         (Dog),
 *         (Cat), 
 *         (Hamster)
 *      );
 *
 *      int main() {
 *          Animal c = Cat;
 *          std::cout << c << std::endl; // Prints 'Cat'
 *          std::optional<Animal> d = strToEnum("Dog");
 *          std::cout << enumToStr(*d) << std::endl; // Prints 'Dog'
 *          return 0;
 *      }
 *
 * Unfortunately the braces () around each element are necessary...
 * You can provide custom values for each value with this notation: (Dog) (2)
 */
#pragma once

#include <boost/preprocessor.hpp>

#include <ostream>
#include <string_view>
#include <type_traits>
#include <optional>
#include <vector>

template<typename T>
struct is_serializable_enum : std::false_type {};

template<typename T>
inline constexpr bool is_serializable_enum_v = is_serializable_enum<T>::value;

template<class E, typename = typename std::enable_if_t<is_serializable_enum_v<E>>>
static const std::vector<E> enumValues;

template<class E, typename = typename std::enable_if_t<is_serializable_enum_v<E>>>
static const std::vector<std::string_view> enumNames;

template<class E, typename = typename std::enable_if_t<is_serializable_enum_v<E>>>
[[maybe_unused]] std::optional<E> strToEnum(std::string_view name) {
    for (size_t i = 0; i < enumValues<E>.size(); i++) {
        if (enumNames<E>[i] == name) { // cppcheck-suppress internalAstError
            return enumValues<E>[i];
        }
    }
    return std::nullopt;
}

template<class E, typename = typename std::enable_if_t<is_serializable_enum_v<E>>>
[[maybe_unused]] std::string_view enumToStr(E e) {
    for (size_t i = 0; i < enumValues<E>.size(); i++) {
        if (enumValues<E>[i] == e) { // cppcheck-suppress internalAstError
            return enumNames<E>[i];
        }
    }
    return enumNames<E>[0]; // should never be reached
}

template<class E, typename = typename std::enable_if_t<is_serializable_enum_v<E>>>
[[maybe_unused]] std::ostream &operator<<(std::ostream &os, E e) {
    return os << enumToStr<E>(e);
}

#define SERIALIZABLE_ENUM(enumName, ...) \
    _ENUM(enumName, _NIL, __VA_ARGS__)

#define SERIALIZABLE_ENUM_I(enumName, baseClass, ...) \
    _ENUM(enumName, : baseClass, __VA_ARGS__)

#define SERIALIZABLE_ENUM_CLASS(enumName, ...) \
    _ENUM_CLASS(enumName, _NIL, __VA_ARGS__)

#define SERIALIZABLE_ENUM_CLASS_I(enumName, baseClass, ...) \
    _ENUM_CLASS(enumName, : baseClass, __VA_ARGS__)


#define _ENUM(enumName, defSuffix, ...) \
    _VARIADIC_TO_SEQ_ENUM(_ENUM_SEQ, enumName, defSuffix, __VA_ARGS__)

#define _ENUM_SEQ(enumName, defSuffix, memberSeq) \
    _ENUM_DEF(enum, enumName, defSuffix, memberSeq) \
    _FUNC_DEFS(enumName, _ENUM_NAME, _MEMBER_NAME_SEQ(memberSeq))

#define _ENUM_CLASS(enumName, defSuffix, ...) \
    _VARIADIC_TO_SEQ_ENUM(_ENUM_CLASS_SEQ, enumName, defSuffix, __VA_ARGS__)

#define _ENUM_CLASS_SEQ(enumName, defSuffix, memberSeq) \
    _ENUM_DEF(enum class, enumName, defSuffix, memberSeq) \
    _FUNC_DEFS(enumName, _ENUM_CLASS_NAME, _MEMBER_NAME_SEQ(memberSeq))

#define _VARIADIC_TO_SEQ_ENUM(seqFunc, enumName, defSuffix, ...) \
    seqFunc(enumName, defSuffix, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define _ENUM_DEF(defPrefix, enumName, defSuffix, memberSeq) \
    defPrefix enumName defSuffix{_TO_ENUM_MEMBERS(_ADD_HAS_VALUE_TO_ALL(memberSeq))}; \
    template<> \
    struct is_serializable_enum<enumName> : std::true_type {};

#define _NIL

#define _MEMBER_NAME BOOST_PP_SEQ_HEAD
#define _MEMBER_VALUE(mem) BOOST_PP_SEQ_ELEM(1, mem)
#define _MEMBER_HAS_VALUE(mem) BOOST_PP_EQUAL(BOOST_PP_SEQ_SIZE(mem), 2)
#define _MEMBER_VALUE_NOT_EMPTY(mem)  BOOST_PP_SEQ_ELEM(2, mem)

#define _ENUM_NAME(r, enumName, elem) elem,
#define _ENUM_CLASS_NAME(r, enumName, elem) enumName::elem,

#define _MEMBER_NAME_SEQ(memberSeq) \
    BOOST_PP_SEQ_FOR_EACH(_MEMBER_NAME_ELEM, _, memberSeq)

#define _MEMBER_NAME_ELEM(r, data, member) (_MEMBER_NAME(member))

#define _MAKE_MEMBER(r, data, mem) \
    BOOST_PP_IF(_MEMBER_VALUE_NOT_EMPTY(mem), \
      _MEMBER_NAME(mem) = _MEMBER_VALUE(mem), \
      _MEMBER_NAME(mem)),

#define _TO_ENUM_MEMBERS(memberSeq) \
    BOOST_PP_SEQ_FOR_EACH(_MAKE_MEMBER, _, memberSeq)

// Transform each member to a sequence of 3 elements, the third entry
// indicates, if this member has a custom value
#define _ADD_HAS_VALUE(r, data, mem) \
    BOOST_PP_IF(_MEMBER_HAS_VALUE(mem), \
        (mem (1)), \
        (mem data (0)))

#define _ADD_HAS_VALUE_TO_ALL(memSeq) \
    BOOST_PP_SEQ_FOR_EACH(_ADD_HAS_VALUE, (), memSeq)

#define _FUNC_DEFS(enumName, nameMaker, nameSeq) \
    _MAKE_ENUM_VALUES(enumName, nameMaker, nameSeq) \
    _MAKE_ENUM_NAMES(enumName, nameSeq)

#define _GET_NAME(r, enumName, elem) BOOST_PP_STRINGIZE(elem),

#define _MAKE_ENUM_VALUES(enumName, nameMaker, nameSeq) \
    template<> \
    const std::vector<enumName> enumValues<enumName> = {BOOST_PP_SEQ_FOR_EACH(nameMaker, enumName, nameSeq)};

#define _MAKE_ENUM_NAMES(enumName, nameSeq) \
    template<> \
    const std::vector<std::string_view> enumNames<enumName> = {BOOST_PP_SEQ_FOR_EACH(_GET_NAME, enumName, nameSeq)};
