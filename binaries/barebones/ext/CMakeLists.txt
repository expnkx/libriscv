# Tiny-printf library
add_library(tinyprintf STATIC
		tinyprintf/tinyprintf.c
	)
target_include_directories(tinyprintf PUBLIC
		tinyprintf
	)
target_compile_definitions(tinyprintf PUBLIC TINYPRINTF_OVERRIDE_LIBC=0)
