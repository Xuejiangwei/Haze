cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# 项目信息
project(Haze 
    VERSION 1.0.0
    DESCRIPTION "Haze编程语言编译器和虚拟机核心"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义源文件组
set(AST_SOURCES
    AST/Source/ASTBase.cpp
    AST/Source/ASTClass.cpp
    AST/Source/ASTEnum.cpp
    AST/Source/ASTFunction.cpp
    AST/Source/ASTLibrary.cpp
    AST/Source/ASTTemplateBase.cpp
    AST/Source/ASTTemplateClass.cpp
)

set(BACKEND_SOURCES
    Backend/Source/BackendParse.cpp
    Backend/Source/ModuleUnit.cpp
    Backend/Source/Optimizer.cpp
)

set(COMPILER_SOURCES
    Compiler/Source/Compiler.cpp
    Compiler/Source/CompilerArrayValue.cpp
    Compiler/Source/CompilerBlock.cpp
    Compiler/Source/CompilerClass.cpp
    Compiler/Source/CompilerClassValue.cpp
    Compiler/Source/CompilerClosureFunction.cpp
    Compiler/Source/CompilerClosureValue.cpp
    Compiler/Source/CompilerElementValue.cpp
    Compiler/Source/CompilerEnum.cpp
    Compiler/Source/CompilerEnumValue.cpp
    Compiler/Source/CompilerFunction.cpp
    Compiler/Source/CompilerHashValue.cpp
    Compiler/Source/CompilerHelper.cpp
    Compiler/Source/CompilerModule.cpp
    Compiler/Source/CompilerObjectBaseValue.cpp
    Compiler/Source/CompilerPointerFunction.cpp
    Compiler/Source/CompilerRefValue.cpp
    Compiler/Source/CompilerStringValue.cpp
    Compiler/Source/CompilerValue.cpp
    Compiler/Source/HazeCompilerPointerValue.cpp
    Compiler/Source/HazeCompilerTemplateClass.cpp
    Compiler/Source/HazeCompilerTemplateFunction.cpp
)

set(DEBUGGER_SOURCES
    Debugger/Source/HazeDebugger.cpp
    Debugger/Source/HazeDebuggerServer.cpp
)

set(DEFINE_SOURCES
    Define/Source/HazeCompare.cpp
    Define/Source/HazeExecuteFile.cpp
    Define/Source/HazeInstruction.cpp
    Define/Source/HazeUtility.cpp
    Define/Source/HazeValue.cpp
)

set(HAZELOG_SOURCES
    HazeLog/Source/HazeLog.cpp
)

set(LIBRARY_SOURCES
    Library/Source/HazeFileLib.cpp
    Library/Source/HazeMemoryLib.cpp
    Library/Source/HazeStandardLibraryBase.cpp
    Library/Source/HazeStream.cpp
)

set(MEMORY_SOURCES
    Memory/Source/GCObjectList.cpp
    Memory/Source/HazeMemory.cpp
    Memory/Source/MemoryBlock.cpp
    Memory/Source/MemoryFreeList.cpp
    Memory/Source/MemoryHelper.cpp
)

set(METADATA_SOURCES
    MetaData/Source/HazeVariable.cpp
)

set(VM_SOURCES
    VM/Source/HazeLibraryManager.cpp
    VM/Source/HazeStack.cpp
    VM/Source/HazeVM.cpp
)

set(OBJECT_SOURCES
    Object/Source/ObjectArray.cpp
    Object/Source/ObjectBase.cpp
    Object/Source/ObjectClass.cpp
    Object/Source/ObjectClosure.cpp
    Object/Source/ObjectDynamicClass.cpp
    Object/Source/ObjectHash.cpp
    Object/Source/ObjectString.cpp
)

set(MAIN_SOURCES
    Source/Haze.cpp
    Source/HazeFilePathHelper.cpp
    Source/HazePch.cpp
    Source/Parse.cpp
)

set(XJSON_SOURCES
    XJson/XJson.cpp
)

# 组合所有源文件
set(ALL_SOURCES
    ${AST_SOURCES}
    ${BACKEND_SOURCES}
    ${COMPILER_SOURCES}
    ${DEBUGGER_SOURCES}
    ${DEFINE_SOURCES}
    ${HAZELOG_SOURCES}
    ${LIBRARY_SOURCES}
    ${MEMORY_SOURCES}
    ${METADATA_SOURCES}
    ${VM_SOURCES}
    ${OBJECT_SOURCES}
    ${MAIN_SOURCES}
    ${XJSON_SOURCES}
)

# 定义包含目录
set(INCLUDE_DIRS
    Header
    AST/Header
    Backend/Header
    Compiler/Header
    Debugger/Header
    Define/Header
    HazeLog/Header
    Library/Header
    Memory/Header
    MetaData/Header
    Object/Header
    VM/Header
)

# 创建库目标
add_library(HazeCore STATIC ${ALL_SOURCES})

# 设置包含目录
target_include_directories(HazeCore 
    PUBLIC 
        ${INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置编译定义
target_compile_definitions(HazeCore 
    PUBLIC 
        HAZE
    PRIVATE
        $<$<PLATFORM_ID:Windows>:_UNICODE UNICODE>
)

# 设置库属性
set_target_properties(HazeCore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "haze"
)

# 创建可执行文件目标
add_executable(HazeCompiler ${MAIN_SOURCES})

# 链接核心库
target_link_libraries(HazeCompiler PRIVATE HazeCore)

# 设置可执行文件属性
set_target_properties(HazeCompiler PROPERTIES
    OUTPUT_NAME "haze"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 平台特定的链接库
if(WIN32)
    target_link_libraries(HazeCore PRIVATE ws2_32)
endif()

# 安装规则
install(TARGETS HazeCore HazeCompiler
    EXPORT HazeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装头文件
install(DIRECTORY Header/
    DESTINATION include/haze
    FILES_MATCHING PATTERN "*.h"
)

foreach(DIR ${INCLUDE_DIRS})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIR})
        install(DIRECTORY ${DIR}/
            DESTINATION include/haze
            FILES_MATCHING PATTERN "*.h"
        )
    endif()
endforeach()

# 导出目标
install(EXPORT HazeTargets
    FILE HazeTargets.cmake
    NAMESPACE Haze::
    DESTINATION lib/cmake/Haze
)
