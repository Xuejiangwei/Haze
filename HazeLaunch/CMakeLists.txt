cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# 项目信息
project(HazeLaunch 
    VERSION 1.0.0
    DESCRIPTION "Haze编程语言启动器"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 创建启动器可执行文件
add_executable(HazeLaunch HazeLaunch.cpp)

# 链接到HazeCore库
target_link_libraries(HazeLaunch PRIVATE HazeCore)

# 设置可执行文件属性
set_target_properties(HazeLaunch PROPERTIES
    OUTPUT_NAME "haze-launch"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 平台特定的链接库
if(WIN32)
    # Windows平台需要的系统库
    target_link_libraries(HazeLaunch PRIVATE ws2_32)
endif()

# 安装规则
install(TARGETS HazeLaunch
    RUNTIME DESTINATION bin
)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
