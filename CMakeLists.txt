cmake_minimum_required(VERSION 3.21)
project(TWhostcomputer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort Charts)

if (QT_VERSION_MAJOR EQUAL 6)
    qt_standard_project_setup()
else()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/SerialEndpointWidget.h
    src/SerialEndpointWidget.cpp
    src/SerialConnection.h
    src/SerialConnection.cpp
    src/DataProcessor.h
    src/DataProcessor.cpp
    src/DataPlotWidget.h
    src/DataPlotWidget.cpp
)

qt_add_executable(TWhostcomputer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(TWhostcomputer
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::SerialPort
        Qt${QT_VERSION_MAJOR}::Charts
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TWhostcomputer)
endif()
