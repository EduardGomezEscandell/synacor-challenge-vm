cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Wpedantic -Wconversion")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(synacor_challenge LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(ENABLE_SANITIZER)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

message("Compiling with: ${CMAKE_CXX_FLAGS}")

add_subdirectory(external_libraries)
add_subdirectory(testutils)
add_subdirectory(arch)
add_subdirectory(vm)
add_subdirectory(assembler)