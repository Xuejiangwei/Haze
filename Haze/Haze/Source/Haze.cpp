#include "HazePch.h"


#include "Haze.h"
#include "HazeVM.h"
#include "HazeMemory.h"
#include "HazeLog.h"
#include "HazeUtility.h"

#include "HazeLibraryManager.h"
#include "HazeDebugger.h"

extern Unique<HazeDebugger> g_Debugger;
extern Unique<HazeLibraryManager> g_HazeLibManager;

bool g_IsHazeEnd = false;
std::wstring g_HazeExePath;
std::wstring g_MainFilePath;
Unique<V_Array<std::wstring>> g_CustomPaths;

void HazeNewHandler()
{
	HAZE_LOG_ERR_W("没有足够内存可分配了!!!!\n");
}

void HazePreInit()
{
	std::set_new_handler(HazeNewHandler);
	setlocale(LC_ALL, "chs");
	std::wcout.imbue(std::locale("chs"));
	std::locale::global(std::locale(std::locale::classic(), "", std::locale::ctype));
};

void HazeEnd()
{
	HazeMemory::GetMemory()->TryGC(true);

	if (g_Debugger)
	{
		g_Debugger->SendProgramEnd();
	}

	if (g_CustomPaths)
	{
		g_CustomPaths.release();
	}

	g_IsHazeEnd = true;
	std::cout << HAZE_ENDL << HAZE_ENDL << "Haze End!" << HAZE_ENDL;
	system("pause");
}

void HazeExit()
{
	if (g_Debugger)
	{
		g_Debugger->SendProgramEnd();
		g_Debugger.release();
	}

	/*if (!IsHazeEnd)
	{
		HazeEnd();
	}*/
}

enum class ParamType
{
	MainFile,
	MainFunction,
	DebugType,
	LoadLibraryDll,
	Files,
	IncludeFilePath,
};

int GetParam(ParamType type, char** paramArray, int length)
{
	static HashMap<ParamType, const char*> HashMap_Param =
	{
		{ ParamType::MainFile, "-m" },
		{ ParamType::MainFunction, "-mf" },
		{ ParamType::DebugType, "-d" },
		{ ParamType::LoadLibraryDll, "-ld" },
		{ ParamType::Files, "-f" },
		{ ParamType::IncludeFilePath, "-ifp" },
	};

	auto Iter = HashMap_Param.find(type);
	if (Iter != HashMap_Param.end())
	{
		for (int i = 0; i < length; i++)
		{
			if (i + 1 < length && strcmp(paramArray[i], Iter->second) == 0)
			{
				return i + 1;
			}
		}
	}

	return 0;
}

//解析文本  --->  生成字节码   --->  用虚拟机解析字节码，并执行
HazeVM* HazeMain(int argCount, char* argValue[])
{
	atexit(HazeExit);

	for (int i = 0; i < argCount; i++)
	{
		HAZE_LOG_INFO_W("Param<%d><%s>\n", i, String2WString(argValue[i]).c_str());
	}

	HazePreInit();

	std::filesystem::path ExeFile(argValue[0]);
	g_HazeExePath = ExeFile.parent_path().wstring() + H_TEXT("\\");

	char* mainFilePath = nullptr;
	HString mainFunction;
	if (argCount < 2)
	{
		return 0;
	}
	else if (argCount == 2)
	{
		mainFilePath = argValue[1];
	}
	else
	{
		mainFilePath = argValue[GetParam(ParamType::MainFile, argValue, argCount)];
	}

	HazeRunType runType = GetParam(ParamType::DebugType, argValue, argCount) != 0 ?
		strcmp(argValue[GetParam(ParamType::DebugType, argValue, argCount)], "debug") == 0 ? 
		HazeRunType::Debug : HazeRunType::Release : HazeRunType::Release;

	if (GetParam(ParamType::MainFunction, argValue, argCount) > 0)
	{
		mainFunction = String2WString(argValue[GetParam(ParamType::MainFunction, argValue, argCount)]);
	}

	std::filesystem::path mainFile(mainFilePath);
	g_MainFilePath = mainFile.parent_path().wstring() + H_TEXT("\\");

	std::filesystem::create_directory(g_MainFilePath + HAZE_FILE_INTER);
	std::filesystem::create_directory(g_MainFilePath + HAZE_FILE_PATH_BIN);

	for (int dll_Index = 0; dll_Index < argCount; dll_Index += 2)
	{
		dll_Index = GetParam(ParamType::LoadLibraryDll, argValue + dll_Index, argCount - dll_Index);
		if (dll_Index > 0)
		{
			g_HazeLibManager->LoadDLLLibrary(String2WString(argValue[dll_Index]), String2WString(argValue[dll_Index + 1]));
		}
		else
		{
			break;
		}
	}

	// 添加自定义的引用文件路径
	{
		int index = GetParam(ParamType::IncludeFilePath, argValue, argCount);
		if (index > 0)
		{
			if (!g_CustomPaths)
			{
				g_CustomPaths = MakeUnique<V_Array<std::wstring>>();
			}
			g_CustomPaths->clear();

			std::string str = argValue[index];
			char* s = new char[str.size() + 1];
			s[str.size()] = '\0';
			strcpy_s(s, str.size() + 1, str.c_str());
			char* p = nullptr;
			char* token = strtok_s(s, "+p", &p);
			while (token)
			{
				g_CustomPaths->push_back(String2WString(token));
				token = strtok_s(NULL, "+p", &p);
			}
		}
	}

	auto vm = new HazeVM(runType);

	{
		V_Array<HString> files;
		files.push_back(mainFile);

		int index = GetParam(ParamType::Files, argValue, argCount);
		if (index > 0)
		{
			std::string str = argValue[index];
			char* s = new char[str.size() + 1];
			s[str.size()] = '\0';
			strcpy_s(s, str.size() + 1, str.c_str());
			char* p = nullptr;
			char* token = strtok_s(s, " ", &p);
			while (token)
			{
				files.push_back(String2WString(token));
				token = strtok_s(NULL, " ", &p);
			}
		}

		if (!vm->InitVM(files))
		{
			vm = nullptr;
		}
	}

	//VM.LoadStandardLibrary({ {Path + H_TEXT("\\Code\\HazeCode.hz"), HAZE_TEST_FILE} });
	//VM.ParseFile(Path + H_TEXT("\\Other\\HazeCode.hz"), H_TEXT("HazeCode"));

	if (vm && vm->GetFucntionIndexByName(mainFunction) >= 0)
	{
		std::cout << HAZE_ENDL << HAZE_ENDL << "Haze Start" << HAZE_ENDL << HAZE_ENDL;
		vm->CallFunction(mainFunction.c_str());

		vm->ClearGlobalData();
		HazeEnd();
	}

	return vm;
}