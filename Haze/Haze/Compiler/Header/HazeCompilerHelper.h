#pragma once

#include "HazeHeader.h"

class HazeCompilerModule;
class HazeCompilerValue;
class HazeCompilerFunction;
class HazeCompilerClass;

HAZE_STRING GetHazeClassFunctionName(const HAZE_STRING& className, const HAZE_STRING& functionName);

HAZE_STRING GetLocalVariableName(const HAZE_STRING& name, std::shared_ptr<HazeCompilerValue> value);

void HazeCompilerStream(HAZE_STRING_STREAM& hss, HazeCompilerValue* value, bool streamValue = true);

void HazeCompilerStream(HAZE_STRING_STREAM& hss, std::shared_ptr<HazeCompilerValue> value, bool streamValue = true);

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* compilerModule, const HazeDefineVariable& var, 
	HazeVariableScope scope, HazeDataDesc desc, int count, std::shared_ptr<HazeCompilerValue> refValue = nullptr,
	std::vector<std::shared_ptr<HazeCompilerValue>> arraySize = {}, std::vector<HazeDefineType>* params = nullptr);

std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> CreateVariableCopyClassMember(
	HazeCompilerModule* compilerModule, HazeVariableScope scope, HazeCompilerClass* compilerClass);

void StreamCompilerValue(HAZE_STRING_STREAM& hss, InstructionOpCode insCode, std::shared_ptr<HazeCompilerValue> value, 
	const HAZE_CHAR* defaultName = nullptr);

HAZE_STRING GetObjectName(const HAZE_STRING& inName);

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HAZE_STRING& inName);

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HAZE_STRING& inName, bool& isPointer);

std::shared_ptr<HazeCompilerValue> GetObjectNameAndMemberName(HazeCompilerModule* compilerModule, const HAZE_STRING& inName,
	HAZE_STRING& outObjectName, HAZE_STRING& outMemberName, bool& isPointer);

std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* compilerModule, const HAZE_STRING& inName);

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> GetObjectFunction(
	HazeCompilerModule* compilerModule, const HAZE_STRING& inName, bool& isPointer);

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> GetObjectNameAndFunctionName(
	HazeCompilerModule* compilerModule, const HAZE_STRING& inName, HAZE_STRING& outObjectName, HAZE_STRING& outFunctionName, bool& isPointer);

bool TrtGetVariableName(HazeCompilerFunction* function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& data,
	const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName);

bool TrtGetVariableName(HazeCompilerFunction* function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& data,
	const HazeCompilerValue* value, HAZE_STRING& outName);

std::shared_ptr<HazeCompilerValue> GetArrayElementToValue(HazeCompilerModule* compilerModule,
	std::shared_ptr<HazeCompilerValue> elementValue, std::shared_ptr<HazeCompilerValue> movToValue = nullptr);

void GetTemplateClassName(HAZE_STRING& inName, const std::vector<HazeDefineType>& templateTypes);
