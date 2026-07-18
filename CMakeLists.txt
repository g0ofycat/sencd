cmake_minimum_required(VERSION 3.16)
project(sencd LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sencd
	core/core.cpp
)

target_include_directories(sencd PRIVATE

)
