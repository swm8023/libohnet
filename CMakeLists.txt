cmake_minimum_required(VERSION 2.8.9)
project(libfastnet C CXX)


set(ENTEST true)
set(ENEFFTEST false)
set(DEBUG  false)


if (ENTEST)
	add_definitions(-DENTEST)
    if (ENEFFTEST)
        add_definitions(-DENEFFTEST)
    endif()
endif()

if (DEBUG)
    set(CMAKE_BUILD_TYPE DEBUG)
	add_definitions(-DDEBUG)
    add_compile_options(-g -O0)
else()
    add_definitions(-DNDEBUG)
    add_compile_options(-O2)
endif()


if(HAVE_JEMALLOC_H)
    add_definitions(-DHAVEJEMALLOC)
endif()

include_directories(include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


add_subdirectory(src)

if (ENTEST)
	add_subdirectory(tests)
endif()

set(extra_libs
	oh
)

link_directories(${PROJECT_BINARY_DIR}/lib)
add_executable(main main.cpp)
add_dependencies(main oh_so)
target_link_libraries(main ${extra_libs})