cmake_minimum_required(VERSION 3.16)

# 强制使用g++而不是clang（如果clang有链接问题）
set(CMAKE_CXX_COMPILER g++)

project(TorqueCalibration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6或Qt5
# 用户指定使用Qt 5.12，优先查找Qt5
set(QT_FOUND FALSE)
set(QT_VERSION_MAJOR 5)

# 添加常见的Qt 5.12安装路径
set(QT_POSSIBLE_PATHS
    "/usr/lib/x86_64-linux-gnu/cmake/Qt5"
    "/usr/lib/cmake/Qt5"
    "/opt/Qt/5.12.*/lib/cmake/Qt5"
    "/opt/Qt5.12.*/lib/cmake/Qt5"
    "$ENV{HOME}/Qt/5.12.*/lib/cmake/Qt5"
    "$ENV{HOME}/Qt5.12.*/lib/cmake/Qt5"
    "/usr/local/Qt/5.12.*/lib/cmake/Qt5"
    "/usr/local/Qt5.12.*/lib/cmake/Qt5"
)

# 尝试从环境变量获取Qt路径
if(DEFINED ENV{QT_DIR})
    list(INSERT QT_POSSIBLE_PATHS 0 "$ENV{QT_DIR}")
    set(CMAKE_PREFIX_PATH "$ENV{QT_DIR}" ${CMAKE_PREFIX_PATH})
    message(STATUS "从环境变量QT_DIR获取路径: $ENV{QT_DIR}")
endif()

if(DEFINED ENV{QT5_DIR})
    list(INSERT QT_POSSIBLE_PATHS 0 "$ENV{QT5_DIR}")
    set(CMAKE_PREFIX_PATH "$ENV{QT5_DIR}" ${CMAKE_PREFIX_PATH})
    message(STATUS "从环境变量QT5_DIR获取路径: $ENV{QT5_DIR}")
endif()

# 从CMAKE_PREFIX_PATH获取（如果通过命令行指定）
if(CMAKE_PREFIX_PATH)
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
endif()

# 尝试Qt5（优先，因为用户指定了Qt 5.12）
foreach(QT_PATH ${QT_POSSIBLE_PATHS})
    if(EXISTS ${QT_PATH})
        set(CMAKE_PREFIX_PATH ${QT_PATH} ${CMAKE_PREFIX_PATH})
        message(STATUS "尝试Qt路径: ${QT_PATH}")
    endif()
endforeach()

find_package(Qt5 QUIET COMPONENTS Core Widgets SerialPort Charts)
if(Qt5_FOUND)
    set(QT_FOUND TRUE)
    message(STATUS "找到Qt5: ${Qt5_VERSION}")
else()
    # 如果指定路径找不到，尝试系统默认路径
    message(STATUS "在指定路径未找到Qt5，尝试系统默认路径...")
    find_package(Qt5 QUIET COMPONENTS Core Widgets SerialPort Charts)
    if(Qt5_FOUND)
        set(QT_FOUND TRUE)
        message(STATUS "在系统路径找到Qt5: ${Qt5_VERSION}")
    else()
        # 尝试Qt6作为备选
        find_package(Qt6 QUIET COMPONENTS Core Widgets SerialPort Charts)
        if(Qt6_FOUND)
            set(QT_VERSION_MAJOR 6)
            set(QT_FOUND TRUE)
            message(STATUS "找到Qt6: ${Qt6_VERSION}")
        endif()
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
