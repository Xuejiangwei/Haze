#pragma once

#include "HazeHeader.h"
#include "HazeVariable.h"

class Compiler;

class HazeDebugger;
class HazeStack;
class GarbageCollection;
class ObjectClass;
class ObjectString;
struct AdvanceFunctionInfo;

enum class HazeRunType : x_uint8
{
	Debug,
	Release,
};

class HazeVM
{
	friend class HazeDebugger;
	friend class InstructionProcessor;
	friend class HazeMemory;
	friend class HazeStack;
	friend class HazeExecuteFile;

public:
	HazeVM(HazeRunType GenType);

	~HazeVM();

	bool InitVM(V_Array<STDString> Vector_ModulePath);

	bool IsDebug() const { return GenType == HazeRunType::Debug; }

	const V_Array<Instruction>& GetInstruction() const { return m_Instructions; }

	void CallFunction(const x_HChar* functionName, ...);

	void CallFunction(const FunctionData* functionData, ...);

	void CallFunction(const FunctionData* functionData, va_list& args);

	AdvanceFunctionInfo* GetAdvanceFunction(x_uint16 index);

	STDString GetAdvanceFunctionName(x_uint16 index);

	ObjectClass* CreateObjectClass(const x_HChar* className, ...);

	bool ParseString(const x_HChar* moduleName, const x_HChar* moduleCode);

	STDString ParseFile(const STDString& FilePath);

	Unique<Compiler>& GetCompiler() { return m_Compiler; }

	const HashSet<STDString>& GetReferenceModules() const { return HashSet_RefModule; }

public:
	const STDString* GetModuleNameByCurrFunction();

	const STDString* GetFunctionNameByData(const FunctionData* data);

	int GetFucntionIndexByName(const STDString& name);

	const FunctionData& GetFunctionByName(const STDString& name, const x_HChar* className = nullptr);

	const FunctionData* GetFunctionDataByName(const STDString& name, const x_HChar* className = nullptr);

	const FunctionData* GetFunctionDataById(x_uint32 id);

	const class ObjectString* GetConstantStringByIndex(int index) const;

	char* GetGlobalValueById(x_uint32 id);

	ClassData* FindClass(x_uint32 typeId);
	ClassData* FindClass(const STDString& name);

	x_uint32 GetClassSize(x_uint32 typeId);

	x_uint32 AddGlobalValue(ObjectClass* value);

	void RemoveGlobalValue(x_uint32 index);

	void ClearGlobalData();

	HazeTypeInfoMap* GetTypeInfoMap() { return m_TypeInfoMap.get(); }

	//JIT
public:
	void JitFunction(const FunctionData* func);
	
	// 设置JIT优化级别
	void setJITOptimizationLevel(int level);

private:
	void InitRegisterObjectFunction();

	void InitGlobalStringCount(x_uint64 count);
	void SetGlobalString(x_uint64 index, const STDString& str);

	const STDString* GetSymbolClassName(const STDString& name);

	void ResetSymbolClassIndex(const STDString& name, x_uint64 index);

	void LoadDLLModules();
	
	void DynamicInitializerForGlobalData();

	void OnExecLine(x_uint32 Line);

	void InstructionExecPost();

	x_uint32 GetNextLine(x_uint32 currLine);

	x_uint32 GetNextInstructionLine(x_uint32 currLine);

	Pair<HStringView, x_uint32> GetStepIn(x_uint32 currLine);

	x_uint32 GetCurrCallFunctionLine();

private:
	Unique<Compiler> m_Compiler;

private:
	//HashMap<HString, Unique<Module>> MapModule;
	HashSet<STDString> MapString;

	Unique<HazeStack> m_Stack;

	HashSet<STDString> HashSet_RefModule;

private:
	V_Array<ModuleData> m_ModuleData;
	V_Array<STDString> m_ModuleFilePath;

	V_Array<x_uint64> m_GlobalInitFunction;
	HashMap<x_uint32, HazeVariable> m_GlobalData;
	V_Array<ObjectClass*> m_ExtreGlobalData;

	V_Array<ObjectString*> m_StringTable;

	HashMap<STDString, x_uint64> m_ClassSymbol;
	V_Array<ClassData> m_ClassTable;

	V_Array<FunctionData> m_FunctionTable;
	HashMap<STDString, x_uint32> m_HashFunctionTable;
	V_Array<AdvanceFunctionInfo*> m_FunctionObjectTable;

	V_Array<Instruction> m_Instructions;

	Unique<HazeTypeInfoMap> m_TypeInfoMap;

	HazeRunType GenType;
};
