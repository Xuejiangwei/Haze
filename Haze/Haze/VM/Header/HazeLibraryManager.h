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
	HString LibraryPath;
	HString FilePath;
};

class HazeLibraryManager
{
public:
	HazeLibraryManager();

	~HazeLibraryManager();

	void ExecuteDLLFunction(const HString& moduleName, const HString& functionName, char* paramStartAddress, char* retStartAddress, void* stack);

	void LoadDLLLibrary(const HString& libraryPath, const HString& filePath);

	void UnloadDLLLibrary(const HString& libraryPath);

	const HString* TryGetFilePath(const HString& moduleName);

	const void* GetExeAddress();

	static void LoadStdLibrary();

private:
	HashMap<HString, LibraryData> m_Libraries;
};
