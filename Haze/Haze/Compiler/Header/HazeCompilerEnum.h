#pragma once

#include "HazeHeader.h"

class HazeCompilerValue;
class HazeCompilerEnumValue;
class HazeCompilerModule;

// Ĭ�ϼ̳��������ͣ�ö������ֻ�ڽ������жϣ��������ͼ��ֽڴ�С�Ǽ̳�����
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
