#pragma once

class HazeLog
{
public:
	enum
	{
		Info = 1,
		Warning,
		Error
	};

	static void SetWPrintFunction(void (*wprintFunction)(int type, const wchar_t* str));
	static void SetPrintFunction(void (*printFunction)(int type, const char* str));

	static void SetSystemColor(int type = 0);

	static void LogInfo(int type, const wchar_t* format, ...);

	static void LogInfo(int type, const char* format, ...);
};

#define HAZE_LOG_ERR(FORMAT, ...) HazeLog::LogInfo(HazeLog::Error, FORMAT, ##__VA_ARGS__)

#define HAZE_LOG_ERR_W(FORMAT, ...) HazeLog::LogInfo(HazeLog::Error, H_TEXT(FORMAT), ##__VA_ARGS__)

#define HAZE_LOG_INFO(FORMAT, ...) HazeLog::LogInfo(HazeLog::Info, FORMAT, ##__VA_ARGS__)

#define HAZE_LOG_INFO_W(FORMAT, ...) HazeLog::LogInfo(HazeLog::Info, H_TEXT(FORMAT), ##__VA_ARGS__)

#define HAZE_TO_DO(X) HazeLog::LogInfo(HazeLog::Error, H_TEXT("%s\n"), L"Haze to do : " L###X)