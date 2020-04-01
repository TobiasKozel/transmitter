project(networklibrary)
cmake_minimum_required(VERSION 2.8)

if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(netlib_PLATFORM_DEPS
            ws2_32
            iphlpapi)
        add_definitions(-DWIN64=1)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(netlib_PLATFORM_DEPS
            wsock32
            iphlpapi)
        add_definitions(-DWIN32=1)
    endif()

    add_definitions(-DWINDOWS=1)
elseif(UNIX)
    add_definitions(-DUNIX=1)
endif()

set(NETLIB_SRC
    src/netlib.c
    src/netlib.h
    src/netlib_tcp.c
    src/netlib_udp.c
    src/netlib_select.c
    src/platform.h
    src/util.h)

add_library("netlib" SHARED ${NETLIB_SRC})
target_include_directories("netlib" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")

target_link_libraries(netlib
    ${netlib_PLATFORM_DEPS})

# Tests
add_executable("netlib_client" "tests/client.c")
add_dependencies("netlib_client" "netlib")
target_link_libraries("netlib_client" "netlib")
add_executable("netlib_server" "tests/server.c")
add_dependencies("netlib_server" "netlib")
target_link_libraries("netlib_server" "netlib")
