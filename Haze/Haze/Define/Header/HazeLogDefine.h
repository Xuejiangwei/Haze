#pragma once
#include "HazeLog.h"

#define PARSE_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<解析错误>：" H_TEXT(" 【<%s>模块 <%d>行 】") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_LineCount, __VA_ARGS__)

#define COMPILER_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<编译错误>：" H_TEXT(" 【<%s>模块】") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Module->GetName().c_str(), __VA_ARGS__)

#define COMPILER_ERR_MODULE_W(INFO, ...) HAZE_LOG_ERR_W("<编译错误>：" H_TEXT(" 【<%s>模块】") H_TEXT(INFO) H_TEXT("!\n"), \
							 __VA_ARGS__)

#define BACKEND_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<后端错误>：" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)

#define INS_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<运行错误>：" H_TEXT(" 【%s 指令】") H_TEXT(INFO) H_TEXT("!\n"), \
							 GetInstructionString(stack->m_VM->m_Instructions[stack->m_PC].InsCode), __VA_ARGS__)

#define INS_ERR_CODE_W(INFO, INS_CODE, ...) HAZE_LOG_ERR_W("<运行错误>：" H_TEXT(" 【%s 指令】") H_TEXT(INFO) H_TEXT("!\n"), \
							 GetInstructionString(INS_CODE), __VA_ARGS__)

#define AST_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<语法分析错误>：" H_TEXT(" 【<%s>模块<%d>行】") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_Location.Line, \
							 __VA_ARGS__); \
							 m_Compiler->MarkCompilerError()

#define GC_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<垃圾回收错误>：" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)

#define GLOBAL_INIT_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<全局数据初始化错误>：" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)

#define OBJECT_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<类对象调用错误>：" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__); \
								stack->LogStack()