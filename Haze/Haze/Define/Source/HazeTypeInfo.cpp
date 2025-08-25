#include "HazePch.h"
#include "HazeTypeInfo.h"
#include "Compiler.h"

// type id需要与枚举值相等
//HashMap<HazeValueType, x_uint32> g_HazeTypeIdMap =
//{
//#define HAZE_TYPE_DEFINE(TYPE) { HazeValueType::TYPE, (x_uint32)HazeValueType::TYPE },
//	#include "HazeValueTypeTemplate"
//#undef HAZE_TYPE_DEFINE
//};

#define COMPLEX_TYPE_START 100

struct ResetInfoNameAddress
{
	ResetInfoNameAddress(HazeTypeInfoMap::TypeInfo& info) : Info(info) {}

	~ResetInfoNameAddress()
	{
		switch (Info.Info._BaseType.BaseType)
		{
			case HazeValueType::Class:
				Info.Info._Class.SetName(&Info.Name);
				break;
			case HazeValueType::Enum:
				Info.Info._Enum.SetName(&Info.Name);
				break;
			default:
				break;
		}
	}

private:
	HazeTypeInfoMap::TypeInfo& Info;
};

HazeTypeInfoMap::HazeTypeInfoMap(Compiler* compiler) : m_Compiler(compiler)
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

x_uint32 HazeTypeInfoMap::ReserveTypeId(const HString& name)
{
	x_uint32 newTypeId = GetNewTypeId(name);

	// 一律先当作类来存储
	CLASS_TYPE_INFO(info, name);
	m_Map[newTypeId] = { name, 1, newTypeId, info };
	ResetInfoNameAddress temp(m_Map[newTypeId]);

	return newTypeId;
}

x_uint32 HazeTypeInfoMap::RegisterType(const HString& moduleName, HazeComplexTypeInfo* info, x_uint32 resolvedTypeId)
{
	HString str;
	switch (info->_BaseType.BaseType)
	{
		case HazeValueType::Array:
		{
			auto complexType = (HazeComplexTypeInfo::Array*)info;
			
			if (complexType->Dimension > 1)
			{
				HazeComplexTypeInfo elementInfo = *info;
				elementInfo._Array.Dimension -= 1;
				info->_Array.TypeId1 = RegisterType(moduleName, &elementInfo);
			}

			str += H_TEXT("Array<");
			str += m_Map[complexType->TypeId1].Name;
			str += H_TEXT(">") + ToHazeString(complexType->Dimension);
		}
			break;
		case HazeValueType::Enum:
		{
			auto complexType = (HazeComplexTypeInfo::Enum*)info;
			str = *complexType->GetString();
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

	x_uint32 newTypeId = resolvedTypeId;
	if (newTypeId == 0)
	{
		newTypeId = GetNewTypeId(str);
	}
	
	m_Map[newTypeId] = { str, 1, newTypeId, *info };
	ResetInfoNameAddress temp(m_Map[newTypeId]);

	m_NameCache[str] = newTypeId;
	AddModuleRef(moduleName, newTypeId);

	return newTypeId;
}

x_uint32 HazeTypeInfoMap::RegisterType(const HString& moduleName, x_uint32 functionTypeId, V_Array<x_uint32>&& paramTypeId)
{
	return RegisterFunctionParamListType(moduleName, functionTypeId, paramTypeId);
}

HazeVariableType HazeTypeInfoMap::GetVarTypeById(x_uint32 typeId)
{
	if (typeId <= COMPLEX_TYPE_START)
	{
		return HazeVariableType(HazeValueType(typeId), typeId);
	}

	auto info = GetTypeById(typeId);
	return HazeVariableType(info->GetBaseType(), typeId);
}

const HString* HazeTypeInfoMap::GetClassNameById(x_uint32 typeId)
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
	auto iter = m_Map.find(typeId);
	if (iter != m_Map.end())
	{
		return &iter->second;
	}

	return nullptr;
}

const HString* HazeTypeInfoMap::GetTypeName(x_uint32 typeId)
{
	auto iter = m_Map.find(typeId);
	if (iter != m_Map.end())
	{
		return &iter->second.Name;
	}

	return nullptr;
}

const HazeComplexTypeInfo* HazeTypeInfoMap::GetTypeById(x_uint32 typeId)
{
	auto iter = m_Map.find(typeId);
	if (iter != m_Map.end())
	{
		return &iter->second.Info;
	}

	return nullptr;
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

HazeTypeInfoMap::ModuleRefrenceTypeId& HazeTypeInfoMap::GetModuleRefTypeId(const HString& name)
{
	return m_ModuleRefTypes[name];
}

void HazeTypeInfoMap::GenICode(HAZE_STRING_STREAM& hss)
{
	hss << GetTypeInfoFunctionBeginHeader() << HAZE_ENDL_D;
	
	for (auto& iter : m_FunctionInfoMap)
	{
		hss << iter.first << " " << iter.second.size() << " ";

		for (x_uint64 i = 0; i < iter.second.size(); i++)
		{
			hss << iter.second[i] << " ";
		}
		hss << HAZE_ENDL;
	}
	
	hss << HAZE_ENDL << GetTypeInfoFunctionEndHeader() << HAZE_ENDL_D;

	hss << TYPE_INFO_BEGIN << HAZE_ENDL_D;

	for (auto& iter : m_Map)
	{
		auto& typeInfo = iter.second;
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

	hss << HAZE_ENDL << TYPE_INFO_END << HAZE_ENDL_D;

	for (auto& m : m_ModuleRefTypes)
	{
		hss << GetRefTypeIdString() << HAZE_ENDL;
		hss << m.first << HAZE_ENDL;
		hss << m.second.TypeIds.size() << HAZE_ENDL;
		for (auto& typeId : m.second.TypeIds)
		{
			hss << typeId << HAZE_ENDL;
		}
	}

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

const HString* HazeTypeInfoMap::GetRegisterTypeModule(const HString& symbol)
{
	return GetRegisterTypeModule(GetTypeId(symbol));
}

const HString* HazeTypeInfoMap::GetRegisterTypeModule(x_uint32 typeId)
{
	for (auto& it : m_ModuleRefTypes)
	{
		if (it.second.DefineTypes.find(typeId) != it.second.DefineTypes.end())
		{
			return &it.first;
		}
	}

	return nullptr;
}

void HazeTypeInfoMap::RegisterModuleRefTypes(const HString& moduleName, ModuleRefrenceTypeId&& refTypes)
{
	m_ModuleRefTypes[moduleName] = refTypes;
}

void HazeTypeInfoMap::RemoveModuleRefTypes(const HString& moduleName)
{
	auto iter = m_ModuleRefTypes.find(moduleName);
	if (iter != m_ModuleRefTypes.end())
	{
		for (auto& typeId : iter->second.TypeIds)
		{
			auto info = m_Map.find(typeId);
			if (info != m_Map.end())
			{
				if (--info->second.RefCount == 0)
				{
					m_NameCache.erase(m_NameCache.find(info->second.Name));
					//m_NoRefTypeIds.insert(typeId);
					m_Map.erase(info);
				}
			}
		}
		m_ModuleRefTypes.erase(iter);
	}
}

void HazeTypeInfoMap::ParseInterFile(HAZE_IFSTREAM& stream)
{
	HString str;
	stream >> str;
	if (str == GetTypeInfoFunctionBeginHeader())
	{
		stream >> str;
		while (str != GetTypeInfoFunctionEndHeader())
		{
			x_uint32 typeId;
			stream >> typeId;

			x_uint32 count;
			stream >> count;
			
			V_Array<x_uint32> params(count);
			for (x_uint64 i = 0; i < count; i++)
			{
				stream >> params[i];
			}
			
			m_FunctionInfoMap[typeId] = Move(params);
			stream >> str;
		}
	}

	
	stream >> str;
	if (str == GetTypeInfoBeginHeader())
	{
		stream >> str;
		while (str != GetTypeInfoEndHeader())
		{
			TypeInfo info;
			info.Name = str;
			stream >> info.RefCount;
			stream >> info.TypeId;
			stream >> (x_uint32&)info.Info._BaseType.BaseType;

			switch (info.Info.GetBaseType())
			{
				case HazeValueType::ObjectBase:
					stream >> info.Info._ObjectBase.TypeId1;
					break;
				case HazeValueType::Class:
				case HazeValueType::Enum:
					break;
				case HazeValueType::Array:
					stream >> info.Info._Array.TypeId1 >> info.Info._Array.Dimension;
					break;
				case HazeValueType::Hash:
					stream >> info.Info._Hash.TypeId1  >> info.Info._Hash.TypeId2;
					break;
				case HazeValueType::Function:
					stream >> info.Info._Function.TypeId1 >> info.Info._Function.FunctionInfoIndex;
					break;
				default:
					break;
			}

			m_Map[info.TypeId] = info;
			m_NameCache[info.Name] = info.TypeId;
			stream >> str;

			ResetInfoNameAddress temp(m_Map[info.TypeId]);
		}
	}

	stream >> str;
	while (str == GetRefTypeIdString())
	{
		stream >> str;

		x_uint64 count;
		stream >> count;
		ModuleRefrenceTypeId refTypeIds;
		for (x_uint64 i = 0; i < count; i++)
		{
			x_uint32 typeId;
			stream >> typeId;
			refTypeIds.TypeIds.insert(typeId);
		}

		m_ModuleRefTypes[str] = Move(refTypeIds);
		stream >> str;
	}
}

x_uint32 HazeTypeInfoMap::GetNewTypeId(const HString& symbol)
{
#define SIP_ROTATE_LEFT(X, BITS) ((X << BITS) | (X >> (64 - BITS)))
#define SIP_ROUND(V0, V1, V2, V3) V0 += V1; V1 = SIP_ROTATE_LEFT(V1, 13); V1 ^= V0; V0 = SIP_ROTATE_LEFT(V0, 32); \
								  v2 += v3; v3 = SIP_ROTATE_LEFT(v3, 16); v3 ^= v2; \
								  v0 += v3; v3 = SIP_ROTATE_LEFT(v3, 21); v3 ^= v0; \
								  v2 += v1; v1 = SIP_ROTATE_LEFT(v1, 17); v1 ^= v2; v2 = SIP_ROTATE_LEFT(v2, 32) \

#define SIP_FINISH_64() (v0 ^ v1 ^ v2 ^ v3)
#define SIP_FINISH_32() static_cast<uint32_t>(SIP_FINISH_64() ^ (SIP_FINISH_64() >> 32))

	// 简化的SipHash实现，固定种子确保一致性
	x_uint64 v0 = 0x736f6d6570736575ULL;
	x_uint64 v1 = 0x646f72616e646f6dULL;
	x_uint64 v2 = 0x6c7967656e657261ULL;
	x_uint64 v3 = 0x7465646279746573ULL;

	for (x_uint64 i = 0; i < symbol.length(); ++i)
	{
		auto c = symbol[i];
		v3 ^= c;
		
		SIP_ROUND(v0, v1, v2, v3);

		v0 ^= c;
	}
	
	for (int i = 0; i < 4; i++)
	{
		SIP_ROUND(v0, v1, v2, v3);
	}

	x_uint32 newTypeId = (SIP_FINISH_32() % std::numeric_limits<x_uint32>::max()) + COMPLEX_TYPE_START;

	auto iter = m_Map.find(newTypeId);
	if (iter != m_Map.end())
	{
		auto typeName = GetTypeName(newTypeId);
		if (typeName && *typeName != symbol)
		{
			SYMBOL_ERR_W("<%s>和<%s>生成冲突的类型ID", typeName->c_str(), symbol.c_str());
		}
	}

	return newTypeId;
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

	auto newTypeId = GetNewTypeId(str);

	HazeComplexTypeInfo info;
	info._Function.BaseType = HazeValueType::Function;
	info._Function.TypeId1 = typeId;
	info._Function.FunctionInfoIndex = newTypeId;

	m_Map[newTypeId] = { str, 1, newTypeId, info };
	m_NameCache[str] = newTypeId;

	// 将函数返回类型添加到第0个
	paramList.insert(paramList.begin(), typeId);
	m_FunctionInfoMap[newTypeId] = Move(paramList);

	AddModuleRef(moduleName, newTypeId);

	return newTypeId;
}

void HazeTypeInfoMap::RegisterResolvedType(const HString& moduleName, x_uint32 typeId, HazeComplexTypeInfo* type)
{
	RegisterType(moduleName, type, typeId);
	m_ModuleRefTypes[moduleName].DefineTypes.insert(typeId);
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
		auto& typeIds = m_ModuleRefTypes[moduleName].TypeIds;
		if (typeIds.find(typeId) == typeIds.end())
		{
			typeIds.insert(typeId);
			return true;
		}
	}
	return false;
}
