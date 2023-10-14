# initialize variable defaults if not set
if (NOT DEFINED NAO)
    set(NAO TRUE)
endif()
if (NOT DEFINED IWYU)
    set(IWYU FALSE)
endif()
if (NOT DEFINED JENKINS)
    set(JENKINS FALSE)
endif()

# silence boost warnings
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

# enable ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

# setup compiler/linker
if (NOT ${IWYU})
    set(CFLAGS_COMMON "-pipe -fdiagnostics-show-option -Wall -Wextra -Wno-strict-aliasing -ggdb3 -fpch-preprocess")
    set(CFLAGS_COMMON "${CFLAGS_COMMON} -fdiagnostics-color=auto")
    set(CFLAGS_COMMON "${CFLAGS_COMMON} -Wno-ignored-qualifiers")
    set(CFLAGS_COMMON "${CFLAGS_COMMON} -fmax-errors=5")
    if (${BB_ASAN})
        set(CFLAGS_COMMON "${CFLAGS_COMMON} -fsanitize=address")
    endif()
endif()

# disable select warnings
if (NOT ${IWYU})
    set(CFLAGS_COMMON "${CFLAGS_COMMON} -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-local-typedefs -Wno-strict-aliasing -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label")

    # tune for Atom E3845 CPU
    set(CFLAGS_COMMON "${CFLAGS_COMMON} -march=silvermont -mtune=silvermont -mfpmath=sse -msse4.2")
endif()

set(LINKER_FLAGS_COMMON "-Wl,--no-undefined -Wl,--build-id -rdynamic")
if (${BB_ASAN})
    set(LINKER_FLAGS_COMMON "${LINKER_FLAGS_COMMON} -fsanitize=address -static-libasan")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CFLAGS_COMMON} -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CFLAGS_COMMON} -O3")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS_COMMON}")

# set by cmake variable
message(STATUS "build type: " ${CMAKE_BUILD_TYPE})
