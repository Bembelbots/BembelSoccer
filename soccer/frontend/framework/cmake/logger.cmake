set(BBLOGGER_PATH ${BBFRAMEWORK_PATH}/logger)

target_sources(bbframework
PRIVATE
    ${BBLOGGER_PATH}/backends.cpp
    ${BBLOGGER_PATH}/logstream.cpp
    ${BBLOGGER_PATH}/xlogger.cpp
    #${BBLOGGER_PATH}/nao_say_backend.cpp
)

add_library(bblogger INTERFACE)
target_compile_features(bblogger INTERFACE cxx_std_17)
target_link_libraries(bblogger INTERFACE pthread)