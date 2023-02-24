#pragma once

#include <sstream>
#include "Haze.h"

class HazeCompilerModule;
class HazeCompilerValue;

extern void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value);

extern void HazeCompilerOFStream(HAZE_OFSTREAM& OFStream, HazeCompilerValue* Value);

extern void PushICode(HazeCompilerModule* Module, HazeCompilerValue* Value);

extern void StreamAssCode(HazeCompilerModule* Module, HazeCompilerValue* Value);