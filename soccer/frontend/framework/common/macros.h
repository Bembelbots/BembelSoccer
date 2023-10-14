#pragma once

// No-operation statement that will be compiled away to nothing.
// Useful for disabling macros.
#define NOOP (static_cast<void>(0))

#ifdef NDEBUG
# define BUILDING_DEBUG   false
# define BUILDING_RELEASE true
#else
# define BUILDING_DEBUG   true
# define BUILDING_RELEASE false
#endif

#define ATTR_NO_RETURN __attribute__((noreturn))

// Tell gcc that a certain branch is likely to be taken or not taken.
// This can help the compiler to optimize.
#define LIKELY(x)   _COMPILER_EXPECT((x), true)
#define UNLIKELY(x) _COMPILER_EXPECT((x), false)

#define _COMPILER_EXPECT(x, b) __builtin_expect((x), b)

// vim: set ts=4 sw=4 sts=4 expandtab:
