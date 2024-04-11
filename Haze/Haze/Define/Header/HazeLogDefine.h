#pragma once
#include "HazeLog.h"

#define PARSE_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<��������>��" HAZE_TEXT(" ��<%s>ģ�� <%d>�� ��") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_LineCount, __VA_ARGS__)

#define COMPILER_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<�������>��" HAZE_TEXT(" ��<%s>ģ�顿") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 m_Module->GetName().c_str(), __VA_ARGS__)

#define COMPILER_ERR_MODULE_W(INFO, ...) HAZE_LOG_ERR_W("<�������>��" HAZE_TEXT(" ��<%s>ģ�顿") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 __VA_ARGS__)

#define INS_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<���д���>��" HAZE_TEXT(" ��%s ָ�") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 GetInstructionString(stack->m_VM->Instructions[stack->m_PC].InsCode), __VA_ARGS__)

#define AST_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<�﷨��������>��" HAZE_TEXT(" ��<%s>ģ��<%s>����<%d>�С�") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str(), m_Location.Line, \
							 __VA_ARGS__)