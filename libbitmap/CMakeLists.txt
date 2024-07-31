cmake_minimum_required(VERSION 3.5)

project(BitmapLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_library(BitmapLib STATIC
    BitmapManager.cpp
    BitmapManager.h
)

target_include_directories(BitmapLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
)

target_link_libraries(BitmapLib PUBLIC
    Qt5::Core
)
