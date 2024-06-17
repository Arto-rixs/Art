cmake_minimum_required(VERSION 3.10)
project(Terminal)

set(CMAKE_CXX_STANDARD 20)

add_executable(Terminal main.cpp)

find_package(SFML 2.5 COMPONENTS system window graphics network REQUIRED)
find_package(TGUI REQUIRED)

target_link_libraries(Terminal sfml-system sfml-window sfml-graphics sfml-network tgui)
