#pragma once

#include "Haze.h"

HAZE_STRING GetModuleFilePath(const HAZE_STRING& ModuleName);

HAZE_STRING GetMainBinaryFilePath();

HAZE_STRING GetIntermediateModuleFile(const HAZE_STRING& ModuleName);