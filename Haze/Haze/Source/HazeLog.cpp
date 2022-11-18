#include <stdio.h>
#include <windows.h>

#include "HazeLog.h"

static void SetSystemColor(int type = 0)
{
	switch (type)
	{
	case HazeLog::Info:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	case HazeLog::Warning:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		break;
	case HazeLog::Error:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		break;
	default:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	}
}

void HazeLog::LogInfo(int type, const HAZE_CHAR* format, ...)
{
	SetSystemColor(type);
	va_list st;
	va_start(st, format);
	vfprintf(stderr, (char*)format, st);
	va_end(st);
	SetSystemColor();
}