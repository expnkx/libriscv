
set(LIBC_SOURCES
	# .c files
	libc.c
	heap.c
	malloc.c
	print.c
	# .cpp files
	start.cpp
	assert.cpp
	libcxx.cpp
	write.cpp
  )

add_library(libc STATIC ${LIBC_SOURCES})
target_include_directories(libc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libc tinyprintf)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set_source_files_properties(
			kernel/panic.cpp
		PROPERTIES COMPILE_FLAGS -Wno-frame-address)
endif()
