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