

include(CheckIncludeFile)
include(CheckLibraryExists)

check_include_file(check.h HAVE_CHECK_H)


if(HAVE_CHECK_H)
	include_directories(${PROJECT_SOURCE_DIR}/include)
	file(GLOB TEST_FILE     ohstl/*.c ohev/*.c ohutil/*.c main.c)
    file(GLOB TEST_FILE_CPP ohstl/*.cpp)
	set(TEST_LIB oh check pthread rt m)
	link_directories(${PROJECT_BINARY_DIR}/lib /usr/local/lib)
	add_executable(test ${TEST_FILE} ${TEST_FILE_CPP})
	target_link_libraries(test ${TEST_LIB})
	if (ENGPROF)
	    target_link_libraries(test -pg)
	endif()

	add_executable(testtcpsrv1 testtcpsrv1.c)
	target_link_libraries(testtcpsrv1 ${TEST_LIB})

	add_executable(perftestserver perftestserver.c)
	target_link_libraries(perftestserver ${TEST_LIB})

	add_executable(perftestclient perftestclient.c)
	target_link_libraries(perftestclient ${TEST_LIB})

	add_executable(easyproxy ohapp/easyproxy.c)
	target_link_libraries(easyproxy ${TEST_LIB})

	add_executable(holeproxyclient ohapp/holeproxyclient.c)
	target_link_libraries(holeproxyclient ${TEST_LIB})

	add_executable(holeproxyserver ohapp/holeproxyserver.c)
	target_link_libraries(holeproxyserver ${TEST_LIB})

	add_executable(easyhttp ohapp/easyhttp.c)
	target_link_libraries(easyhttp ${TEST_LIB})


	# add_executable(testlibcstl testlibcstl.c)
	# target_link_libraries(testlibcstl cstl fastnet)

else()
	message(FATAL_ERROR "libaray check not found!")
endif()