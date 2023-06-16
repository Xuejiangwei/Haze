#pragma once

#include "Haze.h"

enum class LibraryLoadState
{
	UnLoad,
	Load,
};

class HazeLibraryManager
{
public:
	HazeLibraryManager();
	~HazeLibraryManager();

	void ExecuteDLLFunction(const HAZE_STRING& ModuleName, const HAZE_STRING& FunctionName, char* ParamStartAddress, char* RetStartAddress);

	void LoadDLLLibrary(const HAZE_STRING& LibraryPath, const HAZE_STRING& FilePath);

	void UnloadDLLLibrary(const HAZE_STRING& LibraryPath);

private:
	std::unordered_map<HAZE_STRING, std::pair<LibraryLoadState, void*>> HashMap_Library;
};
