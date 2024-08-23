#pragma once

#include "HazeHeader.h"
#include "HazeVariable.h"

class Compiler;

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

	void InitVM(V_Array<HString> Vector_ModulePath);

	bool IsDebug() const { return GenType == HazeRunType::Debug; }

	void LoadStandardLibrary(V_Array<HString> Vector_ModulePath);

	const V_Array<Instruction>& GetInstruction() const { return Instructions; }

	void CallFunction(const HChar* functionName, ...);

	void CallFunction(FunctionData* functionData, ...);

	void* CreateHazeClass(const HString& className, ...);

	void ParseString(const HChar* moduleName, const HChar* moduleCode);

	void ParseFile(const HString& FilePath);

	Unique<Compiler>& GetCompiler() { return m_Compiler; }

	const HashSet<HString>& GetReferenceModules() const { return HashSet_RefModule; }

public:
	const HString* GetModuleNameByCurrFunction();

	int GetFucntionIndexByName(const HString& m_Name);

	const FunctionData& GetFunctionByName(const HString& m_Name);

	const class ObjectString* GetConstantStringByIndex(int index) const;

	char* GetGlobalValueByIndex(uint32 Index);

	ClassData* FindClass(const HString& className);

	uint32 GetClassSize(const HString& className);

private:
	void InitGlobalStringCount(uint64 count);
	void SetGlobalString(uint64 index, const HString& str);

	const HString* GetSymbolClassName(const HString& name);

	void ResetSymbolClassIndex(const HString& name, uint64 index);

	void DynamicInitializerForGlobalData();

	void OnExecLine(uint32 Line);

	void InstructionExecPost();

	uint32 GetNextLine(uint32 CurrLine);

	uint32 GetNextInstructionLine(uint32 currLine);

	Pair<HString, uint32> GetStepIn(uint32 CurrLine);

	uint32 GetCurrCallFunctionLine();

private:
	Unique<Compiler> m_Compiler;

private:
	//HashMap<HString, Unique<Module>> MapModule;
	HashSet<HString> MapString;

	Unique<HazeStack> VMStack;

	HashSet<HString> HashSet_RefModule;

private:
	V_Array<ModuleData> Vector_ModuleData;
	V_Array<HString> m_ModuleFilePath;

	V_Array<Pair<HazeVariable, bool>> Vector_GlobalData; //是否初始化
	V_Array<Pair<uint32, uint32>> m_GlobalDataInitAddress;	//全局变量初始化开始地址与结束地址

	V_Array<class ObjectString*> m_StringTable;

	HashMap<HString, uint64> m_ClassSymbol;
	V_Array<ClassData> Vector_ClassTable;

	V_Array<FunctionData> Vector_FunctionTable;
	HashMap<HString, uint32> HashMap_FunctionTable;

	V_Array<Instruction> Instructions;

	V_Array<char> Vector_GlobalDataClassObjectMemory;

	HazeRunType GenType;
};
