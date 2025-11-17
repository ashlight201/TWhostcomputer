cmake_minimum_required(VERSION 3.16)

# 强制使用g++而不是clang（如果clang有链接问题）
set(CMAKE_CXX_COMPILER g++)

project(TorqueCalibration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6或Qt5
# 首先尝试Qt6，如果失败则尝试Qt5
set(QT_FOUND FALSE)

# 尝试Qt6
find_package(Qt6 QUIET COMPONENTS Core Widgets SerialPort Charts)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_FOUND TRUE)
    message(STATUS "找到Qt6")
else()
    # 尝试Qt5
    find_package(Qt5 QUIET COMPONENTS Core Widgets SerialPort Charts)
    if(Qt5_FOUND)
        set(QT_VERSION_MAJOR 5)
        set(QT_FOUND TRUE)
        message(STATUS "找到Qt5")
    endif()
endif()

if(NOT QT_FOUND)
    message(WARNING "未找到Qt5或Qt6，尝试使用pkg-config...")
    # 尝试通过pkg-config查找
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(QT5 QUIET Qt5Core Qt5Widgets Qt5SerialPort Qt5Charts)
        if(QT5_FOUND)
            set(QT_VERSION_MAJOR 5)
            set(QT_FOUND TRUE)
            message(STATUS "通过pkg-config找到Qt5")
        else()
            pkg_check_modules(QT6 QUIET Qt6Core Qt6Widgets Qt6SerialPort Qt6Charts)
            if(QT6_FOUND)
                set(QT_VERSION_MAJOR 6)
                set(QT_FOUND TRUE)
                message(STATUS "通过pkg-config找到Qt6")
            endif()
        endif()
    endif()
endif()

if(NOT QT_FOUND)
    message(FATAL_ERROR 
        "未找到Qt5或Qt6开发库！\n"
        "请安装Qt开发库：\n"
        "  Ubuntu/Debian: sudo apt-get install qtbase5-dev libqt5serialport5-dev qtcharts5-dev\n"
        "  或: sudo apt-get install qt6-base-dev qt6-serialport-dev qt6-charts-dev"
    )
endif()

# 确保正确找到Qt包
if(NOT Qt${QT_VERSION_MAJOR}_FOUND)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets SerialPort Charts)
endif()

message(STATUS "使用Qt版本: ${QT_VERSION_MAJOR}")

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
