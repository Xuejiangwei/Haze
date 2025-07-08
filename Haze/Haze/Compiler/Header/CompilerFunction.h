#pragma once

#include "CompilerValue.h"

class CompilerModule;
class CompilerClass;
class CompilerClassValue;
class CompilerBlock;

enum class ClassCompilerFunctionType : x_uint8
{
	None,
	Normal,
	Virtual,
	PureVirtual
};

class CompilerFunction
{
	friend class Compiler;
	friend class CompilerModule;
	friend class ASTMultiExpression;

public:
	CompilerFunction(CompilerModule* compilerModule, const HString& name, HazeVariableType& type,
		V_Array<HazeDefineVariable>& params, CompilerClass* compilerClass = nullptr,
		ClassCompilerFunctionType classFunctionType = ClassCompilerFunctionType::None);

	virtual ~CompilerFunction();

	void SetStartEndLine(x_uint32 startLine, x_uint32 endLine);

	Share<CompilerValue> GetLocalVariable(const HString& VariableName, HString* nameSpace);

	const HString& GetName() const { return m_Name; }

	HString GetRealName() const;

	const HazeVariableType& GetFunctionType() const { return m_Type; }

	CompilerModule* GetModule() const { return m_Module; }

	CompilerClass* GetClass() const { return m_OwnerClass; }

	Share<CompilerBlock> GetEntryBlock() { return m_EntryBlock; }

	static void ParseIntermediate(HAZE_IFSTREAM& stream, CompilerModule* m);

	void GenI_Code(HAZE_STRING_STREAM& hss);

	HString GenDafaultBlockName();

	HString GenIfThenBlockName();

	HString GenElseBlockName();

	HString GenLoopBlockName();

	HString GenWhileBlockName();

	HString GenForBlockName();

	HString GenForConditionBlockName();

	HString GenForStepBlockName();

	bool FindLocalVariableName(const Share<CompilerValue> value, HString& outName);

	int FindLocalVariableIndex(const Share<CompilerValue> value);

	bool HasExceptThisParam() const;

	void AddLocalVariable(Share<CompilerValue> value, int line);

	const HazeVariableType& GetParamTypeByIndex(x_uint64 index);

	const HazeVariableType& GetParamTypeLeftToRightByIndex(x_uint64 index);

	const x_uint64 GetParamSize() const;

	Share<CompilerClassValue> GetThisLocalVariable();

	bool IsVirtualFunction() const { return m_ClassFunctionType == ClassCompilerFunctionType::PureVirtual ||
			m_ClassFunctionType == ClassCompilerFunctionType::Virtual; }

	x_uint32 GetFunctionPointerTypeId();

protected:
	void FunctionFinish();

	void AddFunctionParam(const HazeDefineVariable& variable);

	Share<CompilerValue> CreateGlobalVariable(const HazeDefineVariable& variable, int line, Share<CompilerValue> refValue = nullptr, x_uint64 arrayDimension = 0,
		TemplateDefineTypes* params = nullptr);

	Share<CompilerValue> CreateLocalVariable(const HazeDefineVariable& variable, int line, Share<CompilerValue> refValue = nullptr);

	//Share<CompilerValue> CreateNew(const HazeDefineType& data, V_Array<Share<CompilerValue>>* countValue);

	void InitEntryBlock(Share<CompilerBlock> block) { m_EntryBlock = block; }

protected:
	struct TempRegisterCountData
	{
		Share<CompilerValue> Value = nullptr;
		int Offset = 0;
		bool HasClear = false;
	};

	// 同一类型，但是不再引用的临时寄存器可以重复使用, 统一大小都为8个字节
	Share<CompilerValue> CreateTempRegister(const HazeVariableType& type);

	void TryClearTempRegister();

protected:
	CompilerModule* m_Module;
	CompilerClass* m_OwnerClass;

	HString m_Name;
	HazeVariableType m_Type;

	V_Array<Pair<HString, Share<CompilerValue>>> m_Params;	//从右到左加入参数

	V_Array<Pair<Share<CompilerValue>, int>> m_LocalVariables; // { 变量, 定义所在的行数 }
	V_Array<TempRegisterCountData> m_TempRegisters;		//{ 临时寄存器, 相对偏移个数 } 相对偏移是 个数 * 8

	Share<CompilerBlock> m_EntryBlock;

	int m_CurrBlockCount;
	int m_CurrVariableCount;
	
	x_uint32 m_StartLine;
	x_uint32 m_EndLine;
	InstructionFunctionType m_DescType;

	ClassCompilerFunctionType m_ClassFunctionType;
};
