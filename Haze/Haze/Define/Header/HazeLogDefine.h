#pragma once
#include "HazeLog.h"

#define PARSE_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<½âÎö´íÎó>£º" H_TEXT(" ¡¾<%s>Ä£¿é <%d>ÐÐ ¡¿") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_LineCount, __VA_ARGS__)

#define COMPILER_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<±àÒë´íÎó>£º" H_TEXT(" ¡¾<%s>Ä£¿é¡¿") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Module->GetName().c_str(), __VA_ARGS__)

#define COMPILER_ERR_MODULE_W(INFO, ...) HAZE_LOG_ERR_W("<±àÒë´íÎó>£º" H_TEXT(" ¡¾<%s>Ä£¿é¡¿") H_TEXT(INFO) H_TEXT("!\n"), \
							 __VA_ARGS__)

#define BACKEND_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<ºó¶Ë´íÎó>£º" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)

#define INS_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<ÔËÐÐ´íÎó>£º" H_TEXT(" ¡¾%s Ö¸Áî¡¿") H_TEXT(INFO) H_TEXT("!\n"), \
							 GetInstructionString(stack->m_VM->Instructions[stack->m_PC].InsCode), __VA_ARGS__)

#define AST_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<Óï·¨·ÖÎö´íÎó>£º" H_TEXT(" ¡¾<%s>Ä£¿é<%d>ÐÐ¡¿") H_TEXT(INFO) H_TEXT("!\n"), \
							 m_Compiler->GetCurrModuleName().c_str(), m_Location.Line, \
							 __VA_ARGS__)

#define GC_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<À¬»ø»ØÊÕ´íÎó>£º" H_TEXT(INFO) H_TEXT("!\n"), __VA_ARGS__)