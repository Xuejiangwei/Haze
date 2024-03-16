#pragma once

#include "HazeHeader.h"

enum class LibraryLoadState
{
	UnLoad,
	Load,
};

struct LibraryData
{
	LibraryLoadState State;
	void* Address;
	HAZE_STRING LibraryPath;
	HAZE_STRING FilePath;
};

class HazeLibraryManager
{
public:
	HazeLibraryManager();

	~HazeLibraryManager();

	void ExecuteDLLFunction(const HAZE_STRING& moduleName, const HAZE_STRING& functionName, char* paramStartAddress, char* retStartAddress, void* stack, 
		void(*exeHazeFunctionCall)(void*, void*, int, ...));

	void LoadDLLLibrary(const HAZE_STRING& libraryPath, const HAZE_STRING& filePath);

	void UnloadDLLLibrary(const HAZE_STRING& libraryPath);

	const HAZE_STRING* TryGetFilePath(const HAZE_STRING& moduleName);

	const void* GetExeAddress();

	static void LoadStdLibrary();

private:
	std::unordered_map<HAZE_STRING, LibraryData> m_Libraries;
};
