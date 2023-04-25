#pragma once

#define HAZE_INS_LOG 1

#define BACKEND_INSTRUCTION_LOG 1

#define HAZE_TO_DO(X) HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("%s\n"), L"Haze to do:" L###X)

#define HAZE_TEST_FOLDER			HAZE_TEXT("\\Code\\")
#define HAZE_TEST_FILE_PATH			HAZE_TEXT("HazeCode.hz")
#define HAZE_TEST_FILE				HAZE_TEXT("HazeCode")