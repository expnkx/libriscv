cmake_minimum_required(VERSION 3.1.0)
project(hello_world C CXX)

option(MINIMAL     "Minimal build" OFF)
option(DEBUGGING   "Debugging build" OFF)
option(GCSECTIONS  "Garbage collect empty sections" ON)
option(NATIVE      "Enable native performance" OFF)
option(LTO         "Enable interprocedural optimizations" OFF)

set(RISCV_ABI "-march=rv32imc -mabi=ilp32")
set(WARNINGS  "-Wall -Wextra")
set(COMMON    "-O0 -fstack-protector -fexceptions -frtti")
set(CMAKE_C_FLAGS "${WARNINGS} ${RISCV_ABI} -std=c11 ${COMMON}")
set(CMAKE_CXX_FLAGS "${WARNINGS} ${RISCV_ABI} -std=c++17 ${COMMON}")

if (DEBUGGING)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
elseif (MINIMAL)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
endif()

if (NATIVE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

if (GCSECTIONS)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-gc-sections")
endif()


set(SOURCES
	src/hello_world.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "") # remove -rdynamic

add_executable(hello_world ${SOURCES})
target_link_libraries(hello_world -static -static-libgcc -Wl,--start-group -Wl,-lstdc++ -Wl,-lsupc++ -Wl,-lc -Wl,-lgloss -Wl,--end-group)
# place ELF into the top binary folder
set_target_properties(hello_world
	PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
