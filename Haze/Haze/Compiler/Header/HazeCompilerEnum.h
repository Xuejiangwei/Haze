#pragma once

#include "HazeHeader.h"

class HazeCompilerValue;
class HazeCompilerModule;

class HazeCompilerEnum
{
public:
	HazeCompilerEnum(HazeCompilerModule* compilerModule, HazeValueType parentType);

	~HazeCompilerEnum();

	void AddEnumValue(const HString& name, Share<HazeCompilerValue> & value);

	Share<HazeCompilerValue> GetEnumValue(const HString& name);

private:
	HazeValueType m_ParentType;
	HazeCompilerModule* m_Module;

	V_Array<Pair<HString, Share<HazeCompilerValue>>> m_EnumValues;
};
