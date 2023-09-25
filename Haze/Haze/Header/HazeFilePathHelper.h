#pragma once

#include "Haze.h"

HAZE_STRING GetModuleFilePath(const HAZE_STRING& m_ModuleName);

HAZE_STRING GetMainBinaryFilePath();

HAZE_STRING GetIntermediateModuleFile(const HAZE_STRING& m_ModuleName);