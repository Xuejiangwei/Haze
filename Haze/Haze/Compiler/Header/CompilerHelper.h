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

Share<CompilerValue> CreateAstTempVariable(CompilerModule* compilerModule, const HazeVariableType& type);

Share<CompilerValue> CreateVariable(CompilerModule* compilerModule, const HazeVariableType& type,
	HazeVariableScope scope, HazeDataDesc desc, int count, Share<CompilerValue> refValue = nullptr,
	TemplateDefineTypes* params = nullptr);

Share<CompilerValue> CreateVariableCopyVar(CompilerModule* compilerModule, HazeVariableScope scope, Share<CompilerValue> var);

bool TrtGetVariableName(const Pair<HString, Share<CompilerValue>>& data, const CompilerValue* value, HString& outName);

void GetTemplateClassName(HString& inName, const V_Array<TemplateDefineType>& templateTypes);

void GenVariableHzic(CompilerModule* compilerModule, HAZE_STRING_STREAM& hss, const Share<CompilerValue>& value);

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, Share<CompilerValue> assignTo, 
	Share<CompilerValue> oper1, Share<CompilerValue> oper2 = nullptr, const HazeVariableType* expectType = nullptr, bool check = true);

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, Share<CompilerBlock> block1,
	Share<CompilerBlock> block2 = nullptr);

HString GenIRCode(InstructionOpCode opCode, x_uint64 number);


void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, x_uint64 paramCount, x_uint64 paramSize,
	Share<CompilerFunction> function, Share<CompilerValue> pointerFunction = nullptr, Share<CompilerValue> advancePointerTo = nullptr, 
	x_int16 advanceFuncIndex = -1, const HString* nameSpace = nullptr);

//void GenIRCode_NewSign(HAZE_STRING_STREAM& hss, TemplateDefineTypes* defineTypes);