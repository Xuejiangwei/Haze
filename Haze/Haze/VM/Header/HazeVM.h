#pragma once

#include "HazeHeader.h"
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

	void InitVM(V_Array<HString> Vector_ModulePath);

	bool IsDebug() const { return GenType == HazeRunType::Debug; }

	void LoadStandardLibrary(V_Array<HString> Vector_ModulePath);

	const V_Array<Instruction>& GetInstruction() const { return Instructions; }

	void CallFunction(const HChar* functionName, ...);

	void CallFunction(FunctionData* functionData, ...);

	void* CreateHazeClass(const HString& className, ...);

	void ParseString(const HChar* moduleName, const HChar* moduleCode);

	void ParseFile(const HString& FilePath);

	Unique<HazeCompiler>& GetCompiler() { return m_Compiler; }

	const std::unordered_set<HString>& GetReferenceModules() const { return HashSet_RefModule; }

public:
	const HString* GetModuleNameByCurrFunction();

	int GetFucntionIndexByName(const HString& m_Name);

	const FunctionData& GetFunctionByName(const HString& m_Name);

	const HChar* GetConstantStringByIndex(int Index) const;

	char* GetGlobalValueByIndex(uint32 Index);

	ClassData* FindClass(const HString& m_ClassName);

	uint32 GetClassSize(const HString& m_ClassName);

private:
	void DynamicInitializerForGlobalData();

	void OnExecLine(uint32 Line);

	void InstructionExecPost();

	uint32 GetNextLine(uint32 CurrLine);

	uint32 GetNextInstructionLine(uint32 currLine);

	Pair<HString, uint32> GetStepIn(uint32 CurrLine);

	uint32 GetCurrCallFunctionLine();

	uint64 GetRegisterArrayLength(uint64 address);

private:
	Unique<HazeCompiler> m_Compiler;

private:
	//HashMap<HString, Unique<Module>> MapModule;
	std::unordered_set<HString> MapString;

	Unique<HazeStack> VMStack;

	std::unordered_set<HString> HashSet_RefModule;

private:
	V_Array<ModuleData> Vector_ModuleData;
	V_Array<HString> m_ModuleFilePath;

	V_Array<Pair<HazeVariable, bool>> Vector_GlobalData; //�Ƿ��ʼ��
	V_Array<Pair<uint32, uint32>> m_GlobalDataInitAddress;	//ȫ�ֱ�����ʼ����ʼ��ַ�������ַ

	V_Array<HString> Vector_StringTable;

	V_Array<ClassData> Vector_ClassTable;

	V_Array<FunctionData> Vector_FunctionTable;
	HashMap<HString, uint32> HashMap_FunctionTable;

	V_Array<Instruction> Instructions;

	V_Array<char> Vector_GlobalDataClassObjectMemory;

	HashMap<uint64, uint64> Vector_ArrayCache;

	HazeRunType GenType;
};
