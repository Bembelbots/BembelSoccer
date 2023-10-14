project(lola-backend)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SRC
	src/main.cpp
	src/lola/connector.cpp
)

add_executable(${PROJECT_NAME} ${SRC})
detach_dbg_info(${PROJECT_NAME})
add_buildinfo(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
	libbembelbots
	stdc++fs
	Eigen3::Eigen
	elfio
	rt
	pthread
	boost_system
	boost_filesystem
	boost_program_options
	PkgConfig::SPEECHD
)
