#pragma once

enum class LibraryLoadState
{
	UnLoad,
	Load,
};

struct LibraryData
{
	LibraryLoadState State;
	void* Address;
	STDString LibraryPath;
	STDString FilePath;
};

class HazeLibraryManager
{
public:
	HazeLibraryManager();

	~HazeLibraryManager();

	void ExecuteDLLFunction(const STDString& moduleName, const STDString& functionName, char* paramStartAddress, char* retStartAddress, void* stack);

	void LoadDLLLibrary(STDString libraryPath, STDString filePath);

	void UnloadDLLLibrary(const STDString& libraryPath);

	const STDString* TryGetFilePath(const STDString& moduleName);

	const void* GetExeAddress();

	static void LoadStdLibrary();

private:
	HashMap<STDString, LibraryData> m_Libraries;
};
