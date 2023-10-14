set(BBNETWORK_PATH ${BBFRAMEWORK_PATH}/network)

target_sources(bbframework
PRIVATE
    ${BBNETWORK_PATH}/network.cpp
    ${BBNETWORK_PATH}/tcp-client.cpp
    ${BBNETWORK_PATH}/tcp-server.cpp
    ${BBNETWORK_PATH}/udp.cpp
)

add_library(bbnetwork INTERFACE)
target_compile_features(bbnetwork INTERFACE cxx_std_17)