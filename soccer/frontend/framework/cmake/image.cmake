set(BBIMAGE_PATH ${BBFRAMEWORK_PATH}/image)

target_sources(bbframework
PRIVATE
    ${BBIMAGE_PATH}/image.cpp
    ${BBIMAGE_PATH}/camimage.cpp
    ${BBIMAGE_PATH}/yuv422.cpp
    ${BBIMAGE_PATH}/rgb.cpp
    ${BBIMAGE_PATH}/bl2.cpp
    ${BBIMAGE_PATH}/svg/body.cpp
    ${BBIMAGE_PATH}/svg/header.cpp
    ${BBIMAGE_PATH}/svg/image.cpp
)

add_library(bbimage INTERFACE)
target_compile_features(bbimage INTERFACE cxx_std_17)
target_link_libraries(bbimage INTERFACE bbrepr)
