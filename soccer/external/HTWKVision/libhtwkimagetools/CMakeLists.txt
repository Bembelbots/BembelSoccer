project(htwkimagetools)

find_package(PNG)

set(FILES
    htwkcolorconversion.h
    htwkcolorconversion.cpp

    htwkpngimageprovider.h
    htwkpngimageprovider.cpp

    htwkpngimagesaver.h
    htwkpngimagesaver.cpp

    htwkyuv422image.h
    htwkyuv422image.cpp

    htwkrgbaimage.h
    htwkrgbaimage.cpp

    internal/htwkpngimageproviderlodepng.h
    internal/htwkpngimageproviderlodepng.cpp

    internal/htwkpngimagesaverlodepng.h
    internal/htwkpngimagesaverlodepng.cpp

    ../3rdparty/liblodepng/lodepng.h
    ../3rdparty/liblodepng/lodepng.cpp
)

include_directories(../3rdparty/liblodepng)

if(PNG_FOUND)

    set(FILES
        ${FILES}
        internal/htwkpngimageproviderlibpng.h
        internal/htwkpngimageproviderlibpng.cpp
        internal/htwkpngimagesaverlibpng.h
        internal/htwkpngimagesaverlibpng.cpp
    )
else()
    message("\n###########################################################################")
    message("# PERFORMANCE: libpng was NOT FOUND. Please install libpng12-dev to get better speed. Will use lodepng as fallback!")
    message("###########################################################################\n")

    add_definitions(-DLIBPNG_NOT_AVAILABLE)
endif()

add_library(${PROJECT_NAME} STATIC ${FILES})
target_link_libraries(${PROJECT_NAME} stdc++ png)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${PROJECT_NAME} DESTINATION "lib")
