set(BBBLACKBOARD_PATH ${BBFRAMEWORK_PATH}/blackboard)

target_sources(bbframework
PRIVATE
    ${BBBLACKBOARD_PATH}/blackboardregistry.cpp
    ${BBBLACKBOARD_PATH}/blackboard.cpp
    ${BBBLACKBOARD_PATH}/datacontainer.cpp
    ${BBBLACKBOARD_PATH}/introspection.cpp
)

add_library(bbblackboard INTERFACE)
target_compile_features(bbblackboard INTERFACE cxx_std_17)
