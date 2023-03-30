cmake_minimum_required(VERSION 3.25)
project(NanoGUI)

set(CMAKE_CXX_STANDARD 17)

add_library(NanoGUI nanogui.cpp)
add_executable(NanoGUIExample main.cpp)
target_link_libraries(NanoGUIExample PRIVATE NanoGUI)
