cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)

# Включение TGUI и SFML
find_package(SFML 2.5 COMPONENTS system window graphics network audio REQUIRED)
find_package(TGUI 0.9 REQUIRED)

add_executable(MyProject cline.cpp)

# Линковка с библиотеками
target_link_libraries(MyProject sfml-system sfml-window sfml-graphics sfml-network sfml-audio tgui)
