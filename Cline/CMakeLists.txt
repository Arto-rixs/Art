cmake_minimum_required(VERSION 3.10)
project(TradingClient)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение Qt5
find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

# Включение TGUI и SFML
find_package(SFML 2.5 COMPONENTS system window graphics network audio REQUIRED)
find_package(TGUI 0.9 REQUIRED)

# Добавление исходных файлов
add_executable(TradingClient
    ${CMAKE_SOURCE_DIR}/cline.cpp
)

# Линковка с библиотеками
target_link_libraries(TradingClient
    sfml-system sfml-window sfml-graphics sfml-network sfml-audio
    tgui
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network
)
