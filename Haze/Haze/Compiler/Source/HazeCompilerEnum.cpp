#include "HazeCompilerEnum.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerModule.h"

HazeCompilerEnum::HazeCompilerEnum(HazeCompilerModule* compilerModule, HazeValueType parentType)
	: m_Module(compilerModule), m_ParentType(parentType)
{
}

HazeCompilerEnum::~HazeCompilerEnum()
{
}

void HazeCompilerEnum::InitEnumValues(std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& values)
{
	m_EnumValues = std::move(values);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerEnum::GetEnumValue(const HAZE_STRING& name)
{
	for (auto& it : m_EnumValues)
	{
		if (it.first == name)
		{
			return it.second;
		}
	}

	return nullptr;
}
