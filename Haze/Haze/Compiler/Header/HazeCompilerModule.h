#pragma once

#include <unordered_map>
#include <fstream>

#include "Haze.h"

class HazeCompiler;
class HazeCompilerValue;
class HazeBaseBlock;
class HazeCompilerFunction;
class HazeCompilerClass;

class HazeCompilerModule
{
public:
	friend class HazeCompiler;

	HazeCompilerModule(HazeCompiler* Compiler, const HAZE_STRING& ModuleName);
	~HazeCompilerModule();

	void MarkStandardLibrary();
	
	void GenCodeFile();

	std::shared_ptr<HazeCompilerClass> CreateClass(const HAZE_STRING& Name, std::vector<std::pair<HazeDataDesc, 
		std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& ClassData);

	void FinishCreateClass();

	std::shared_ptr<HazeCompilerFunction> GetCurrFunction();

	std::shared_ptr<HazeCompilerFunction> CreateFunction(const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);

	std::shared_ptr<HazeCompilerFunction> CreateFunction(std::shared_ptr<HazeCompilerClass> Class, const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);

	void FinishFunction();

	std::shared_ptr<HazeCompilerValue> GetGlobalStringVariable(const HAZE_STRING& String);

	uint32 GetGlobalStringIndex(std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> GetGlobalVariable(const HAZE_STRING& Name);

	bool GetGlobalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	bool GetGlobalVariableName(const HazeCompilerValue* Value, HAZE_STRING& OutName);

public:
	std::shared_ptr<HazeCompilerClass> FindClass(const HAZE_STRING& ClassName);
	
	uint32 GetClassSize(const HAZE_STRING& ClassName);

	std::shared_ptr<HazeCompilerFunction> GetFunction(const HAZE_STRING& Name);

private:
	std::shared_ptr<HazeCompilerValue> CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);
	
	std::shared_ptr<HazeCompilerValue> CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);
	
	std::shared_ptr<HazeCompilerValue> CreateMod(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateBitNeg(std::shared_ptr<HazeCompilerValue> Value);
	
	std::shared_ptr<HazeCompilerValue> CreateBitXor(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShl(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateShr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign = false);

	std::shared_ptr<HazeCompilerValue> CreateNot(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right);

	std::shared_ptr<HazeCompilerValue> CreateNeg(std::shared_ptr<HazeCompilerValue> Value);

	std::shared_ptr<HazeCompilerValue> CreateInc(std::shared_ptr<HazeCompilerValue> Value, bool IsPreInc);

	std::shared_ptr<HazeCompilerValue> CreateDec(std::shared_ptr<HazeCompilerValue> Value, bool IsPreDec);

	std::shared_ptr<HazeCompilerValue> CreateArrayInit(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> InitList);

	std::shared_ptr<HazeCompilerValue> GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, InstructionOpCode IO_Code);

	void GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> Value);

	void GenIRCode_Cmp(HazeCmpType CmpType, std::shared_ptr<HazeBaseBlock> IfJmpBlock, std::shared_ptr<HazeBaseBlock> ElseJmpBlock);

	void GenIRCode_JmpTo(std::shared_ptr<HazeBaseBlock> Block);

	static void GenValueHzicText(HazeCompilerModule* Module, HAZE_STRING_STREAM& HSS, const std::shared_ptr<HazeCompilerValue>& Value, int Index = -1);

public:
	HazeCompiler* GetCompiler() { return Compiler; }

private:
	std::shared_ptr<HazeCompilerValue> CreateGlobalVariable(const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> RefValue = nullptr, 
		std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize = {}, std::vector<HazeDefineType>* Vector_Param = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> CallFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo = nullptr);
	
	std::shared_ptr<HazeCompilerValue> CreateFunctionCall(std::shared_ptr<HazeCompilerValue> PointerFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo = nullptr);

	void FunctionCall(HAZE_STRING_STREAM& SStream, uint32& Size, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo);

	bool IsStandardLibrary() { return IsStdLib; }

private:
	void GenICode();

private:
	HazeCompiler* Compiler;

	bool IsStdLib;

	HAZE_OFSTREAM FS_I_Code;

	HAZE_STRING CurrClass;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerClass>> HashMap_Class;

	HAZE_STRING CurrFunction;
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerFunction>> HashMap_Function;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> Vector_Variable; //这个是Symbol table(符号表)

	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> HashMap_StringTable;
	std::unordered_map<int, const HAZE_STRING*> HashMap_StringMapping;

	std::vector<HazeCompilerModule*> Vector_ImportModule;
};
