set(BBUTIL_PATH ${BBFRAMEWORK_PATH}/util)

target_sources(bbframework
PRIVATE
    ${BBUTIL_PATH}/assert.cpp
    ${BBUTIL_PATH}/clock.cpp
    ${BBUTIL_PATH}/configparser.cpp
    ${BBUTIL_PATH}/getenv.cpp
)

add_library(bbutil INTERFACE)
target_compile_features(bbutil INTERFACE cxx_std_17)
target_link_libraries(bbutil INTERFACE elfio ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} stdc++fs)
