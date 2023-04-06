#pragma once

#include "Haze.h"

class HazeCompilerModule;
class HazeCompilerValue;
class HazeCompilerFunction;

extern HAZE_STRING GetHazeClassFunctionName(const HAZE_STRING& ClassName, const HAZE_STRING& FunctionName);

extern void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value);

extern void HazeCompilerOFStream(HAZE_OFSTREAM& OFStream, HazeCompilerValue* Value);

extern std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, HazeDataDesc Scope);

extern std::shared_ptr<HazeCompilerValue> CreateVariable(const HazeValue& Var, HazeDataDesc Scope);

extern void StreamCompilerValue(HAZE_STRING_STREAM& HSS, InstructionOpCode InsCode, std::shared_ptr<HazeCompilerValue> Value, const HAZE_CHAR* DefaultName = nullptr);

extern void StreamDefineVariable(HAZE_STRING_STREAM& HSS, const HazeDefineVariable& DefineVariable);

extern HAZE_STRING GetObjectName(const HAZE_STRING& InName);

extern std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* Module, const HAZE_STRING& InName);

extern std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* Module, const HAZE_STRING& InName, bool& IsPointer);

extern std::shared_ptr<HazeCompilerValue> GetObjectNameAndMemberName(HazeCompilerModule* Module, const HAZE_STRING& InName, HAZE_STRING& OutObjectName, HAZE_STRING& OutMemberName, bool& IsPointer); 

extern std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* Module, const HAZE_STRING& InName);

extern std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* Module, const HAZE_STRING& InName, bool& IsPointer);

extern std::shared_ptr<HazeCompilerFunction> GetObjectNameAndFunctionName(HazeCompilerModule* Module, const HAZE_STRING& InName, HAZE_STRING& OutObjectName, HAZE_STRING& OutFunctionName, bool& IsPointer);