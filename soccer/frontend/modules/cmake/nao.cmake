set(MODNAO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nao)

target_sources(libfrontend
PRIVATE
    ${MODNAO_DIR}/modules/naothread.cpp
    ${MODNAO_DIR}/modules/imagethread.cpp

    ${MODNAO_DIR}/camera/image_buffer.cpp
    ${MODNAO_DIR}/camera/naocameras.cpp
    ${MODNAO_DIR}/camera/cameraconfig.cpp

    ${MODNAO_DIR}/camera/sources/tcpvideosource.cpp

    ${MODNAO_DIR}/say/nao_say_backend.cpp

    ${MODNAO_DIR}/nao.cpp
    
	${MODNAO_DIR}/camera/CameraV4L2.cpp
    ${MODNAO_DIR}/camera/sources/v6videosource.cpp
)

add_library(modnao INTERFACE)
target_link_libraries(modnao INTERFACE libbembelbots atomic ${OpenCV_LIBS})
