
set(MODBEHAVIOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/behavior)

set(header_output_directory "${CMAKE_CURRENT_BINARY_DIR}")
set(dot_creation_script "${MODBEHAVIOR_DIR}/soccer/bin/createOptionHeaderFile")
set(behavior_dot_header "behavior_dot.h")
set(behavior_options "${MODBEHAVIOR_DIR}/soccer/options.h")

add_custom_command(
    OUTPUT "${header_output_directory}/${behavior_dot_header}"
    COMMAND "${dot_creation_script}" -o "${header_output_directory}" "${behavior_options}" "${behavior_dot_header}"
    #Rebuild behavior graph if build script was changed...
    DEPENDS ${dot_creation_script}
    #or any behavior files were changed.
    IMPLICIT_DEPENDS CXX ${behavior_options}
    #script has to run in behavior folder, so behavior_dot.h is created in
    #the correct folder.
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(behavior_graph DEPENDS "${header_output_directory}/${behavior_dot_header}")

target_sources(libfrontend
PRIVATE
    ${MODBEHAVIOR_DIR}/behavior.cpp
    ${MODBEHAVIOR_DIR}/simple/simplebehavior.cpp
    ${MODBEHAVIOR_DIR}/soccer/behavior.cpp
	${MODBEHAVIOR_DIR}/soccer/behaviorblackboard.cpp
    ${MODBEHAVIOR_DIR}/soccer/behaviorcontrol.cpp
    ${MODBEHAVIOR_DIR}/soccer/graph.cpp
)

add_library(modbehavior INTERFACE)
target_include_directories(modbehavior
INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(modbehavior INTERFACE cabsl)
add_dependencies(modbehavior INTERFACE behavior_graph)

add_dependencies(behavior_graph git_submodules)
