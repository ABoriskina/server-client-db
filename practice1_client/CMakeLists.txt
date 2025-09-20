cmake_minimum_required(VERSION 3.16)
project(practice1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(practice1 
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    resources/mainwindow.qrc
    serverHandler.cpp
    serverHandler.hpp
)

target_link_libraries(practice1 PRIVATE Qt6::Widgets)