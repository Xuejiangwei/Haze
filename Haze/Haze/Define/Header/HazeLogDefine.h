#pragma once
#include "HazeLog.h"

#define PARSE_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<½âÎö´íÎó>£º" HAZE_TEXT(" ¡¾<%s>Ä£¿é <%d>ÐÐ¡¿\n") HAZE_TEXT(INFO), \
							 m_Compiler->GetCurrModuleName().c_str(), m_LineCount, __VA_ARGS__)

#define COMPILER_ERR_W(INFO, ...) HAZE_LOG_ERR_W("<±àÒë´íÎó>£º" HAZE_TEXT(" ¡¾<%s>Ä£¿é¡¿\n") HAZE_TEXT(INFO), \
							 m_Module->GetName().c_str(), __VA_ARGS__)

#define COMPILER_ERR_MODULE_W(INFO, ...) HAZE_LOG_ERR_W("<±àÒë´íÎó>£º" HAZE_TEXT(INFO) HAZE_TEXT(" ¡¾<%s>Ä£¿é¡¿\n"), \
							 __VA_ARGS__)