# detaches debug info from binary and add .gnu_debuglink property, so GDB will find the debug info
# see http://www.sourceware.org/gdb/onlinedocs/gdb/Separate-Debug-Files.html
macro(detach_dbg_info target)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${target} ${target}.debug
		COMMAND ${CMAKE_OBJCOPY} --strip-debug ${target}
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${target}.debug ${target}
		COMMAND chmod a-x ${target}.debug
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}.debug)
endmacro()
