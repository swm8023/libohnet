

include(CheckIncludeFile)
include(CheckLibraryExists)

check_include_file(check.h HAVE_CHECK_H)

if(HAVE_CHECK_H)
	include_directories(${PROJECT_SOURCE_DIR}/include)
	file(GLOB TEST_FILE *.c)
    file(GLOB TEST_FILE_CPP *.cpp)
	set(TEST_LIB fastnet check pthread rt m)
	link_directories(${PROJECT_BINARY_DIR}/lib)
	add_executable(test ${TEST_FILE} ${TEST_FILE_CPP})
	target_link_libraries(test ${TEST_LIB})
	if (ENGPROF)
	    target_link_libraries(test -pg)
	endif()
	enable_testing()
	add_test(test ${EXECUTABLE_OUTPUT_PATH}/test)
else()
	message(FATAL_ERROR "libaray check not found!")
endif()