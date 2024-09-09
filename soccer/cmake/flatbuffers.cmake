macro(__fbs_option_default OPTION DEFAULT)
    if(NOT DEFINED ${OPTION})
        set(${OPTION} ${DEFAULT})
    endif()
endmacro()

macro(__flatbuffers_command DESTINATION)
    cmake_parse_arguments(
        FLATBUFFERS_OPTIONS
        ""
        "CPP;PYTHON;REFLECTION;OBJECT_API;MUTABLE;NAME_STRINGS;CPP_CHANGE_CASE"
        ""
        ${ARGN}
    )

    if(NOT DEFINED ${FLATBUFFERS_EXE})
        set(FLATBUFFERS_EXE "flatc")
    else()
        message("flatbuffers executable " ${FLATBUFFERS_EXE})
    endif()

    set(FLATBUFFERS_COMMAND ${FLATBUFFERS_EXE})

    __fbs_option_default(FLATBUFFERS_OPTIONS_CPP                TRUE)
    __fbs_option_default(FLATBUFFERS_OPTIONS_PYTHON             TRUE)
    __fbs_option_default(FLATBUFFERS_OPTIONS_REFLECTION         TRUE)
    __fbs_option_default(FLATBUFFERS_OPTIONS_OBJECT_API         TRUE)
    __fbs_option_default(FLATBUFFERS_OPTIONS_MUTABLE            TRUE)
    __fbs_option_default(FLATBUFFERS_OPTIONS_NAME_STRINGS       TRUE)
    __fbs_option_default(FLATBUFFERS_OPTIONS_CPP_CHANGE_CASE    TRUE)
    __fbs_option_default(FLATBUFFERS_OPTIONS_CPP_PTR_TYPE       std::unique_ptr)

    if(${FLATBUFFERS_OPTIONS_CPP})
        list(APPEND FLATBUFFERS_COMMAND -c --cpp-std c++17 --scoped-enums)
    endif()

    if(${FLATBUFFERS_OPTIONS_CPP_CHANGE_CASE})
        list(APPEND FLATBUFFERS_COMMAND --cpp-field-case-style lower)
    endif()
    
    if(${FLATBUFFERS_OPTIONS_PYTHON})
        list(APPEND FLATBUFFERS_COMMAND -p)
    endif()
    
    if(${FLATBUFFERS_OPTIONS_REFLECTION})
        list(APPEND FLATBUFFERS_COMMAND --reflect-names --reflect-types --cpp-static-reflection)
    endif()
    
    if(${FLATBUFFERS_OPTIONS_OBJECT_API})
        list(APPEND FLATBUFFERS_COMMAND --gen-object-api --gen-compare)
    endif()

    if(${FLATBUFFERS_OPTIONS_MUTABLE})
        list(APPEND FLATBUFFERS_COMMAND --gen-mutable)
    endif()

    if(${FLATBUFFERS_OPTIONS_NAME_STRINGS})
        list(APPEND FLATBUFFERS_COMMAND --gen-name-strings)
    endif()

    if(DEFINED ${FLATBUFFERS_OPTIONS_CPP_PTR_TYPE})
        list(APPEND FLATBUFFERS_COMMAND --cpp-ptr-type ${FLATBUFFERS_OPTIONS_CPP_PTR_TYPE})
    endif()


    list(APPEND -I ${CMAKE_CURRENT_SOURCE_DIR})
    list(APPEND -o ${DESTINATION})
endmacro()

function(add_flatbuffers_library TARGET)
    set(FLATBUFFERS_OPTIONS "")

    cmake_parse_arguments(
        FLATBUFFERS
        ""
        "DESTINATION"
        "SOURCES;OPTIONS"
        ${ARGN}
    )
    
    if(NOT DEFINED FLATBUFFERS_SOURCES AND DEFINED FLATBUFFERS_UNPARSED_ARGUMENTS)
        set(FLATBUFFERS_SOURCES ${FLATBUFFERS_UNPARSED_ARGUMENTS})
    elseif(NOT DEFINED FLATBUFFERS_SOURCES)
        set(FLATBUFFERS_SOURCES "")
    endif()

    __fbs_option_default(FLATBUFFERS_DESTINATION "${CMAKE_BINARY_DIR}/include")

    __flatbuffers_command(${FLATBUFFERS_DESTINATION} ${FLATBUFFERS_OPTIONS})
    
    file(MAKE_DIRECTORY ${FLATBUFFERS_DESTINATION})

    set(FLATBUFFERS_ABSOLUTE_SOURCES "")
    set(FLATBUFFERS_GENERATED_H "")
    foreach(FILE ${FLATBUFFERS_SOURCES})
        get_filename_component(BASE_NAME "${FILE}" NAME_WLE)
        get_filename_component(ABS_FILE "${FILE}" REALPATH BASE_DIR)
        list(APPEND FLATBUFFERS_ABSOLUTE_SOURCES "${ABS_FILE}")
        list(APPEND FLATBUFFERS_GENERATED_H "${FLATBUFFERS_DESTINATION}/${BASE_NAME}_generated.h")
    endforeach()

    add_custom_command(
        OUTPUT ${FLATBUFFERS_GENERATED_H}
        COMMAND ${FLATBUFFERS_COMMAND} ${FLATBUFFERS_ABSOLUTE_SOURCES}
        DEPENDS ${FLATBUFFERS_ABSOLUTE_SOURCES}
        WORKING_DIRECTORY ${FLATBUFFERS_DESTINATION}
        VERBATIM
    )

    add_library(${TARGET} ${FLATBUFFERS_GENERATED_H})
    set_target_properties(${TARGET}
        PROPERTIES
            LINKER_LANGUAGE CXX
    )

    target_include_directories(${TARGET} PUBLIC ${FLATBUFFERS_DESTINATION})
endfunction()
