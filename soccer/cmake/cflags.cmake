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

# strip file paths in debug symbols
set(CFLAGS_COMMON "${CFLAGS_COMMON} -fdebug-prefix-map=${CMAKE_SOURCE_DIR}=soccer")
set(CFLAGS_COMMON "${CFLAGS_COMMON} -fdebug-prefix-map=${CTC_DIR}=CTC")

# disable select warnings
if (NOT ${IWYU})
    set(CFLAGS_COMMON "${CFLAGS_COMMON} -Wno-unused-parameter -Wno-unused-but-set-parameter -Wno-unused-local-typedefs -Wno-strict-aliasing -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label")

    #set(CFLAGS_COMMON "${CFLAGS_COMMON} -Wno-unused-command-line-argument") # Required for clang because it doesn't know the tree-parallize flag...
    if (${V6})
        # tune for Atom E3845 CPU
        set(CFLAGS_COMMON "${CFLAGS_COMMON} -march=silvermont -mtune=silvermont -mfpmath=sse -msse4.2")
        #set(CFLAGS_COMMON "${CFLAGS_COMMON} -fopenmp -ftree-parallelize-loops=4")
    elseif (${NAO})
        # use old c++11 ABI, otherwise GCC cannot link against boost from CTC
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif (${V6})
endif()

set(LINKER_FLAGS_COMMON "-Wl,--no-undefined -Wl,--build-id -rdynamic")
if (${NAO})
    set(LINKER_FLAGS_COMMON "${LINKER_FLAGS_COMMON} -static-libgcc -static-libstdc++")
    if (${BB_ASAN})
        set(LINKER_FLAGS_COMMON "${LINKER_FLAGS_COMMON} -fsanitize=address -static-libasan")
    endif()
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CFLAGS_COMMON} -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CFLAGS_COMMON} -O3")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS_COMMON}")

set(CMAKE_CXX_STANDARD 17)

# set by cmake variable
message(STATUS "build type: " ${CMAKE_BUILD_TYPE})
