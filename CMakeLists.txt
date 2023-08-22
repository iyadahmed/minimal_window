cmake_minimum_required(VERSION 3.16)
project(minimal_window)

set(FORCE_X11_ON_LINUX NO CACHE BOOL "Use X11 instead of Wayland on Linux")

add_library(minimal_window)
target_compile_features(minimal_window PRIVATE c_std_99)
set_target_properties(minimal_window PROPERTIES C_EXTENSIONS OFF)
target_include_directories(minimal_window PUBLIC include)

function(setup_x11)
    target_sources(minimal_window PRIVATE x11.c)
    find_package(X11 REQUIRED)
    target_link_libraries(minimal_window PRIVATE ${X11_LIBRARIES})
    target_include_directories(minimal_window PRIVATE ${X11_INCLUDE_DIR})
endfunction()

if(WIN32)
    target_sources(minimal_window PRIVATE win32.c)

elseif(UNIX AND NOT APPLE)
    if(FORCE_X11_ON_LINUX)
        setup_x11()

    else()
        target_sources(minimal_window PRIVATE wayland.c)

        # Needed for shm_open/close
        target_link_libraries(minimal_window PRIVATE librt.so)

        # Finding Wayland based on https://github.com/emersion/hello-wayland/blob/f6a8203309977af03cda94765dd61367c189bea6/Makefile
        find_package(PkgConfig REQUIRED)
        pkg_search_module(WaylandClient REQUIRED wayland-client)
        target_compile_options(minimal_window PRIVATE ${WaylandClient_CFLAGS})
        target_link_libraries(minimal_window PRIVATE ${WaylandClient_LIBRARIES})

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
        target_sources(minimal_window PRIVATE xdg-shell-protocol.c ${GENERATED_HEADERS_DIR}/xdg-shell-client-protocol.h)
        target_include_directories(minimal_window PRIVATE ${GENERATED_HEADERS_DIR})
    endif()

elseif(UNIX AND APPLE)
    setup_x11()

else()
    message(FATAL_ERROR "Unsupported platform")
endif()
