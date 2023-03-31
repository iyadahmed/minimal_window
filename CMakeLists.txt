cmake_minimum_required(VERSION 3.16)
project(NanoGUI)

set(CMAKE_C_STANDARD 17)

add_library(NanoGUI nanogui.c)
if (NOT WIN32)
    find_package(X11 REQUIRED)
    target_link_libraries(NanoGUI PRIVATE ${X11_LIBRARIES})
    target_include_directories(NanoGUI PRIVATE ${X11_INCLUDE_DIR})
endif ()
