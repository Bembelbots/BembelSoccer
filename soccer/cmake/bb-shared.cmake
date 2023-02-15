set_property(GLOBAL PROPERTY build_opts_all "")
set_property(GLOBAL PROPERTY build_opts_on "")
set_property(GLOBAL PROPERTY build_opts_off "")

set(build_options_finished FALSE)

function(add_build_option option_name desc)
    if(${build_options_finished})
        message(FATAL_ERROR "Trying to add a build option 'finalize_build_options' was called.")
    endif()
    set(${option_name} OFF CACHE BOOL ${desc})
    append_opt(build_opts_all ${option_name})
    if(${option_name})
        add_definitions(-D${option_name}=1)
        append_opt(build_opts_on ${option_name})
    else()
        add_definitions(-D${option_name}=0)
        append_opt(build_opts_off ${option_name})
    endif()
endfunction()

function(append_opt opt_list_property new_opt)
    get_property(opt_list GLOBAL PROPERTY ${opt_list_property})
    if(opt_list STREQUAL "")
        set_property(GLOBAL PROPERTY ${opt_list_property} "${new_opt}")
    else()
        set_property(GLOBAL PROPERTY ${opt_list_property} "${opt_list} ${new_opt}")
    endif()
endfunction()

function(finalize_build_options)
    if(${build_options_finished})
        message(FATAL_ERROR "Calling 'finalize_build_options' twice.")
    endif()
    get_property(all_build_options GLOBAL PROPERTY build_opts_all)
    get_property(enabled_build_options GLOBAL PROPERTY build_opts_on)
    get_property(disabled_build_options GLOBAL PROPERTY build_opts_off)
    message("Enabled options: ${enabled_build_options}")
    add_definitions(
        -DBB_BUILD_OPTS_ALL="${all_build_options}"
        -DBB_BUILD_OPTS_ON="${enabled_build_options}"
        -DBB_BUILD_OPTS_OFF="${disabled_build_options}"
    )
    set(build_options_finished TRUE PARENT_SCOPE)
endfunction()

# shared libs
set(SHARED_DIR "${CMAKE_CURRENT_LIST_DIR}/../shared")

# function to add a static library
function(add_staticlib libname libpath)
    get_filename_component(libpath ${libpath} REALPATH)
	if(NOT EXISTS ${libpath})
		message(FATAL_ERROR "Static library ${libname} not found. ${libpath} does not exist.")
	endif()
    add_library(${libname} STATIC IMPORTED GLOBAL)
    set_target_properties(${libname} PROPERTIES
        IMPORTED_LOCATION ${libpath})
    if (${ARGC} GREATER 2)
        target_include_directories(${libname} SYSTEM INTERFACE ${ARGN})
    endif ()
endfunction()

# function to add a static library from framework/shared
# Deprecated since the framework was unified to a single project
function(add_sharedlib libname)
	set(f "${SHARED_DIR}/lib/lib${libname}.a")
	# check if shared libs have been build
	if(NOT EXISTS ${f})
		message(FATAL_ERROR "Shared lib ${libname} not found. Build ../shared before building ${PROJECT_NAME}!")
	endif()
	message(STATUS "using lib '${libname}' from shared dir")
	add_staticlib(${libname} ${f})
endfunction()

function(set_sources tgt_var)
    foreach(file IN LISTS ARGN)
        set(${tgt_var} ${${tgt_var}} ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    endforeach()
    set(${tgt_var} ${${tgt_var}} PARENT_SCOPE)
endfunction()
