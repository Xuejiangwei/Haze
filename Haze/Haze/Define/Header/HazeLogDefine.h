#pragma once
#include "HazeLog.h"

#define PARSE_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<½âÎö´íÎó>£º" HAZE_TEXT(" ¡¾<%s>Ä£¿é <%d>ÐÐ ¡¿") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_LineCount, __VA_ARGS__)

#define COMPILER_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<±àÒë´íÎó>£º" HAZE_TEXT(" ¡¾<%s>Ä£¿é¡¿") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 m_Module->GetName().c_str(), __VA_ARGS__)

#define COMPILER_ERR_MODULE_W(INFO, ...) HAZE_LOG_ERR_W("<±àÒë´íÎó>£º" HAZE_TEXT(" ¡¾<%s>Ä£¿é¡¿") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 __VA_ARGS__)

#define INS_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<ÔËÐÐ´íÎó>£º" HAZE_TEXT(" ¡¾%s Ö¸Áî¡¿") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 GetInstructionString(stack->m_VM->Instructions[stack->m_PC].InsCode), __VA_ARGS__)

#define AST_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<Óï·¨·ÖÎö´íÎó>£º" HAZE_TEXT(" ¡¾<%s>Ä£¿é<%s>º¯Êý<%d>ÐÐ¡¿") HAZE_TEXT(INFO) HAZE_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_Compiler->GetCurrModule()->GetCurrFunction()->GetName().c_str(), m_Location.Line, \
							 __VA_ARGS__)

#define GC_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<À¬»ø»ØÊÕ´íÎó>£º" HAZE_TEXT(INFO) HAZE_TEXT("!\n"), __VA_ARGS__)