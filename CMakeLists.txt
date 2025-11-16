cmake_minimum_required(VERSION 3.16)
project(JointImpedanceControl VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialPort)

qt6_standard_project_setup()

set(SOURCES
    main.cpp
    mainwindow.cpp
    motorcontrol.cpp
)

set(HEADERS
    mainwindow.h
    motorcontrol.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
)
