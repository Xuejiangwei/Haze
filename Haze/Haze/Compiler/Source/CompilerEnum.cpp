#include "HazePch.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "CompilerModule.h"
#include "CompilerHelper.h"

CompilerEnum::CompilerEnum(CompilerModule* compilerModule, const HString& name, HazeValueType parentType)
	: m_Module(compilerModule), m_Name(name), m_ParentType(parentType)
{
}

CompilerEnum::~CompilerEnum()
{
}

void CompilerEnum::AddEnumValue(const HString& name, Share<CompilerValue>& value)
{
	if (!GetEnumValue(name))
	{
		auto v = MakeShare<CompilerEnumValue>(this, value);
		m_EnumValues.push_back({ name, v });
	}
	else
	{
		COMPILER_ERR_W("添加重复枚举<%s>", name.c_str());
	}
}

Share<CompilerEnumValue> CompilerEnum::GetEnumValue(const HString& name)
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

Share<CompilerEnumValue> CompilerEnum::GetEnumValueByIndex(x_uint64 index)
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

void CompilerEnum::GenEnum_I_Code(HAZE_STRING_STREAM& hss)
{
	hss << GetEnumStartHeader() << HAZE_ENDL;
	for (auto& it : m_EnumValues)
	{
		hss << it.first << " ";
		HazeCompilerStream(hss, it.second);
		hss << HAZE_ENDL;
	}
	hss << GetEnumEndHeader() << HAZE_ENDL_D;
}
