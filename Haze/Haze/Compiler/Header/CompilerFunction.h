#pragma once

#include "CompilerValue.h"

class CompilerModule;
class CompilerClass;
class CompilerClassValue;
class CompilerBlock;

class CompilerFunction
{
	friend class Compiler;
	friend class CompilerModule;
	friend class ASTMultiExpression;

public:
	CompilerFunction(CompilerModule* compilerModule, const HString& name, HazeDefineType& type,
		V_Array<HazeDefineVariable>& params, CompilerClass* compilerClass = nullptr);

	~CompilerFunction();

	void SetStartEndLine(uint32 startLine, uint32 endLine);

	Share<CompilerValue> GetLocalVariable(const HString& VariableName);

	const HString& GetName() const { return m_Name; }

	const HazeDefineType& GetFunctionType() const { return m_Type; }

	CompilerModule* GetModule() const { return m_Module; }

	CompilerClass* GetClass() const { return m_OwnerClass; }

	Share<CompilerBlock> GetEntryBlock() { return m_EntryBlock; }

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

	bool FindLocalVariableName(const CompilerValue* value, HString& outName, bool getOffset = false, V_Array<Pair<uint64, CompilerValue*>>* = nullptr);

	bool HasExceptThisParam() const;

	void AddLocalVariable(Share<CompilerValue> value, int line);

	const HazeDefineType& GetParamTypeByIndex(uint64 index);

	const HazeDefineType& GetParamTypeLeftToRightByIndex(uint64 index);

	const HazeDefineType& GetThisParam();

	Share<CompilerClassValue> GetThisLocalVariable();

private:
	void AddFunctionParam(const HazeDefineVariable& variable);

	Share<CompilerValue> CreateLocalVariable(const HazeDefineVariable& variable, int line, Share<CompilerValue> refValue = nullptr, uint64 arrayDimension = 0,
		V_Array<HazeDefineType>* params = nullptr);

	Share<CompilerValue> CreateNew(const HazeDefineType& data, V_Array<Share<CompilerValue>>* countValue);

	void InitEntryBlock(Share<CompilerBlock> block) { m_EntryBlock = block; }

private:
	struct TempRegisterCountData
	{
		Share<CompilerValue> Value = nullptr;
		int Offset = 0;
		bool HasClear = false;
	};

	//ͬһ���ͣ����ǲ������õ���ʱ�Ĵ��������ظ�ʹ��, ͳһ��С��Ϊ8���ֽ�
	Share<CompilerValue> CreateTempRegister(const HazeDefineType& type, uint64 arrayDimension = 0);

	void TryClearTempRegister();

private:
	CompilerModule* m_Module;
	CompilerClass* m_OwnerClass;

	HString m_Name;
	HazeDefineType m_Type;

	V_Array<Pair<HString, Share<CompilerValue>>> m_Params;	//���ҵ���������

	V_Array<Pair<Share<CompilerValue>, int>> m_LocalVariables;
	V_Array<TempRegisterCountData> m_TempRegisters;		//{ ��ʱ�Ĵ���, ���ƫ�Ƹ��� } ���ƫ���� ���� * 8

	Share<CompilerBlock> m_EntryBlock;

	int m_CurrBlockCount;
	int m_CurrVariableCount;
	
	uint32 m_StartLine;
	uint32 m_EndLine;
};