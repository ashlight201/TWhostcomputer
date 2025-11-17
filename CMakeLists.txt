cmake_minimum_required(VERSION 3.16)
project(MotorCalibration VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt库
find_package(Qt6 COMPONENTS Core Widgets SerialPort Charts QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Widgets SerialPort Charts REQUIRED)
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/serialporthandler.cpp
    src/dataprocessor.cpp
)

# 头文件
set(HEADERS
    src/mainwindow.h
    src/serialporthandler.h
    src/dataprocessor.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 链接Qt库
if(Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Widgets
        Qt6::SerialPort
        Qt6::Charts
    )
else()
    target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::SerialPort
        Qt5::Charts
    )
endif()

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
