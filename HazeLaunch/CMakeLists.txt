cmake_minimum_required(VERSION 3.5.0)
project(HazeLaunch VERSION 0.1.0 LANGUAGES C CXX)

include_directories(G:/Github/Haze/Haze/Haze/Header)
link_libraries(G:/Github/Haze/Haze/x64/Debug/Haze.lib)

add_executable(HazeLaunch HazeLaunch.cpp)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
