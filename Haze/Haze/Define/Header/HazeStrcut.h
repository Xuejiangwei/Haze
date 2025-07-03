#pragma once

#include "HazeToken.h"
#include "HazeValue.h"
#include "HazeLog.h"

#ifdef HAZE
	#include <sstream>
	#include "JwHeader.h"
#endif

enum class HazeSectionSignal : x_uint8
{
	Global,
	Local,
	Static,
	Class,
	Enum,
	Closure,
};

struct HazeDefineType
{
	HazeValueType PrimaryType;				//Type类型
	HazeValueType SecondaryType;
	const HString* CustomName;

	HazeDefineType() : PrimaryType(HazeValueType::None), SecondaryType(HazeValueType::None), CustomName(nullptr)
	{
	}

	~HazeDefineType()
	{
	}

	HazeDefineType(HazeValueType type) : PrimaryType(type), SecondaryType(HazeValueType::None), CustomName(nullptr)
	{
		CheckValid();
	}

	HazeDefineType(HazeValueType type, HazeValueType type2) : PrimaryType(type), SecondaryType(type2), CustomName(nullptr)
	{
		CheckValid();
	}

	HazeDefineType(HazeValueType type, const HString* name) : PrimaryType(type), SecondaryType(HazeValueType::None), CustomName(name)
	{
		CheckValid();
	}

	HazeDefineType(HazeValueType type, HazeValueType type2, const HString* name) : PrimaryType(type), SecondaryType(type2), CustomName(name)
	{
		CheckValid();
	}

	void CheckValid() const
	{
#ifdef _DEBUG

		if (IsHazeBaseTypeAndVoid(PrimaryType)) {}
		else if (IsEnumType(PrimaryType) && IsIntegerType(SecondaryType)) {}
		else if (IsRefrenceType(PrimaryType) && IsHazeBaseType(SecondaryType)) {}
		else if (IsArrayType(PrimaryType) && IsHazeBaseType(SecondaryType)) {}
		else if (IsArrayType(PrimaryType) && IsClassType(SecondaryType) && CustomName && !CustomName->empty()) {}
		else if (IsClassType(PrimaryType) && IsNoneType(SecondaryType) && CustomName && !CustomName->empty()) {}
		else if (IsStringType(PrimaryType) && IsNoneType(SecondaryType) && !CustomName) {}
		else if (IsMultiVariableTye(PrimaryType) && IsNoneType(SecondaryType) && !CustomName) {}
		else if (IsFunctionType(PrimaryType) && IsNoneType(SecondaryType)) {}
		else if (IsDynamicClassUnknowType(PrimaryType) && IsNoneType(SecondaryType) && !CustomName) {}
		else if (IsPureStringType(PrimaryType) && IsNoneType(SecondaryType) && !CustomName) {}
		else if (IsClosureType(PrimaryType) && IsNoneType(SecondaryType) && !CustomName) {}
		else if (IsObjectBaseType(PrimaryType) && !CustomName) {}
		else
		{
			HAZE_LOG_ERR_W("基本类型指针的类型错误<%s><%s><%s>\n", GetHazeValueTypeString(PrimaryType), GetHazeValueTypeString(SecondaryType),
				CustomName ? CustomName->empty() ? H_TEXT("") : CustomName->c_str() : H_TEXT(""));
		}

#endif // _DEBUG
	}

	bool operator==(const HazeDefineType& type) const 
	{ return type.PrimaryType == PrimaryType && type.SecondaryType == SecondaryType && type.CustomName == CustomName; }

	bool operator!=(const HazeDefineType& type) const { return !(type == *this); }

	x_uint32 GetCompilerTypeSize() const { return IsEnumType(PrimaryType) ? GetSizeByHazeType(SecondaryType) : GetSizeByHazeType(PrimaryType); }
	
	x_uint32 GetTypeSize() const { return GetSizeByHazeType(PrimaryType); }

	void Reset() { PrimaryType = HazeValueType::None; SecondaryType = HazeValueType::None, CustomName = nullptr; }

	bool NeedSecondaryType() const { return IsArrayType(PrimaryType) || IsEnumType(PrimaryType) || IsRefrenceType(PrimaryType) || IsObjectBaseType(PrimaryType); }

	bool NeedCustomName() const { return IsClassType(PrimaryType) || (IsArrayType(PrimaryType) && IsClassType(SecondaryType)); }

	bool StringStreamTo(HAZE_STRING_STREAM& hss) const { return StringStreamTo(hss, *this); }

	bool IsStrongerType(const HazeDefineType& type) const
	{
		if (*this != type)
		{
			if (type.PrimaryType != PrimaryType)
			{
				auto strongerType = GetStrongerType(PrimaryType, type.PrimaryType, false);
				return strongerType == HazeValueType::None ? false : strongerType == PrimaryType;
			}
			else
			{
				return CustomName == type.CustomName;
			}
		}

		return false;
	}

	HString GetFullTypeName() const
	{
		HString fullName;

		if (CustomName)
		{
			fullName = *CustomName;
		}
		else
		{
			if (!NeedSecondaryType())
			{
				fullName = GetHazeValueTypeString(PrimaryType);
			}
			else
			{
				HAZE_LOG_ERR_W("获得类型<%s><%s><%s>的全长名错误\n", GetHazeValueTypeString(PrimaryType), 
					GetHazeValueTypeString(SecondaryType),
					CustomName ? CustomName->empty() ? H_TEXT("") : CustomName->c_str() : H_TEXT(""));
			}
		}
		
		return fullName;
	}

	void UpToRefrence()
	{
		SecondaryType = PrimaryType;
		PrimaryType = HazeValueType::Refrence;
	}

	void UpTypeToRefrence(const HazeDefineType& type)
	{
		*this = type;
		if (IsHazeBaseType(PrimaryType))
		{
			UpToRefrence();
		}
	}

	void UpToArray()
	{
		SecondaryType = PrimaryType;
		PrimaryType = HazeValueType::Array;
	}

	void ToArrayElement(const HazeDefineType& type)
	{
		if (IsArrayType(type.PrimaryType))
		{
			PrimaryType = type.SecondaryType;
			SecondaryType = HazeValueType::None;
			CustomName = type.CustomName;
		}
		else
		{
			HAZE_LOG_ERR_W("获得数组类型的成员类型错误, 不是数组类型\n");
		}
	}

	HazeDefineType GetArrayElement() const
	{
		if (IsArrayType(PrimaryType))
		{
			return { SecondaryType, CustomName };
		}
		else
		{
			HAZE_LOG_ERR_W("获得数组类型的成员类型错误, 不是数组类型\n");
			return HazeValueType::None;
		}
	}

	void Pointer()
	{
		PrimaryType = HazeValueType::UInt64;
		SecondaryType = HazeValueType::None;
		CustomName = nullptr;
	}

	void DynamicClassUnknow()
	{
		PrimaryType = HazeValueType::DynamicClassUnknow;
		SecondaryType = HazeValueType::None;
		CustomName = nullptr;
	}

	template<typename Class>
	void StringStream(Class* pThis, void(Class::* stringCall)(const HString*&), void(Class::* typeCall)(x_uint32&)) { StringStream(pThis, stringCall, typeCall, *this); }

	/*bool HasCustomName(const HazeDefineType& type)
	{
		return  type.CustomName && !type.CustomName->empty();
	}*/

	template<typename Class>
	static HazeDefineType StringStreamFrom(HAZE_IFSTREAM& stream, Class* pThis, const HString*(Class::* stringCall)(const HString&))
	{
		HazeDefineType type;
		stream >> *(x_uint32*)(&type.PrimaryType);

		if (type.NeedSecondaryType())
		{
			stream >> *(x_uint32*)(&type.SecondaryType);
		}

		if (type.NeedCustomName())
		{
			HString str;
			stream >> str;
			type.CustomName = (pThis->*stringCall)(str);
		}
	
		return type;
	}

	static bool StringStreamTo(HAZE_STRING_STREAM& hss, const HazeDefineType& type)
	{
		if (IsEnumType(type.PrimaryType))
		{
			hss << CAST_TYPE(type.SecondaryType);
			return true;
		}

		hss << CAST_TYPE(type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"暂时只读多参数的基本类型");
		}*/

		if (type.NeedSecondaryType())
		{
			hss << " " << CAST_TYPE(type.SecondaryType);
		}

		if (type.NeedCustomName())
		{
			if (type.CustomName)
			{
				hss << " " << *type.CustomName;
			}
			else
			{
				return false;
			}
		}

		//MoreTypeStream

		return true;
	}

	template<typename Class>
	static void StringStream(Class* pThis, void(Class::* stringCall)(const HString*&), void(Class::* typeCall)(x_uint32&), HazeDefineType& type)
	{
		(pThis->*typeCall)((x_uint32&)type.PrimaryType);

		/*if (Type.PrimaryType == HazeValueType::MultiVariable)
		{
			HazeLog::LogInfo(HazeLog::Error, L"%s\n", L"Haze to do : " L"暂时只写多参数的基本类型");
		}*/

		if (type.NeedSecondaryType())
		{
			(pThis->*typeCall)((x_uint32&)type.SecondaryType);
		}

		if (type.NeedCustomName())
		{
			(pThis->*stringCall)(type.CustomName);
		}
	}

	static const HazeDefineType& VoidType()
	{
		static HazeDefineType s_Type(HazeValueType::Void);
		return s_Type;
	}

	static const HazeDefineType& StringType()
	{
		static HazeDefineType s_Type(HazeValueType::String);
		return s_Type;
	}
};

struct HazeNewDefineType
{
	HazeDefineType BaseType;
	x_uint64 ArrayDimension;
};

struct HazeDefineTypeHashFunction
{
	x_uint64 operator()(const HazeDefineType& type) const
	{
		if (type.CustomName && !type.CustomName->empty())
		{
			return std::hash<HString>()(*type.CustomName);
		}
		else
		{
			return (x_uint64)type.PrimaryType * 100 + (x_uint64)type.SecondaryType * 10;
		}
	}
};

struct HazeDefineVariable
{
	HazeDefineType Type;		//变量类型
	HString Name;				//变量名

	HazeDefineVariable() {}
	HazeDefineVariable(const HazeDefineType& type, const HString& name)
		: Type(type), Name(name) {}
};

struct HazeVariableData
{
	HazeDefineVariable Variable;
	int Offset;
	x_uint32 Size;
	x_uint32 Line;
};

struct HazeTempRegisterData
{
	HString Name;
	x_uint32 Offset;
	HazeDefineType Type;
};

struct TemplateDefineTypes
{
	V_Array<struct TemplateDefineType> Types;

	TemplateDefineTypes()
	{
		Types.clear();
	}
};

struct TemplateDefineType
{
	bool IsDefines;
	Share<TemplateDefineTypes> Defines;
	Share<HazeNewDefineType> Type;

	TemplateDefineType() : IsDefines(false)
	{
	}

	TemplateDefineType(bool isDefines, Share<TemplateDefineTypes> defineTypes, Share<HazeNewDefineType> type) : IsDefines(isDefines)
	{
		if (IsDefines)
		{
			Defines = defineTypes;
		}
		else
		{
			Type = type;
		}
	}

	TemplateDefineType(const TemplateDefineType& type)
		: TemplateDefineType(type.IsDefines, type.Defines, type.Type)
	{}
		
	TemplateDefineType& operator=(const TemplateDefineType& type)
	{
		if (IsDefines)
		{
			Defines = type.Defines;
		}
		else
		{
			Type = type.Type;
		}

		return *this;
	}

	/*~TemplateDefineType()
	{
	}*/
};

struct HazFrameFunctionData
{
	V_Array<HazeDefineVariable*> LocalParams;
};

// 类型记录系统 - 类似Java的Class对象
struct HazeTypeInfo
{
	HazeDefineType BaseType;				// 基础类型信息
	HString TypeName;						// 类型名称
	HString ModuleName;						// 所属模块
	x_uint32 TypeId;						// 类型唯一ID
	x_uint32 Size;							// 类型大小
	bool IsGeneric;							// 是否为泛型
	bool IsArray;							// 是否为数组
	x_uint64 ArrayDimension;				// 数组维度
	
	// 泛型参数信息
	V_Array<Share<HazeTypeInfo>> GenericParams;
	
	// 类成员信息（如果是类类型）
	V_Array<HazeVariableData> Members;
	
	// 继承信息
	V_Array<Share<HazeTypeInfo>> ParentTypes;
	
	// 创建时间戳
	x_uint64 CreateTime;
	
	HazeTypeInfo() : TypeId(0), Size(0), IsGeneric(false), IsArray(false), ArrayDimension(0), CreateTime(0) {}
	
	HazeTypeInfo(const HazeDefineType& type, const HString& name, const HString& module)
		: BaseType(type), TypeName(name), ModuleName(module), TypeId(0), Size(0), 
		  IsGeneric(false), IsArray(false), ArrayDimension(0), CreateTime(0) {}
};

// 类型注册表 - 管理所有类型信息
class HazeTypeRegistry
{
public:
	static HazeTypeRegistry& GetInstance()
	{
		static HazeTypeRegistry instance;
		return instance;
	}
	
	// 注册类型
	x_uint32 RegisterType(const HazeDefineType& type, const HString& name, const HString& module);
	
	// 注册复杂类型（泛型、数组等）
	x_uint32 RegisterComplexType(const HazeDefineType& type, const HString& name, const HString& module,
		const TemplateDefineTypes& templateTypes, x_uint64 arrayDimension = 0);
	
	// 获取类型信息
	Share<HazeTypeInfo> GetTypeInfo(x_uint32 typeId);
	Share<HazeTypeInfo> GetTypeInfo(const HString& typeName, const HString& module = H_TEXT(""));
	
	// 查询类型
	bool IsTypeRegistered(const HazeDefineType& type, const HString& name, const HString& module);
	
	// 获取类型的完整描述
	HString GetTypeDescription(x_uint32 typeId);
	
	// 类型兼容性检查
	bool IsTypeCompatible(x_uint32 typeId1, x_uint32 typeId2);
	
	// 序列化类型信息
	void SerializeTypeInfo(HAZE_STRING_STREAM& hss, x_uint32 typeId);
	
	// 反序列化类型信息
	x_uint32 DeserializeTypeInfo(HAZE_IFSTREAM& stream);
	
	// 获取所有已注册类型
	const HashMap<x_uint32, Share<HazeTypeInfo>>& GetAllTypes() const { return m_TypeInfos; }
	
private:
	HashMap<x_uint32, Share<HazeTypeInfo>> m_TypeInfos;
	HashMap<HString, x_uint32> m_TypeNameToId;
	x_uint32 m_NextTypeId;
	
	HazeTypeRegistry() : m_NextTypeId(1) {}
	
	// 生成类型的唯一名称
	HString GenerateUniqueTypeName(const HazeDefineType& type, const HString& name, const HString& module,
		const TemplateDefineTypes& templateTypes, x_uint64 arrayDimension);
};

// 变量的增强定义 - 包含类型记录信息
struct HazeEnhancedDefineVariable : public HazeDefineVariable
{
	x_uint32 TypeId;						// 类型ID
	Share<HazeTypeInfo> TypeInfo;			// 类型信息
	HString SourceLocation;					// 源码位置
	x_uint32 LineNumber;					// 行号
	
	HazeEnhancedDefineVariable() : TypeId(0), LineNumber(0) {}
	
	HazeEnhancedDefineVariable(const HazeDefineVariable& var, x_uint32 typeId, const HString& location, x_uint32 line)
		: HazeDefineVariable(var), TypeId(typeId), SourceLocation(location), LineNumber(line) {}
};