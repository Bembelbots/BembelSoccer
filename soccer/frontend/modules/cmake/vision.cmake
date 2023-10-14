set(MODVISION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vision)

target_sources(libfrontend
PRIVATE
    ${MODVISION_DIR}/vision.cpp
    ${MODVISION_DIR}/toolbox/visiontoolbox.cpp
    ${MODVISION_DIR}/toolbox/colorclasses.cpp
    ${MODVISION_DIR}/detector/ball_detector.cpp
    ${MODVISION_DIR}/detector/crossing_detector.cpp
    ${MODVISION_DIR}/detector/caffeclassifier.cpp
)

add_library(modvision INTERFACE)
target_link_libraries(modvision INTERFACE dl HTWKVision ${OpenCV_LIBS})
