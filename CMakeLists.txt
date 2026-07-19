cmake_minimum_required(VERSION 3.16)
project(sencd LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sencd-client
	core/client.c
	src/debug/logs.c
	src/client/core.c
)

target_include_directories(sencd-client PRIVATE
	src/client
	src/shared
	src/debug
)

add_executable(sencd-server
	core/server.c
	src/debug/logs.c
)

target_include_directories(sencd-server PRIVATE
	src/server
	src/shared
	src/debug
)
