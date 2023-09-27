#pragma once

#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerModule;

class HazeCompilerEnum
{
public:
	HazeCompilerEnum(HazeCompilerModule* compilerModule, HazeValueType parentType);

	~HazeCompilerEnum();

	void InitEnumValues(std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& values);

	std::shared_ptr<HazeCompilerValue> GetEnumValue(const HAZE_STRING& name);

private:
	HazeValueType m_ParentType;
	HazeCompilerModule* m_Module;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> m_EnumValues;
};
