cmake_minimum_required(VERSION 3.5)

project(BitmapApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_subdirectory(libbitmap)

set(SOURCES
    main.cpp
    FileModel.cpp
)

set(HEADERS
    FileModel.h
)

qt5_add_resources(QT_RESOURCES qml.qrc)

set(CMAKE_AUTOMOC ON)

add_executable(BitmapApp
    ${SOURCES}
    ${HEADERS}
    ${QT_RESOURCES}
)

target_link_libraries(BitmapApp PRIVATE Qt5::Quick BitmapLib)
