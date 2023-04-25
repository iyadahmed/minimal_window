cmake_minimum_required(VERSION 3.16)
project(NanoGUI)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(USE_X11_NOT_WAYLAND NO CACHE BOOL "Use X11 instead of Wayland on Linux")

add_library(NanoGUI)
target_include_directories(NanoGUI PUBLIC include)

if(WIN32)
    target_sources(NanoGUI PRIVATE win32.c)
elseif(UNIX AND NOT APPLE)
    if(USE_X11_NOT_WAYLAND)
        target_sources(NanoGUI PRIVATE x11.c)
        find_package(X11 REQUIRED)
        target_link_libraries(NanoGUI PRIVATE ${X11_LIBRARIES})
        target_include_directories(NanoGUI PRIVATE ${X11_INCLUDE_DIR})
    else()
        target_sources(NanoGUI PRIVATE wayland.c)

        # Finding Wayland based on https://github.com/emersion/hello-wayland/blob/f6a8203309977af03cda94765dd61367c189bea6/Makefile
        find_package(PkgConfig REQUIRED)
        pkg_search_module(WaylandClient REQUIRED wayland-client)
        target_compile_options(NanoGUI PRIVATE ${WaylandClient_CFLAGS})
        target_link_libraries(NanoGUI PRIVATE ${WaylandClient_LIBRARIES})

        pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
        pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)
        set(XDG_SHELL_PROTOCOL ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml)

        set(GENERATED_HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_headers)
        add_custom_command(
            OUTPUT ${GENERATED_HEADERS_DIR}/xdg-shell-client-protocol.h
            COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_HEADERS_DIR}
            COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_PROTOCOL} ${GENERATED_HEADERS_DIR}/xdg-shell-client-protocol.h
        )
        add_custom_command(
            OUTPUT xdg-shell-protocol.c
            COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_PROTOCOL} xdg-shell-protocol.c
        )
        target_sources(NanoGUI PRIVATE xdg-shell-protocol.c ${GENERATED_HEADERS_DIR}/xdg-shell-client-protocol.h)
        target_include_directories(NanoGUI PRIVATE ${GENERATED_HEADERS_DIR})
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
