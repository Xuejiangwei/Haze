#pragma once

#include "HazeHeader.h"

class HazeCompilerValue;
class HazeCompilerEnumValue;
class HazeCompilerModule;

// 默认继承整数类型，枚举类型只在解析是判断，真正类型及字节大小是继承类型
class HazeCompilerEnum
{
public:
	HazeCompilerEnum(HazeCompilerModule* compilerModule, const HString& name, HazeValueType parentType);

	~HazeCompilerEnum();

	const HString& GetName() const { return m_Name; }

	void AddEnumValue(const HString& name, Share<HazeCompilerValue> & value);

	Share<HazeCompilerEnumValue> GetEnumValue(const HString& name);

	Share<HazeCompilerEnumValue> GetEnumValueByIndex(uint64 index);

	const HazeValueType GetParentType() const { return m_ParentType; }

private:
	HString m_Name;

	HazeValueType m_ParentType;
	HazeCompilerModule* m_Module;

	V_Array<Pair<HString, Share<HazeCompilerEnumValue>>> m_EnumValues;
};
