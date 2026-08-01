cmake_minimum_required(VERSION 3.16)
project(sencd LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SODIUM REQUIRED libsodium)
pkg_check_modules(CJSON REQUIRED libcjson)
pkg_check_modules(ZSTD REQUIRED libzstd)

if (WIN32)
	set(PLATFORM_SOURCES src/platform/windows/tun.c)
	set(PLATFORM_INCLUDE_DIR src/platform/windows)
else()
	set(PLATFORM_SOURCES src/platform/linux/tun.c)
	set(PLATFORM_INCLUDE_DIR src/platform/linux)
endif()

add_executable(sencd-client
	main/client.c
	src/debug/logs.c
	src/server/serverinit/idle.c
	src/commands/client_env.c
	src/commands/utility/clear.c
	src/client/client_core.c
	src/shared/packet.c
	src/shared/security/crypto.c
	src/sessions/session.c
	src/sessions/session_manager.c
	src/tunnel/udp_tunnel.c
	src/tunnel/tunnel_packet.c
	${PLATFORM_SOURCES}
)

target_include_directories(sencd-client PRIVATE
	src/debug
	src/server/serverinit
	src/commands
	src/commands/utility
	src/client
	src/shared
	src/shared/security
	src/sessions
	src/tunnel
	${PLATFORM_INCLUDE_DIR}
	${SODIUM_INCLUDE_DIRS}
	${CJSON_INCLUDE_DIRS}
	${ZSTD_INCLUDE_DIRS}
)

target_link_directories(sencd-client PRIVATE ${SODIUM_LIBRARY_DIRS} ${CJSON_LIBRARY_DIRS} ${ZSTD_LIBRARY_DIRS})
target_link_libraries(sencd-client PRIVATE ${SODIUM_LIBRARIES} ${CJSON_LIBRARIES} ${ZSTD_LIBRARIES})

add_executable(sencd-server
	main/server.c
	src/debug/logs.c
	src/server/server_core.c
	src/server/serverinit/idle.c
	src/commands/server_env.c
	src/commands/utility/clear.c
	src/shared/packet.c
	src/shared/security/crypto.c
	src/sessions/session.c
	src/sessions/session_manager.c
	src/tunnel/udp_tunnel.c
	src/tunnel/tunnel_packet.c
	${PLATFORM_SOURCES}
)

target_include_directories(sencd-server PRIVATE
	src/debug
	src/server
	src/server/serverinit
	src/commands
	src/commands/utility
	src/shared
	src/shared/security
	src/sessions
	src/tunnel
	${PLATFORM_INCLUDE_DIR}
	${SODIUM_INCLUDE_DIRS}
	${CJSON_INCLUDE_DIRS}
	${ZSTD_INCLUDE_DIRS}
)

target_link_directories(sencd-server PRIVATE ${SODIUM_LIBRARY_DIRS} ${CJSON_LIBRARY_DIRS} ${ZSTD_LIBRARY_DIRS})
target_link_libraries(sencd-server PRIVATE ${SODIUM_LIBRARIES} ${CJSON_LIBRARIES} ${ZSTD_LIBRARIES})
