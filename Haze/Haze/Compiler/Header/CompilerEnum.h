#pragma once

#include "HazeHeader.h"

class CompilerValue;
class CompilerEnumValue;
class CompilerModule;

// 默认继承整数类型，枚举类型只在解析是判断，真正类型及字节大小是继承类型
class CompilerEnum
{
public:
	CompilerEnum(CompilerModule* compilerModule, const HString& name, x_uint32 typeId);

	~CompilerEnum();

	const HString& GetName() const { return m_Name; }

	void AddEnumValue(const HString& name, Share<CompilerValue> value);

	Share<CompilerEnumValue> GetEnumValue(const HString& name);

	Share<CompilerEnumValue> GetEnumValueByIndex(x_uint64 index);

	const x_uint32 GetTypeId() const { return m_Type.TypeId; }

	void GenEnum_I_Code(HAZE_STRING_STREAM& hss);

private:
	HString m_Name;

	HazeVariableType m_Type;
	CompilerModule* m_Module;

	V_Array<Pair<HString, Share<CompilerEnumValue>>> m_EnumValues;
};
