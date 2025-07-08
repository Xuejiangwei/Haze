#pragma once

struct HazeComplexTypeInfoBase
{
	HazeValueType BaseType;
};

struct HazeComplexTypeInfo
{
	struct BaseTemplate : public HazeComplexTypeInfoBase
	{
		x_uint32 TypeId1;
	};

	typedef BaseTemplate BaseObject;

	struct Class : public HazeComplexTypeInfoBase
	{
		char NameAddress[8];
		//x_HChar* ClassName;

		void SetName(const HString* name)
		{
			memcpy(NameAddress, &name, sizeof(name));
		}

		const HString* GetString() const
		{
			HString* str;
			memcpy(&str, NameAddress, sizeof(NameAddress));
			return str;
		}
	};

	// 函数返回类型存储在TypeId1种
	struct Function : public BaseTemplate
	{
		x_uint32 FunctionInfoIndex;
	};

	struct Array : public BaseTemplate
	{
		x_uint32 Dimension;
	};

	struct Hash : public BaseTemplate
	{
		x_uint32 TypeId2;
	};

	union
	{
		HazeComplexTypeInfoBase _BaseType;
		BaseObject _BaseObject;
		Class _Class;
		Array _Array;
		Hash _Hash;
		Function _Function;
	};

	HazeValueType GetBaseType() const { return _BaseType.BaseType; }
};

class HazeTypeInfoMap
{
	struct TypeInfo
	{
		x_uint32 TypeId;
		HazeComplexTypeInfo Info;
	};
public:
	HazeTypeInfoMap();

	~HazeTypeInfoMap();

	x_uint32 RegisterType(HazeComplexTypeInfoBase* type);
	x_uint32 RegisterType(x_uint32 functionTypeId, V_Array<x_uint32>&& paramTypeId);

	const HString* GetClassName(x_uint32 typeId);

	const HazeComplexTypeInfo* GetTypeInfoById(x_uint32 typeId);

private:
	x_uint32 RegisterFunctionParamListType(x_uint32 typeId, V_Array<x_uint32>& paramList);

private:
	V_Array<HazeComplexTypeInfoBase> m_BaseTypeMap;
	HashMap<HString, TypeInfo> m_Map;
	HashMap<x_uint32, const x_HChar*> m_Mapping;
	HashMap<x_int32, V_Array<x_uint32>> m_FunctionInfoMap;
};


