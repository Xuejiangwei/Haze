#include "HazePch.h"
#include "HazeCompilerEnum.h"
#include "HazeCompilerEnumValue.h"
#include "HazeCompilerModule.h"

HazeCompilerEnum::HazeCompilerEnum(HazeCompilerModule* compilerModule, const HString& name, HazeValueType parentType)
	: m_Module(compilerModule), m_Name(name), m_ParentType(parentType)
{
}

HazeCompilerEnum::~HazeCompilerEnum()
{
}

void HazeCompilerEnum::AddEnumValue(const HString& name, Share<HazeCompilerValue>& value)
{
	if (!GetEnumValue(name))
	{
		auto v = MakeShare<HazeCompilerEnumValue>(this, value);
		auto& type = const_cast<HazeDefineType&>(v->GetValueType());
		
		m_EnumValues.push_back({ name, v });
	}
	else
	{
		COMPILER_ERR_W("添加重复枚举<%s>", name.c_str());
	}
}

Share<HazeCompilerEnumValue> HazeCompilerEnum::GetEnumValue(const HString& name)
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

Share<HazeCompilerEnumValue> HazeCompilerEnum::GetEnumValueByIndex(uint64 index)
{
	if (index < m_EnumValues.size())
	{
		return m_EnumValues[index].second;
	}
	else
	{
		COMPILER_ERR_W("枚举没有<%d>个成员数", index + 1);
	}

	return nullptr;
}
