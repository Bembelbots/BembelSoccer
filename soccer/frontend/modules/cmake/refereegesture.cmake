set(MODREFEREEGESTURE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/refereegesture)

target_sources(libfrontend
PRIVATE
    ${MODREFEREEGESTURE_DIR}/refereegesture.cpp
)

add_library(modrefereegesture INTERFACE)
target_link_libraries(modrefereegesture INTERFACE ${OpenCV_LIBS} tensorflow-lite)
