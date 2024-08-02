#pragma once
#include "HazeLog.h"

#define PARSE_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<��������>��" H_TEXT(" ��<%s>ģ�� <%d>�� ��") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_LineCount, __VA_ARGS__)

#define COMPILER_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<�������>��" H_TEXT(" ��<%s>ģ�顿") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Module->GetName().c_str(), __VA_ARGS__)

#define COMPILER_ERR_MODULE_W(INFO, ...) HAZE_LOG_ERR_W("<�������>��" H_TEXT(" ��<%s>ģ�顿") H_TEXT(INFO) H_TEXT("!\n"), \
							 __VA_ARGS__)

#define BACKEND_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<��˴���>��" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)

#define INS_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<���д���>��" H_TEXT(" ��%s ָ�") H_TEXT(INFO) H_TEXT("!\n"), \
							 GetInstructionString(stack->m_VM->Instructions[stack->m_PC].InsCode), __VA_ARGS__)

#define AST_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<�﷨��������>��" H_TEXT(" ��<%s>ģ��<%d>�С�") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_Location.Line, \
							 __VA_ARGS__)

#define GC_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<�������մ���>��" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)

#define GLOBAL_INIT_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<ȫ�����ݳ�ʼ������>��" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)