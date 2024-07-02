#pragma once

#include "HazeHeader.h"

HAZE_STRING GetModuleFilePath(const HAZE_STRING& moduleName, const HAZE_STRING* refModulePath = nullptr, const HAZE_STRING* dir = nullptr);

HAZE_STRING GetMainBinaryFilePath();

HAZE_STRING GetIntermediateModuleFile(const HAZE_STRING& moduleName);