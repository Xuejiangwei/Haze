#include "HazePch.h"
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

void HazeCompilerEnum::AddEnumValue(const HString& name, Share<HazeCompilerValue>& value)
{
	if (!GetEnumValue(name))
	{
		m_EnumValues.push_back({ name, value });
	}
	else
	{
		COMPILER_ERR_W("ÃÌº”÷ÿ∏¥√∂æŸ<%s>", name.c_str());
	}
}

Share<HazeCompilerValue> HazeCompilerEnum::GetEnumValue(const HString& name)
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
