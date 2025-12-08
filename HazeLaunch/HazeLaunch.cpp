#include "Haze.h"
#include "HazeLog.h"
#include <stdio.h>
#include <Windows.h>

void HazeWPrint(int type, const wchar_t* str);
void HazePrint(int type, const char* str);

int main(int ArgCount, char* ArgValue[])
{
	SetConsoleOutputCP(CP_UTF8);
	HazeLog::SetWPrintFunction(&HazeWPrint);
	HazeLog::SetPrintFunction(&HazePrint);
	HazeMain(ArgCount, ArgValue);

	return 0;
}

void HazeWPrint(int type, const wchar_t* str)
{
	HazeLog::SetSystemColor(type);
	wprintf(L"%s", str);
	HazeLog::SetSystemColor();
}

void HazePrint(int type, const char* str)
{
	HazeLog::SetSystemColor(type);
	printf("%s", str);
	HazeLog::SetSystemColor();
}