# Add build options here.
# Build options can be enabled with ccmake. All options are disabled by default.
# Every build option adds a macro with the same name, thats 0 when the option is disabled and 1 if its enabled.
# E.g. you can check with
#   #if BB_BENCHMARKING
# if benchmarking is enabled in your code.
# Prefix build options with BB_ so they all appear grouped togehter in ccmake.

add_build_option(V6 "Target Nao V6.")
add_build_option(BB_ASAN "Enable address sanitizer.")
add_build_option(BB_TIME_BUILD "Measure build time. (run with make -j1)")
add_build_option(BB_BACKEND_DIAGNOSTIC "Enable debug mode for naoqi backend.")
add_build_option(BB_BENCHMARKING "Meassure time. See shared/common/benchmark/benchmarking.h.")
add_build_option(BB_VISION_PATCHES "Write store balldetector patches.")

# Options must be added before this function call.
finalize_build_options()
