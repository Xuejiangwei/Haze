#pragma once

#include "Haze.h"

class HazeCompilerModule;
class HazeCompilerValue;
class HazeCompilerFunction;
class HazeCompilerClass;

HAZE_STRING GetHazeClassFunctionName(const HAZE_STRING& ClassName, const HAZE_STRING& FunctionName);

HAZE_STRING GetLocalVariableName(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerValue> Value);

void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value, bool StreamValue = true);

void HazeCompilerStream(HAZE_STRING_STREAM& Stream, std::shared_ptr<HazeCompilerValue> Value, bool StreamValue = true);

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
	std::shared_ptr<HazeCompilerValue> RefValue = nullptr, std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize = {}, std::vector<HazeDefineType>* Vector_Param = nullptr);

std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> CreateVariableCopyClassMember(HazeCompilerModule* Module, HazeVariableScope Scope, HazeCompilerClass* Class);

void StreamCompilerValue(HAZE_STRING_STREAM& HSS, InstructionOpCode InsCode, std::shared_ptr<HazeCompilerValue> Value, const HAZE_CHAR* DefaultName = nullptr);
HAZE_STRING GetObjectName(const HAZE_STRING& InName);

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* Module, const HAZE_STRING& InName);

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* Module, const HAZE_STRING& InName, bool& IsPointer);

std::shared_ptr<HazeCompilerValue> GetObjectNameAndMemberName(HazeCompilerModule* Module, const HAZE_STRING& InName, HAZE_STRING& OutObjectName, HAZE_STRING& OutMemberName, bool& IsPointer);

std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* Module, const HAZE_STRING& InName);

std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* Module, const HAZE_STRING& InName, bool& IsPointer);

std::shared_ptr<HazeCompilerFunction> GetObjectNameAndFunctionName(HazeCompilerModule* Module, const HAZE_STRING& InName, HAZE_STRING& OutObjectName, HAZE_STRING& OutFunctionName, bool& IsPointer);

bool TrtGetVariableName(HazeCompilerFunction* Function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& Data, const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

bool TrtGetVariableName(HazeCompilerFunction* Function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& Data, const HazeCompilerValue* Value, HAZE_STRING& OutName);

std::shared_ptr<HazeCompilerValue> GetArrayElementToValue(HazeCompilerModule* Module, std::shared_ptr<HazeCompilerValue> ElementValue, std::shared_ptr<HazeCompilerValue> MovToValue = nullptr);
