#pragma once

#include "Haze.h"
//#include <memory>
#include <unordered_set>


#include "HazeVariable.h"

class HazeCompiler;

class HazeStack;
class GarbageCollection;

class HazeVM
{
public:
	friend class InstructionProcessor;
	friend class GarbageCollection;
	friend class HazeStack;
	friend class HazeExecuteFile;

	HazeVM();
	~HazeVM();

	using ModulePair = std::pair<HAZE_STRING, HAZE_STRING>;

	void InitVM(std::vector<ModulePair> Vector_ModulePath);

	void LoadStandardLibrary(std::vector<ModulePair> Vector_ModulePath);

	const std::vector<Instruction>& GetInstruction() const { return Vector_Instruction; }

	void StartMainFunction();

	//void ParseString(const HAZE_STRING& String);

	void ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName);

	std::unique_ptr<HazeCompiler>& GetCompiler() { return Compiler; }

	const std::unordered_set<HAZE_STRING>& GetReferenceModules() const { return HashSet_RefModule; }

	int GetFucntionIndexByName(const HAZE_STRING& Name);

	const FunctionData& GetFunctionByName(const HAZE_STRING& Name);

	const HAZE_STRING& GetHazeStringByIndex(int Index) const { return Vector_StringTable[Index].second; }

	HazeValue* GetGlobalValue(const HAZE_STRING& Name);

	ClassData* FindClass(const HAZE_STRING& ClassName);

	unsigned int GetClassSize(const HAZE_STRING& ClassName);

private:
	void SetHook();

private:
	std::unique_ptr<HazeCompiler> Compiler;

private:
	//std::unordered_map<HAZE_STRING, std::unique_ptr<Module>> MapModule;
	std::unordered_set<HAZE_STRING> MapString;

	std::unique_ptr<HazeStack> VMStack;
	
	std::unique_ptr<GarbageCollection> GC;

	std::pair<HazeDefineType, HazeValue> FunctionReturn;

	std::unordered_set<HAZE_STRING> HashSet_RefModule;

	std::vector<HazeVariable> Vector_GlobalData;

	std::vector<std::pair<HAZE_STRING, HAZE_STRING>> Vector_StringTable;

	std::vector<ClassData> Vector_ClassTable;

	std::vector<FunctionData> Vector_FunctionTable;
	std::unordered_map<HAZE_STRING, unsigned int> HashMap_FunctionTable;

	std::vector<Instruction> Vector_Instruction;
};
