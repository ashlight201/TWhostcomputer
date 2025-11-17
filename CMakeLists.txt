cmake_minimum_required(VERSION 3.20)

project(TWhostcomputer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(QT_REQUIRED_COMPONENTS Widgets SerialPort Charts)

set(QT_PACKAGE Qt6)
find_package(Qt6 COMPONENTS ${QT_REQUIRED_COMPONENTS} QUIET)

if(NOT Qt6_FOUND)
    set(QT_PACKAGE Qt5)
    find_package(Qt5 COMPONENTS ${QT_REQUIRED_COMPONENTS} REQUIRED)
endif()

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SerialChannelManager.cpp
    src/DataProcessor.cpp
    forms/MainWindow.ui
)

if(${QT_PACKAGE} STREQUAL Qt6)
    qt_add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${QT_PACKAGE}::Widgets
        ${QT_PACKAGE}::SerialPort
        ${QT_PACKAGE}::Charts
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
