subdirs(ELFIO)

if (${JENKINS})
	add_custom_target(git_submodules true) # do nothing when executed by jenkins
else()
	add_custom_target(git_submodules git submodule update --init)
endif()

set(libdir "/usr/local/lib")

add_staticlib(absl
    ${libdir}/libabsl.a
)

add_staticlib(clog
    ${libdir}/libclog.a
)

add_staticlib(cpuinfo
    ${libdir}/libcpuinfo.a
)

add_staticlib(farmhash
    ${libdir}/libfarmhash.a
)

add_staticlib(fft2d_fftsg
    ${libdir}/libfft2d_fftsg.a
)

add_staticlib(fft2d_fftsg2d
    ${libdir}/libfft2d_fftsg2d.a
)

add_staticlib(ruy
    ${libdir}/libruy.a
)

add_staticlib(pthreadpool
    ${libdir}/libpthreadpool.a
)

add_staticlib(xnnpack
    ${libdir}/libXNNPACK.a
)

add_staticlib(tensorflow-lite
    ${libdir}/libtensorflow-lite.a
    /usr/local/include
)

target_link_libraries(tensorflow-lite INTERFACE
	ruy farmhash flatbuffers fft2d_fftsg fft2d_fftsg2d xnnpack cpuinfo
	clog ${CMAKE_DL_LIBS} pthread pthreadpool
)


add_library(cabsl INTERFACE)
target_include_directories(cabsl SYSTEM INTERFACE
    cabsl/include
)

add_library(gsl INTERFACE)
target_include_directories(gsl SYSTEM INTERFACE
    GSL/include
)

add_library(entt INTERFACE)
target_include_directories(entt SYSTEM INTERFACE
    entt/src
)


set_sources(HTWK_VISION_SRC
    "HTWKVision/neuralnet/classifier.cpp"
    "HTWKVision/neuralnet/classifier_relu.cpp"
    "HTWKVision/neuralnet/convolutions.cpp"
    "HTWKVision/ball_detector.cpp"
    "HTWKVision/ball_detector_legacy.cpp"
    "HTWKVision/ball_feature_extractor.cpp"
    "HTWKVision/ellifit.cpp"
    "HTWKVision/feet_detector.cpp"
    "HTWKVision/field_color_detector.cpp"
    "HTWKVision/field_detector.cpp"
    "HTWKVision/goal_detector.cpp"
    "HTWKVision/htwk_vision.cpp"
    "HTWKVision/hypotheses_generator.cpp"
    "HTWKVision/hypotheses_generator_blocks.cpp"
    "HTWKVision/hypotheses_generator_blur.cpp"
    "HTWKVision/hypotheses_generator_scanlines.cpp"
    "HTWKVision/integral_image.cpp"
    "HTWKVision/jersey_color_detector.cpp"
    "HTWKVision/line_detector.cpp"
    "HTWKVision/lineedge.cpp"
    "HTWKVision/near_obstacle_detector.cpp"
    "HTWKVision/object_detector.cpp"
    "HTWKVision/ransac_ellipse_fitter.cpp"
    "HTWKVision/region_classifier.cpp"
    "HTWKVision/robot_area_detector.cpp"
    "HTWKVision/robot_detector.cpp"
    "HTWKVision/robot_classifier.cpp"
)

add_library(HTWKVision STATIC EXCLUDE_FROM_ALL ${HTWK_VISION_SRC})
target_link_libraries(HTWKVision
    caffe
    pthread
    pthread_nonshared
    Eigen3::Eigen
)
target_include_directories(HTWKVision SYSTEM PUBLIC
    .
    HTWKVision
    HTWKVision/include
    HTWKVision/neuralnet
    HTWKVision/3rdparty/liblodepng
)

# always build HTWKVision using -Ofast
set(CMAKE_HTWK_FLAGS "-Ofast -funroll-loops -fomit-frame-pointer")
set(HTWK_DISABLE_WARNINGS "-Wno-double-promotion -Wno-strict-overflow -Wno-maybe-uninitialized -Wno-array-bounds -Wno-cpp")
set_target_properties(HTWKVision
    PROPERTIES COMPILE_FLAGS
    "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_HTWK_FLAGS} ${HTWK_DISABLE_WARNINGS}")

add_dependencies(HTWKVision git_submodules)
