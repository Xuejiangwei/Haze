cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# 项目信息
project(HazeProject 
    VERSION 1.0.0
    DESCRIPTION "Haze编程语言编译器和虚拟机"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 全局编译选项
if(MSVC)
    # 启用并行编译
    add_compile_options(/MP)
    # 设置字符集
    add_compile_definitions(_UNICODE UNICODE)
    # 设置警告级别
    add_compile_options(/W4)
    # 禁用一些特定的警告
    add_compile_options(/wd4251 /wd4275)
else()
    # GCC/Clang选项
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3)
    endif()
endif()

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 全局包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 选项
option(BUILD_SHARED_LIBS "构建共享库" OFF)
option(BUILD_TESTS "构建测试" ON)
option(BUILD_EXAMPLES "构建示例" ON)

# 添加子项目
add_subdirectory(Haze/Haze)
add_subdirectory(HazeLaunch)

# 如果构建测试，添加测试DLL
if(BUILD_TESTS)
    add_subdirectory(Haze/HazeTestDLL)
endif()

# 安装配置
install(DIRECTORY HzCode/
    DESTINATION share/haze/examples
    FILES_MATCHING PATTERN "*.hz"
)

install(DIRECTORY 库/
    DESTINATION share/haze/stdlib
    FILES_MATCHING PATTERN "*.hz"
)

# 生成配置文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/HazeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/HazeConfig.cmake"
    @ONLY
)

# 打印配置信息
message(STATUS "Haze项目配置:")
message(STATUS "  版本: ${PROJECT_VERSION}")
message(STATUS "  构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "  编译器: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "  共享库: ${BUILD_SHARED_LIBS}")
message(STATUS "  构建测试: ${BUILD_TESTS}")
message(STATUS "  构建示例: ${BUILD_EXAMPLES}") 