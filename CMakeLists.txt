cmake_minimum_required(VERSION 3.16)
project(NanoGUI)

set(CMAKE_C_STANDARD 17)

set(USE_X11_NOT_WAYLAND NO CACHE BOOL "Use X11 instead of Wayland on Linux")

add_library(NanoGUI nanogui.h)
if (WIN32)
    target_sources(NanoGUI PRIVATE win32.c)
elseif (UNIX AND NOT APPLE)
    if (${USE_X11_NOT_WAYLAND})
        target_sources(NanoGUI PRIVATE x11.c)
        find_package(X11 REQUIRED)
        target_link_libraries(NanoGUI PRIVATE ${X11_LIBRARIES})
        target_include_directories(NanoGUI PRIVATE ${X11_INCLUDE_DIR})
    else ()
        target_sources(NanoGUI PRIVATE wayland.c)
        # Finding Wayland based on https://github.com/emersion/hello-wayland/blob/f6a8203309977af03cda94765dd61367c189bea6/Makefile
        find_package(PkgConfig REQUIRED)
        pkg_search_module(WaylandClient REQUIRED wayland-client)
        target_compile_options(NanoGUI PRIVATE ${WaylandCirent_CFLAGS})
        target_link_libraries(NanoGUI PRIVATE ${WaylandCirent_LIBRARIES})

        pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
        pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)
        set(XDG_SHELL_PROTOCOL ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml)

        add_custom_command(
                OUTPUT xdg-shell-client-protocol.h
                COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_PROTOCOL} xdg-shell-client-protocol.h
        )
        add_custom_command(
                OUTPUT xdg-shell-protocol.c
                COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_PROTOCOL} xdg-shell-protocol.c
        )
        target_sources(NanoGUI PRIVATE xdg-shell-protocol.c xdg-shell-client-protocol.h)
    endif ()
else ()
    message(FATAL_ERROR "Unsupported platform")
endif ()
