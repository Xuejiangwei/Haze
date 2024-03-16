#pragma once

#include "HazeHeader.h"

HAZE_STRING GetModuleFilePath(const HAZE_STRING& moduleName);

HAZE_STRING GetMainBinaryFilePath();

HAZE_STRING GetIntermediateModuleFile(const HAZE_STRING& moduleName);