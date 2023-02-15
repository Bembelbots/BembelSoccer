
set(MODWHISTLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/whistle)

target_sources(libfrontend
PRIVATE
    ${MODWHISTLE_DIR}/detect/alsarecorder.cpp
    ${MODWHISTLE_DIR}/detect/rectangular_smooth.cpp
    ${MODWHISTLE_DIR}/detect/fourier_transform.cpp
    ${MODWHISTLE_DIR}/detect/whistle_classifier.cpp
    ${MODWHISTLE_DIR}/detect/whistle_detector.cpp
    ${MODWHISTLE_DIR}/detect/sine_generator.cpp
    ${MODWHISTLE_DIR}/whistle.cpp
)

add_library(modwhistle INTERFACE)
target_link_libraries(modwhistle INTERFACE fftw3f asound)

