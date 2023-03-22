#pragma once

#include <sstream>
#include "Haze.h"

class HazeCompilerModule;
class HazeCompilerValue;

extern void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value);

extern void HazeCompilerOFStream(HAZE_OFSTREAM& OFStream, HazeCompilerValue* Value);

extern std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* Module, const HazeDefineVariable& Var, InstructionScopeType Scope, std::shared_ptr<HazeCompilerValue> Parent = nullptr);

extern std::shared_ptr<HazeCompilerValue> CreateVariable(const HazeValue& Var, InstructionScopeType Scope);