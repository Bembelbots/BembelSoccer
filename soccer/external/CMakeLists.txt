subdirs(ELFIO)

if (${JENKINS})
	add_custom_target(git_submodules true) # do nothing when executed by jenkins
else()
	add_custom_target(git_submodules git submodule update --init)
endif()

if (${V6})
    set(libdir lib6)
else ()
    set(libdir lib)
    # Only add protobuf on V5, it's pulled from CTC on V6
    add_staticlib(protobuf-lite
        protobuf/lib/libprotobuf-lite.a
    )
    add_staticlib(protobuf
        protobuf/lib/libprotobuf.a
        protobuf/include
    )
    add_staticlib(protoc
        protobuf/lib/libprotoc.a
    )
endif ()


add_staticlib(absl
    tensorflow-lite/${libdir}/libabsl.a
)

add_staticlib(clog
    tensorflow-lite/${libdir}/libclog.a
)

add_staticlib(cpuinfo
    tensorflow-lite/${libdir}/libcpuinfo.a
)

add_staticlib(farmhash
    tensorflow-lite/${libdir}/libfarmhash.a
)

add_staticlib(fft2d_fftsg
    tensorflow-lite/${libdir}/libfft2d_fftsg.a
)

add_staticlib(fft2d_fftsg2d
    tensorflow-lite/${libdir}/libfft2d_fftsg2d.a
)

add_staticlib(ruy
    tensorflow-lite/${libdir}/libruy.a
)

add_staticlib(pthreadpool
    tensorflow-lite/${libdir}/libpthreadpool.a
)

add_staticlib(xnnpack
    tensorflow-lite/${libdir}/libXNNPACK.a
)

add_staticlib(tensorflow-lite
    tensorflow-lite/${libdir}/libtensorflow-lite.a
    tensorflow-lite/include
)

target_link_libraries(tensorflow-lite INTERFACE
	ruy farmhash flatbuffers fft2d_fftsg fft2d_fftsg2d
	xnnpack cpuinfo clog ${CMAKE_DL_LIBS}
	pthread pthreadpool
)

add_staticlib(fftw3f
    fftw3/lib/libfftw3f.a
    fftw3/include
)

add_staticlib(glog
    glog/${libdir}/libglog.a
    glog/include
)

add_staticlib(hdf5
    hdf5/${libdir}/libhdf5.a
    hdf5/include
)

add_staticlib(hdf5_hl
    hdf5/${libdir}/libhdf5_hl.a
)

add_staticlib(gflags
    gflags/${libdir}/libgflags.a
    gflags/include
)

add_staticlib(openblas
    openblas/${libdir}/libopenblas.a
    openblas/include
)



# statically linking caffe needs some extra attention,
# see https://stackoverflow.com/questions/30325108/caffe-layer-creation-failure
add_library(caffe INTERFACE)
target_include_directories(caffe SYSTEM INTERFACE
    caffe/include
)
add_staticlib(caffe_raw
    caffe/${libdir}/libcaffe.a
)
target_link_libraries(caffe INTERFACE
    -Wl,--whole-archive
	caffe_raw
	-Wl,--no-whole-archive
    protobuf
	openblas
	hdf5_hl
	hdf5
	glog
	gflags
)

add_library(cabsl INTERFACE)
target_include_directories(cabsl SYSTEM INTERFACE
    cabsl/include
)

add_library(Eigen INTERFACE)
target_include_directories(Eigen SYSTEM INTERFACE
    eigen
    eigen/Eigen
)

add_library(gsl INTERFACE)
target_include_directories(gsl SYSTEM INTERFACE
    GSL/include
)

add_library(entt INTERFACE)
target_include_directories(entt SYSTEM INTERFACE
    entt/src
)

set(FLATBUFFERS_INSTALL OFF)
set(FLATBUFFERS_BUILD_TESTS OFF)
set(FLATBUFFERS_BUILD_FLATHASH OFF)
add_subdirectory(flatbuffers EXCLUDE_FROM_ALL)
# build fails without disabling double-promotion warning/error
target_compile_options(flatc PUBLIC -Wno-double-promotion)
target_compile_options(flatbuffers PUBLIC -Wno-double-promotion)
target_include_directories(flatbuffers SYSTEM PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/flatbuffers/include>)

set(CELERO_COMPILE_DYNAMIC_LIBRARIES OFF)
add_subdirectory(Celero-2.4.0 EXCLUDE_FROM_ALL)

#Build tinycnn
set(TINY_CNN_SRCS "tiny_cnn/tiny_cnn/io/caffe/caffe.pb.cc")
add_library(tiny_cnn STATIC EXCLUDE_FROM_ALL ${TINY_CNN_SRCS})
target_link_libraries(tiny_cnn protobuf)


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
    Eigen
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

add_dependencies(flatc git_submodules)
add_dependencies(flatbuffers git_submodules)
add_dependencies(HTWKVision git_submodules)
