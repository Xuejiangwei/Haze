cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# 项目信息
project(HazeLaunch 
    VERSION 1.0.0
    DESCRIPTION "Haze编程语言启动器"
    LANGUAGES CXX
)

# 设置C++标准（与主项目保持一致）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 创建启动器可执行文件
add_executable(HazeLaunch HazeLaunch.cpp)

# 设置包含目录
target_include_directories(HazeLaunch PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../Haze/Haze/Header
    ${CMAKE_CURRENT_SOURCE_DIR}/../Haze/Haze/Define/Header
    ${CMAKE_CURRENT_SOURCE_DIR}/../Haze/Haze/VM/Header
    ${CMAKE_CURRENT_SOURCE_DIR}/../Haze/Haze/HazeLog/Header
)

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
