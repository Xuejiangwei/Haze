#pragma once

#include "HazeCompilerValue.h"

class HazeCompilerModule;
class HazeCompilerClass;
class HazeBaseBlock;

class HazeCompilerFunction
{
public:
	friend class HazeCompiler;
	friend class HazeCompilerModule;

	HazeCompilerFunction(HazeCompilerModule* compilerModule, const HString& name, HazeDefineType& type,
		V_Array<HazeDefineVariable>& params, HazeCompilerClass* compilerClass = nullptr);

	~HazeCompilerFunction();

	void SetStartEndLine(uint32 startLine, uint32 endLine);

	Share<HazeCompilerValue> GetLocalVariable(const HString& VariableName);

	const HString& GetName() const { return m_Name; }

	const HazeDefineType& GetFunctionType() const { return m_Type; }

	HazeCompilerModule* GetModule() const { return m_Module; }

	HazeCompilerClass* GetClass() const { return m_OwnerClass; }

	Share<HazeBaseBlock> GetEntryBlock() { return m_EntryBlock; }

	void FunctionFinish();

	void GenI_Code(HAZE_STRING_STREAM& hss);

	HString GenDafaultBlockName();

	HString GenIfThenBlockName();

	HString GenElseBlockName();

	HString GenLoopBlockName();

	HString GenWhileBlockName();

	HString GenForBlockName();

	HString GenForConditionBlockName();

	HString GenForStepBlockName();

	bool FindLocalVariableName(const Share<HazeCompilerValue>& value, HString& outName);

	bool FindLocalVariableName(const HazeCompilerValue* value, HString& outName);

	bool HasExceptThisParam() const;

	void AddLocalVariable(Share<HazeCompilerValue> value, int line);

	const HazeDefineType& GetParamTypeByIndex(uint64 index);

	const HazeDefineType& GetParamTypeLeftToRightByIndex(uint64 index);

private:
	void AddFunctionParam(const HazeDefineVariable& variable);

	Share<HazeCompilerValue> CreateLocalVariable(const HazeDefineVariable& variable, int line,
		Share<HazeCompilerValue> refValue = nullptr,
		V_Array<Share<HazeCompilerValue>> arraySize = {},
		V_Array<HazeDefineType>* params = nullptr);

	Share<HazeCompilerValue> CreateNew(const HazeDefineType& data, V_Array<Share<HazeCompilerValue>>* countValue);

	void InitEntryBlock(Share<HazeBaseBlock> block) { m_EntryBlock = block; }

private:
	HazeCompilerModule* m_Module;
	HazeCompilerClass* m_OwnerClass;

	HString m_Name;
	HazeDefineType m_Type;

	V_Array<Pair<HString, Share<HazeCompilerValue>>> m_Params;	//从右到左加入参数

	V_Array<Pair<Share<HazeCompilerValue>, int>> m_LocalVariables;

	Share<HazeBaseBlock> m_EntryBlock;

	int m_CurrBlockCount;
	int m_CurrVariableCount;
	
	uint32 m_StartLine;
	uint32 m_EndLine;
};
