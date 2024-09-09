set(BBNETWORK_PATH ${BBFRAMEWORK_PATH}/network)

target_sources(bbframework
PRIVATE
    ${BBNETWORK_PATH}/network.cpp
    ${BBNETWORK_PATH}/tcp_client.cpp
    ${BBNETWORK_PATH}/tcp_server.cpp
    ${BBNETWORK_PATH}/tcp_session.cpp
    ${BBNETWORK_PATH}/udp.cpp
)

add_library(bbnetwork INTERFACE)
target_compile_features(bbnetwork INTERFACE cxx_std_17)
