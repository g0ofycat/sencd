cmake_minimum_required(VERSION 3.16)
project(sencd LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sencd-client
	core/client.c
)

target_include_directories(sencd-client PRIVATE

)

add_executable(sencd-server
	core/server.c
)

target_include_directories(sencd-server PRIVATE

)
