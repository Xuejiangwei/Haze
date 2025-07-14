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
	/*m_BaseTypeMap =
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
	};*/

	m_Map =
	{
		#define HAZE_TYPE_DEFINE(TYPE) { (x_uint32)HazeValueType::TYPE, { H_TEXT(#TYPE), 0, (x_uint32)HazeValueType::TYPE, HazeComplexTypeInfo(HazeValueType::TYPE) } },
			#include "HazeValueTypeTemplate"
		#undef HAZE_TYPE_DEFINE
	};
}

HazeTypeInfoMap::~HazeTypeInfoMap()
{
}

x_uint32 HazeTypeInfoMap::RegisterType(const HString& moduleName, HazeComplexTypeInfo* info)
{
	HString str;
	switch (info->_BaseType.BaseType)
	{
		case HazeValueType::Array:
		{
			auto complexType = (HazeComplexTypeInfo::Array*)info;
			str += H_TEXT("Array<");
			str += m_Map[complexType->TypeId1].Name;
			str += H_TEXT(">") + ToHazeString(complexType->Dimension);
		}
		break;
		case HazeValueType::Class:
		{
			auto complexType = (HazeComplexTypeInfo::Class*)info;
			str = *complexType->GetString();
		}
		break;
		case HazeValueType::ObjectBase:
		{
			auto complexType = (HazeComplexTypeInfo::BaseObject*)info;
			str += H_TEXT("ObjectBase<");
			str += m_Map[complexType->TypeId1].Name;
			str += H_TEXT(">");
		}
		break;
		case HazeValueType::Hash:
		{
			auto complexType = (HazeComplexTypeInfo::Hash*)info;
			str += H_TEXT("Hash<");
			str += m_Map[complexType->TypeId1].Name;
			str += H_TEXT(",");
			str += m_Map[complexType->TypeId2].Name;
			str += H_TEXT(">");
		}
		break;
		default:
			break;
	}
	
	auto iter = m_NameCache.find(str);
	if (iter != m_NameCache.end())
	{
		if (AddModuleRef(moduleName, iter->second))
		{
			m_Map[iter->second].RefCount++;
		}
		return iter->second;
	}

	assert(!str.empty());

	auto newTypeId = ++startIndex;
	m_Map[newTypeId] = { str, 1, newTypeId, *info };
	m_NameCache[str] = newTypeId;

	AddModuleRef(moduleName, startIndex);

	auto& typeIno = m_Map[newTypeId];
	switch (info->_BaseType.BaseType)
	{
		case HazeValueType::Class:
			typeIno.Info._Class.SetName(&typeIno.Name);
			break;
		default:
			break;
	}

	return startIndex;
}

x_uint32 HazeTypeInfoMap::RegisterType(const HString& moduleName, x_uint32 functionTypeId, V_Array<x_uint32>&& paramTypeId)
{
	return RegisterFunctionParamListType(moduleName, functionTypeId, paramTypeId);
}

const HString* HazeTypeInfoMap::GetClassName(x_uint32 typeId)
{
	auto info = GetTypeInfoById(typeId);
	return &info->Name;
}

const HString* HazeTypeInfoMap::GetEnumName(x_uint32 typeId)
{
	auto info = (HazeComplexTypeInfo::Enum*)GetTypeInfoById(typeId);
	return info->GetString();
}

const HazeTypeInfoMap::TypeInfo* HazeTypeInfoMap::GetTypeInfoById(x_uint32 typeId)
{
	return &m_Map[typeId];
}

const HazeComplexTypeInfo* HazeTypeInfoMap::GetTypeById(x_uint32 typeId)
{
	return &m_Map[typeId].Info;
}

const x_uint32 HazeTypeInfoMap::GetTypeId(const HString& name) const
{
	auto iter = m_NameCache.find(name);
	if (iter != m_NameCache.end())
	{
		return iter->second;
	}

	return 0;
}

void HazeTypeInfoMap::GenICode(HAZE_STRING_STREAM& hss)
{
	hss << TYPE_INFO_FUNC_PARAM_BEGIN << HAZE_ENDL_D;
	
	for (auto& iter : m_FunctionInfoMap)
	{
		hss << iter.first << " " << iter.second.size() << " ";

		for (x_uint64 i = 0; i < iter.second.size(); i++)
		{
			hss << iter.second[i] << " ";
		}
		hss << HAZE_ENDL;
	}
	
	hss << HAZE_ENDL << TYPE_INFO_FUNC_PARAM_END << HAZE_ENDL_D;

	hss << TYPE_INFO_BEGIN << HAZE_ENDL_D;

	for (x_uint64 i = COMPLEX_TYPE_START + 1; i <= startIndex; i++)
	{
		auto& typeInfo = m_Map[i];
		hss << typeInfo.Name << " " << typeInfo.RefCount << " " << typeInfo.TypeId << " " << (x_uint32)typeInfo.Info._BaseType.BaseType << " ";

		auto& info = typeInfo.Info;
		switch (info.GetBaseType())
		{
			case HazeValueType::ObjectBase:
				hss << info._ObjectBase.TypeId1;
				break;
			case HazeValueType::Class:
			case HazeValueType::Enum:
				break;
			case HazeValueType::Array:
				hss << info._Array.TypeId1 << " " << info._Array.Dimension;
				break;
			case HazeValueType::Hash:
				hss << info._Hash.TypeId1 << " " << info._Hash.TypeId2;
				break;
			case HazeValueType::Function:
				hss << info._Function.TypeId1 << " " << info._Function.FunctionInfoIndex;
				break;
			default:
				break;
		}

		hss << HAZE_ENDL;
	}

	hss << HAZE_ENDL << TYPE_INFO_END;

	// 在添加一个换行当作结束符号
	hss << HAZE_ENDL;
}

void HazeTypeInfoMap::GenModuleReferenceTypeInfo(HAZE_STRING_STREAM& hss, const HString& moduleName)
{
	auto iter = m_ModuleRefTypes.find(moduleName);
	if (iter != m_ModuleRefTypes.end())
	{
		for (auto& typeId : iter->second.TypeIds)
		{
			hss << typeId << " ";
		}
	}
}

const x_uint32 HazeTypeInfoMap::GetTypeIdByClassName(const HString& name) const
{
	for (auto& iter : m_Map)
	{
		if (iter.second.Name == name)
		{
			return iter.first;
		}
	}

	return 0;
}

void HazeTypeInfoMap::AddFunctionTypeInfo(x_uint32 typeId, V_Array<x_uint32>& typeAndParams)
{
	m_FunctionInfoMap[typeId] = Move(typeAndParams);
}

void HazeTypeInfoMap::AddTypeInfo(HString&& name, x_uint32 typeId, HazeComplexTypeInfo* info)
{
	m_Map[typeId] = { name, 0, typeId, *info };
}

x_uint32 HazeTypeInfoMap::RegisterFunctionParamListType(const HString& moduleName, x_uint32 typeId, V_Array<x_uint32>& paramList)
{
	HString str(H_TEXT("FuncParam<"));
	str += m_Map[typeId].Name;
	str += H_TEXT(">(");

	for (x_uint64 i = 0; i < paramList.size(); i++)
	{
		str += m_Map[paramList[i]].Name;
		str += H_TEXT(",");
	}

	if (paramList.size() > 0)
	{
		str.resize(str.length() - 1);
	}
	str += H_TEXT(")");

	auto iter = m_NameCache.find(str);
	if (iter != m_NameCache.end())
	{
		if (AddModuleRef(moduleName, iter->second))
		{
			m_Map[iter->second].RefCount++;
		}
		return iter->second;
	}

	assert(!str.empty());

	auto newTypeId = ++startIndex;

	HazeComplexTypeInfo info;
	info._Function.BaseType = HazeValueType::Function;
	info._Function.TypeId1 = typeId;
	info._Function.FunctionInfoIndex = newTypeId;

	m_Map[newTypeId] = { str, 1, newTypeId, info };
	m_NameCache[str] = newTypeId;

	// 将函数返回类型添加到第0个
	paramList.insert(paramList.begin(), typeId);
	m_FunctionInfoMap[newTypeId] = Move(paramList);

	AddModuleRef(moduleName, startIndex);

	return startIndex;
}

bool HazeTypeInfoMap::AddModuleRef(const HString& moduleName, x_uint32 typeId)
{
	if (m_ModuleRefTypes.find(moduleName) == m_ModuleRefTypes.end())
	{
		m_ModuleRefTypes[moduleName] = { { typeId } };
		return true;
	}
	else
	{
		m_ModuleRefTypes[moduleName].TypeIds.insert(typeId);
	}
	return false;
}
