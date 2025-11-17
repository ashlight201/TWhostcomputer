cmake_minimum_required(VERSION 3.16)
project(TorqueCalibration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6或Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets SerialPort Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets SerialPort Charts)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 源文件
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SerialPortManager.cpp
    src/DataProcessor.cpp
    src/ChartWidget.cpp
)

# 头文件
set(HEADERS
    src/MainWindow.h
    src/SerialPortManager.h
    src/DataProcessor.h
    src/ChartWidget.h
)

# UI文件
set(UI_FILES
    ui/MainWindow.ui
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Charts
)

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
