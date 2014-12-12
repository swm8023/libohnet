cmake_minimum_required(VERSION 2.8.9)
project(libfastnet C)

set(ENTEST true)
set(DEBUG true)

if (ENTEST)
	add_definitions(-DENTEST)
endif()
if (DEBUG)
	add_definitions(-DDEBUG)
endif()

include_directories(include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


add_subdirectory(src)

if (ENTEST)
	add_subdirectory(tests)
endif()

set(extra_libs
	libfastnet.a
)


link_directories(${PROJECT_BINARY_DIR}/lib)
add_executable(main main.c)
add_dependencies(main fastnet fastnet_so)
target_link_libraries(main ${extra_libs})