#pragma once

#include "HazeHeader.h"

class CompilerModule;
class CompilerBlock;
class CompilerValue;
class CompilerFunction;
class CompilerClass;

HString GetLocalVariableName(const HString& name, Share<CompilerValue> value);

void HazeCompilerStream(HAZE_STRING_STREAM& hss, CompilerValue* value, bool streamValue = true);

void HazeCompilerStream(HAZE_STRING_STREAM& hss, Share<CompilerValue> value, bool streamValue = true);

Share<CompilerValue> CreateVariable(CompilerModule* compilerModule, const HazeDefineType& type,
	HazeVariableScope scope, HazeDataDesc desc, int count, Share<CompilerValue> refValue = nullptr,
	uint64 arrayDimension = 0, V_Array<HazeDefineType>* params = nullptr);

Share<CompilerValue> CreateVariableCopyVar(CompilerModule* compilerModule, HazeVariableScope scope, Share<CompilerValue> var);

bool TrtGetVariableName(CompilerFunction* function, const Pair<HString, Share<CompilerValue>>& data,
	const CompilerValue* value, HString& outName, bool getOffset = false, V_Array<Pair<uint64, CompilerValue*>>* = nullptr);

uint32 GetSizeByCompilerValue(Share<CompilerValue> v);

void GetTemplateClassName(HString& inName, const V_Array<TemplateDefineType>& templateTypes);

void GenVariableHzic(CompilerModule* compilerModule, HAZE_STRING_STREAM& hss, const Share<CompilerValue>& value);

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, Share<CompilerValue> assignTo, 
	Share<CompilerValue> oper1, Share<CompilerValue> oper2 = nullptr, const HazeDefineType* expectType = nullptr, bool check = true);

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, Share<CompilerBlock> block1,
	Share<CompilerBlock> block2 = nullptr);

HString GenIRCode(InstructionOpCode opCode, uint64 number);

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, uint64 paramCount, uint64 paramSize,
	Share<CompilerFunction> function, Share<CompilerValue> pointerFunction = nullptr, Share<CompilerValue> advancePointerTo = nullptr, 
	void* advanceFuncAddress = nullptr, const HString* nameSpace = nullptr);