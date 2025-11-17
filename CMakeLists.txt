cmake_minimum_required(VERSION 3.16)
project(TorqueCalibrationApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialPort Charts)

# 启用Qt的MOC、UIC和RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 源文件
set(SOURCES
    main.cpp
    MainWindow.cpp
    SerialPortManager.cpp
    DataProcessor.cpp
)

set(HEADERS
    MainWindow.h
    SerialPortManager.h
    DataProcessor.h
)

set(UI_FILES
    MainWindow.ui
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Charts
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
