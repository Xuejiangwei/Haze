#pragma once

#include "Haze.h"

class HazeLog
{
public:
	enum
	{
		Info = 1,
		Warning,
		Error
	};

	static void LogInfo(int type, const HAZE_CHAR* format, ...);

	static void LogInfo(int type, const char* format, ...);

};

#define HAZE_LOG_ERR(FORMAT, ...) HazeLog::LogInfo(HazeLog::Error, FORMAT, __VA_ARGS__)

#define HAZE_LOG_ERR_W(FORMAT, ...) HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT(FORMAT), __VA_ARGS__)

#define HAZE_LOG_INFO(FORMAT, ...) HazeLog::LogInfo(HazeLog::Info, FORMAT, __VA_ARGS__)

#define HAZE_TO_DO(X) HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("%s\n"), L"Haze to do : " L###X)