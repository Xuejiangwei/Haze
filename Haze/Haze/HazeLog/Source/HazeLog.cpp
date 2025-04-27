#include "HazePch.h"
#include <stdio.h>
#include <mutex>
#include <windows.h>
#include "HazeLog.h"

static std::mutex g_LogMutex;
static char s_CahceLog[512];
static void (*s_WPrintFunction)(int type, const wchar_t* str);
static void (*s_PrintFunction)(int type, const char* str);

void HazeLog::SetSystemColor(int type)
{
	switch (type)
	{
	case HazeLog::Info:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),/* FOREGROUND_INTENSITY | FOREGROUND_RED |*/ FOREGROUND_GREEN /*| FOREGROUND_BLUE*/);
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

void HazeLog::SetWPrintFunction(void(*wprintFunction)(int type, const wchar_t* str))
{
	s_WPrintFunction = wprintFunction;
}

void HazeLog::SetPrintFunction(void(*printFunction)(int type, const char* str))
{
	s_PrintFunction = printFunction;
}

void HazeLog::LogInfo(int type, const wchar_t* format, ...)
{
	std::lock_guard lock(g_LogMutex);

	SetSystemColor(type);
	va_list st;
	va_start(st, format);
	//vwprintf(format, st);
	vswprintf_s((x_HChar*)s_CahceLog, sizeof(s_CahceLog) / 2, format, st);
	va_end(st);
	SetSystemColor();

	if (s_WPrintFunction)
	{
		s_WPrintFunction(type, (x_HChar*)s_CahceLog);
	}
}

void HazeLog::LogInfo(int type, const char* format, ...)
{
	std::lock_guard lock(g_LogMutex);

	SetSystemColor(type);
	va_list st;
	va_start(st, format);
	//vprintf(format, st);
	vsprintf_s(s_CahceLog, format, st);
	va_end(st);
	SetSystemColor();

	if (s_PrintFunction)
	{
		s_PrintFunction(type, s_CahceLog);
	}
}

//void HazeLog::LogInfo(int type, const HChar* format)
//{
//	SetSystemColor(type);
//	std::cout << format;
//	SetSystemColor();
//}