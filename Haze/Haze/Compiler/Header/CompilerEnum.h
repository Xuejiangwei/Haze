#pragma once

#include "HazeHeader.h"

class CompilerValue;
class CompilerEnumValue;
class CompilerModule;

// Ĭ�ϼ̳��������ͣ�ö������ֻ�ڽ������жϣ��������ͼ��ֽڴ�С�Ǽ̳�����
class CompilerEnum
{
public:
	CompilerEnum(CompilerModule* compilerModule, const HString& name, HazeValueType parentType);

	~CompilerEnum();

	const HString& GetName() const { return m_Name; }

	void AddEnumValue(const HString& name, Share<CompilerValue> & value);

	Share<CompilerEnumValue> GetEnumValue(const HString& name);

	Share<CompilerEnumValue> GetEnumValueByIndex(x_uint64 index);

	const HazeValueType GetParentType() const { return m_ParentType; }

private:
	HString m_Name;

	HazeValueType m_ParentType;
	CompilerModule* m_Module;

	V_Array<Pair<HString, Share<CompilerEnumValue>>> m_EnumValues;
};
