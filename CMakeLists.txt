cmake_minimum_required(VERSION 3.25)
project(NanoGUI)

set(CMAKE_C_STANDARD 17)

add_library(NanoGUI nanogui.c)
add_executable(NanoGUIExample main.c)
target_link_libraries(NanoGUIExample PRIVATE NanoGUI)
