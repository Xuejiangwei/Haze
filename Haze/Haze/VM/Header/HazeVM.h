#pragma once

#include "Haze.h"
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

	using ModulePair = std::pair<HAZE_STRING, HAZE_STRING>;

	void InitVM(std::vector<ModulePair> Vector_ModulePath);

	bool IsDebug() const { return GenType == HazeRunType::Debug; }

	void LoadStandardLibrary(std::vector<ModulePair> Vector_ModulePath);

	const std::vector<Instruction>& GetInstruction() const { return Vector_Instruction; }

	void StartMainFunction();

	//void ParseString(const HAZE_STRING& String);

	void ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& m_ModuleName);

	std::unique_ptr<HazeCompiler>& GetCompiler() { return m_Compiler; }

	const std::unordered_set<HAZE_STRING>& GetReferenceModules() const { return HashSet_RefModule; }

public:
	const HAZE_STRING* GetModuleNameByCurrFunction();

	int GetFucntionIndexByName(const HAZE_STRING& m_Name);

	const FunctionData& GetFunctionByName(const HAZE_STRING& m_Name);

	const HAZE_STRING& GetHazeStringByIndex(int Index) const { return Vector_StringTable[Index].second; }

	void* GetGlobalValue(const HAZE_STRING& m_Name);

	char* GetGlobalValueByIndex(uint32 Index);

	ClassData* FindClass(const HAZE_STRING& ClassName);

	uint32 GetClassSize(const HAZE_STRING& ClassName);

private:
	void OnExecLine(uint32 Line);

	void InstructionExecPost();

	uint32 GetNextLine(uint32 CurrLine);

	uint32 GetCurrCallFunctionLine();

	static void Hook(HazeVM* VM);

private:
	std::unique_ptr<HazeCompiler> m_Compiler;

private:
	//std::unordered_map<HAZE_STRING, std::unique_ptr<Module>> MapModule;
	std::unordered_set<HAZE_STRING> MapString;

	std::unique_ptr<HazeStack> VMStack;

	std::unordered_set<HAZE_STRING> HashSet_RefModule;

private:
	std::vector<ModuleData> Vector_ModuleData;

	std::vector<HazeVariable> Vector_GlobalData;

	std::vector<std::pair<HAZE_STRING, HAZE_STRING>> Vector_StringTable;

	std::vector<ClassData> Vector_ClassTable;

	std::vector<FunctionData> Vector_FunctionTable;
	std::unordered_map<HAZE_STRING, uint32> HashMap_FunctionTable;

	std::vector<Instruction> Vector_Instruction;

	std::vector<char> Vector_GlobalDataClassObjectMemory;

	HazeRunType GenType;
};
