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
};