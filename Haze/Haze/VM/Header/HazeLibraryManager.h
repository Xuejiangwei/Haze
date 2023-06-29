#pragma once

#include "Haze.h"

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

	void ExecuteDLLFunction(const HAZE_STRING& ModuleName, const HAZE_STRING& FunctionName, char* ParamStartAddress, char* RetStartAddress);

	void LoadDLLLibrary(const HAZE_STRING& LibraryPath, const HAZE_STRING& FilePath);

	void UnloadDLLLibrary(const HAZE_STRING& LibraryPath);

	const HAZE_STRING* TryGetFilePath(const HAZE_STRING& ModuleName);

	const void* GetExeAddress();

	static void LoadStdLibrary();

private:
	std::unordered_map<HAZE_STRING, LibraryData> HashMap_Library;
};
