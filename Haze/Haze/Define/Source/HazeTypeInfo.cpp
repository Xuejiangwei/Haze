#include "HazePch.h"
#include "HazeTypeInfo.h"

// type id需要与枚举值相等
//HashMap<HazeValueType, x_uint32> g_HazeTypeIdMap =
//{
//#define HAZE_TYPE_DEFINE(TYPE) { HazeValueType::TYPE, (x_uint32)HazeValueType::TYPE },
//	#include "HazeValueTypeTemplate"
//#undef HAZE_TYPE_DEFINE
//};

#define COMPLEX_TYPE_START 100
static x_uint32 startIndex = COMPLEX_TYPE_START;

HazeTypeInfoMap::HazeTypeInfoMap()
{
	m_BaseTypeMap =
	{
		#define HAZE_TYPE_DEFINE(TYPE) { HazeValueType::TYPE },
			#include "HazeValueTypeTemplate"
		#undef HAZE_TYPE_DEFINE
	};

	m_Mapping =
	{
		#define HAZE_TYPE_DEFINE(TYPE) { (x_uint32)HazeValueType::TYPE, H_TEXT(#TYPE) },
			#include "HazeValueTypeTemplate"
		#undef HAZE_TYPE_DEFINE
	};
}

HazeTypeInfoMap::~HazeTypeInfoMap()
{
}

x_uint32 HazeTypeInfoMap::RegisterType(HazeComplexTypeInfoBase* type)
{
	HString str;
	HazeComplexTypeInfo info;
	info._BaseType.BaseType = type->BaseType;

	switch (type->BaseType)
	{
		case HazeValueType::Array:
		{
			auto complexType = (HazeComplexTypeInfo::Array*)type;
			str += H_TEXT("Array<");
			str += m_Mapping[complexType->TypeId1];
			str += H_TEXT(">") + ToHazeString(complexType->Dimension);

			info._Array = *complexType;
		}
		break;
		case HazeValueType::Class:
		{
			auto complexType = (HazeComplexTypeInfo::Class*)type;
			str = *complexType->GetString();
			
			info._Class = *complexType;
		}
		break;
		case HazeValueType::ObjectBase:
		{
			auto complexType = (HazeComplexTypeInfo::BaseObject*)type;
			str += H_TEXT("ObjectBase<");
			str += m_Mapping[complexType->TypeId1];
			str += H_TEXT(">");

			info._BaseObject = *complexType;
		}
		break;
		case HazeValueType::Hash:
		{
			auto complexType = (HazeComplexTypeInfo::Hash*)type;
			str += H_TEXT("Hash<");
			str += m_Mapping[complexType->TypeId1];
			str += H_TEXT(",");
			str += m_Mapping[complexType->TypeId2];
			str += H_TEXT(">");
		
			info._Hash = *complexType;
		}
		break;
		default:
			break;
	}

	auto iter = m_Map.find(str);
	if (iter != m_Map.end())
	{
		return iter->second.TypeId;
	}

	assert(!str.empty());

	m_Mapping[++startIndex] = str.c_str();
	m_Map[Move(str)] = { startIndex, info };
	return startIndex;
}

x_uint32 HazeTypeInfoMap::RegisterType(x_uint32 functionTypeId, V_Array<x_uint32>&& paramTypeId)
{
	return RegisterFunctionParamListType(functionTypeId, paramTypeId);
}

const HString* HazeTypeInfoMap::GetClassName(x_uint32 typeId)
{
	auto info = (HazeComplexTypeInfo::Class*)GetTypeInfoById(typeId);
	return info->GetString();
}

const HazeComplexTypeInfo* HazeTypeInfoMap::GetTypeInfoById(x_uint32 typeId)
{
	if (typeId > COMPLEX_TYPE_START)
	{
		return &m_Map[HString(m_Mapping[typeId])].Info;
	}
	else
	{
		return (HazeComplexTypeInfo*)(&m_BaseTypeMap[typeId]);
	}
}

x_uint32 HazeTypeInfoMap::RegisterFunctionParamListType(x_uint32 typeId, V_Array<x_uint32>& paramList)
{
	HString str(H_TEXT("FuncParam<"));
	str += m_Mapping[typeId];
	str += H_TEXT(">(");

	for (x_uint64 i = 0; i < paramList.size(); i++)
	{
		str += m_Mapping[paramList[i]];
		str += H_TEXT(",");
	}

	if (paramList.size() > 0)
	{
		str.resize(str.length() - 1);
	}
	str += H_TEXT(")");

	auto iter = m_Map.find(str);
	if (iter != m_Map.end())
	{
		return iter->second.TypeId;
	}

	assert(!str.empty());

	m_Mapping[++startIndex] = str.c_str();

	HazeComplexTypeInfo info;
	info._Function.BaseType = HazeValueType::Function;
	info._Function.FunctionInfoIndex = startIndex;

	m_Map[Move(str)] = { startIndex, info };

	m_FunctionInfoMap[startIndex] = Move(paramList);
	return startIndex;
}