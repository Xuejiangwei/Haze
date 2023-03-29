#pragma once

#include "Haze.h"

class HazeCompilerModule;
class HazeCompilerValue;

extern void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value);

extern void HazeCompilerOFStream(HAZE_OFSTREAM& OFStream, HazeCompilerValue* Value);

extern std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, InstructionScopeType Scope);

extern std::shared_ptr<HazeCompilerValue> CreateVariable(const HazeValue& Var, InstructionScopeType Scope);

extern void StreamCompilerValue(HAZE_STRING_STREAM& HSS, InstructionOpCode InsCode, std::shared_ptr<HazeCompilerValue> Value, const HAZE_CHAR* DefaultName = nullptr);

extern void StreamDefineVariable(HAZE_STRING_STREAM& HSS, const HazeDefineVariable& DefineVariable);