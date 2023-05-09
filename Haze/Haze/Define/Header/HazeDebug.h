#pragma once

#define HAZE_INS_LOG 1

#define BACKEND_INSTRUCTION_LOG 1

#define HAZE_TO_DO(X) HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("%s\n"), L"Haze to do : " L###X)