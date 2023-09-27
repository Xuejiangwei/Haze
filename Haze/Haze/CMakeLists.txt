cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Haze LANGUAGES CXX)

include_directories(Header
					AST/Header
					Backend/Header
					Compiler/Header
					Debugger/Header
					Define/Header
					HazeLog/Header
					Library/Header
					Memory/Header
					Library/Header
					MetaData/Header
					VM/Header
					)

aux_source_directory(AST/Source SRC_AST)
aux_source_directory(Backend/Source SRC_Backend)
aux_source_directory(Compiler/Source SRC_Compiler)
aux_source_directory(Debugger/Source SRC_Debugger)
aux_source_directory(Define/Source SRC_Define)
aux_source_directory(HazeLog/Source SRC_HazeLog)
aux_source_directory(Library/Source SRC_Library)
aux_source_directory(Memory/Source SRC_Memory)
aux_source_directory(MetaData/Source SRC_MetaData)
aux_source_directory(VM/Source SRC_VM)
aux_source_directory(ThirdParty/OpenJson SRC_OpenJson)

aux_source_directory(Source SRC_MAIN)

#[[add_subdirectory(AST)
add_subdirectory(Backend)
add_subdirectory(Compiler)
add_subdirectory(Debugger)
add_subdirectory(Define)
add_subdirectory(HazeLog)
add_subdirectory(Library)
add_subdirectory(Memory)
add_subdirectory(MetaData)
add_subdirectory(Source)
add_subdirectory(ThirdParty)
add_subdirectory(VM)]]

add_compile_definitions(HAZE _UNICODE UNICODE)

add_executable(Haze ${SRC_MAIN} ${SRC_AST} ${SRC_Backend} ${SRC_Compiler} ${SRC_Debugger} ${SRC_Define} 
				${SRC_HazeLog} ${SRC_Library} ${SRC_Memory} ${SRC_MetaData} ${SRC_VM} ${SRC_OpenJson})
