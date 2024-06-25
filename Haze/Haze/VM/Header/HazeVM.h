#pragma once

//#include <memory>
#include <unordered_set>
#include "HazeVariable.h"

class HazeCompiler;

class HazeDebugger;
class HazeStack;
class GarbageCollection;

enum class HazeRunType : uint8
{
	Debug,
	Release,
};

class HazeVM
{
public:
	friend class HazeDebugger;
	friend class InstructionProcessor;
	friend class HazeMemory;
	friend class HazeStack;
	friend class HazeExecuteFile;

	HazeVM(HazeRunType GenType);

	~HazeVM();

	void InitVM(std::vector<HAZE_STRING> Vector_ModulePath);

	bool IsDebug() const { return GenType == HazeRunType::Debug; }

	void LoadStandardLibrary(std::vector<HAZE_STRING> Vector_ModulePath);

	const std::vector<Instruction>& GetInstruction() const { return Instructions; }

	void CallFunction(const HAZE_CHAR* functionName, ...);

	void CallFunction(FunctionData* functionData, ...);

	void ParseString(const HAZE_CHAR* moduleName, const HAZE_CHAR* moduleCode);

	void ParseFile(const HAZE_STRING& FilePath);

	std::unique_ptr<HazeCompiler>& GetCompiler() { return m_Compiler; }

	const std::unordered_set<HAZE_STRING>& GetReferenceModules() const { return HashSet_RefModule; }

public:
	const HAZE_STRING* GetModuleNameByCurrFunction();

	int GetFucntionIndexByName(const HAZE_STRING& m_Name);

	const FunctionData& GetFunctionByName(const HAZE_STRING& m_Name);

	const HAZE_STRING& GetHazeStringByIndex(int Index) const { return Vector_StringTable[Index].second; }

	char* GetGlobalValueByIndex(uint32 Index);

	ClassData* FindClass(const HAZE_STRING& m_ClassName);

	uint32 GetClassSize(const HAZE_STRING& m_ClassName);

private:
	void DynamicInitializerForGlobalData();

	void OnExecLine(uint32 Line);

	void InstructionExecPost();

	uint32 GetNextLine(uint32 CurrLine);

	uint32 GetNextInstructionLine(uint32 currLine);

	std::pair<HAZE_STRING, uint32> GetStepIn(uint32 CurrLine);

	uint32 GetCurrCallFunctionLine();

	uint64 GetRegisterArrayLength(uint64 address);

private:
	std::unique_ptr<HazeCompiler> m_Compiler;

private:
	//std::unordered_map<HAZE_STRING, std::unique_ptr<Module>> MapModule;
	std::unordered_set<HAZE_STRING> MapString;

	std::unique_ptr<HazeStack> VMStack;

	std::unordered_set<HAZE_STRING> HashSet_RefModule;

private:
	std::vector<ModuleData> Vector_ModuleData;
	std::vector<HAZE_STRING> m_ModuleFilePath;

	std::vector<std::pair<HazeVariable, bool>> Vector_GlobalData; //是否初始化
	std::vector<std::pair<uint32, uint32>> m_GlobalDataInitAddress;	//全局变量初始化开始地址与结束地址

	std::vector<std::pair<HAZE_STRING, HAZE_STRING>> Vector_StringTable;

	std::vector<ClassData> Vector_ClassTable;

	std::vector<FunctionData> Vector_FunctionTable;
	std::unordered_map<HAZE_STRING, uint32> HashMap_FunctionTable;

	std::vector<Instruction> Instructions;

	std::vector<char> Vector_GlobalDataClassObjectMemory;

	std::unordered_map<uint64, uint64> Vector_ArrayCache;

	HazeRunType GenType;
};
