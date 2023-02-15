# add information about user, git branch & build time to ELF section
macro(add_buildinfo target)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND env OBJCOPY="${CMAKE_OBJCOPY}" ${CMAKE_SOURCE_DIR}/cmake/buildinfo.sh $<TARGET_FILE:${target}>
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()
