cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# 项目信息
project(HazeTestDLL 
    VERSION 1.0.0
    DESCRIPTION "Haze编程语言测试动态库"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件
set(SOURCES
    dllmain.cpp
    pch.cpp
)

# 头文件
set(HEADERS
    pch.h
    framework.h
)

# 创建动态库
add_library(HazeTestDLL SHARED ${SOURCES} ${HEADERS})

# 链接到HazeCore库
target_link_libraries(HazeTestDLL PRIVATE HazeCore)

# 设置预编译头文件
target_precompile_headers(HazeTestDLL PRIVATE pch.h)

# 设置库属性
set_target_properties(HazeTestDLL PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "HazeTestDLL"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 编译定义
target_compile_definitions(HazeTestDLL PRIVATE
    $<$<PLATFORM_ID:Windows>:_WINDOWS _USRDLL HAZETESTDLL_EXPORTS>
)

# 安装规则
install(TARGETS HazeTestDLL
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
) 