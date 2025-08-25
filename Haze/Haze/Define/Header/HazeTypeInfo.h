#pragma once

class Compiler;

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

	typedef Class Enum;

	// 函数返回类型存储在TypeId1种
	struct Function : public BaseTemplate
	{
		// 为当前TypeId
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
		BaseObject _ObjectBase;
		Class _Class;
		Enum _Enum;
		Array _Array;
		Hash _Hash;
		Function _Function;
	};

	HazeValueType GetBaseType() const { return _BaseType.BaseType; }

	HazeComplexTypeInfo() {}
	HazeComplexTypeInfo(HazeValueType type) { _BaseType.BaseType = type; }
};

#define TYPE_INFO_VAR(INFO) HazeComplexTypeInfo INFO
#define ARRAY_TYPE_INFO(INFO, ID1, DIMENSION) TYPE_INFO_VAR(INFO); { INFO._Array.BaseType = HazeValueType::Array; INFO._Array.TypeId1 = ID1, INFO._Array.Dimension = DIMENSION; }
#define HASH_TYPE_INFO(INFO, ID1, ID2) TYPE_INFO_VAR(INFO); { INFO._Hash.BaseType = HazeValueType::Hash; INFO._Hash.TypeId1 = ID1; INFO._Hash.TypeId2 = ID2; }
#define FUNCTION_TYPE_INFO(INFO, ID1, INDEX)  TYPE_INFO_VAR(INFO); { INFO._Function.BaseType = HazeValueType::Function; INFO._Function.TypeId1 = ID1; INFO._Function.FunctionInfoIndex = INDEX; }
#define OBJ_BASE_TYPE_INFO(INFO, ID1) TYPE_INFO_VAR(INFO); { INFO._Function.BaseType = HazeValueType::ObjectBase; INFO._ObjectBase.TypeId1 = ID1; }
#define CLASS_TYPE_INFO(INFO, NAME) TYPE_INFO_VAR(INFO); { INFO._Class.BaseType = HazeValueType::Class; INFO._Class.SetName(&NAME); }
#define ENUM_TYPE_INFO(INFO, NAME) TYPE_INFO_VAR(INFO); { INFO._Enum.BaseType = HazeValueType::Enum; INFO._Enum.SetName(&NAME); }

class HazeTypeInfoMap
{
	friend class CompilerSymbol;
public:
	struct TypeInfo
	{
		HString Name;
		x_uint32 RefCount;
		x_uint32 TypeId;
		HazeComplexTypeInfo Info;
	};

	struct ModuleRefrenceTypeId
	{
		Set<x_uint32> TypeIds;
		Set<x_uint32> DefineTypes;
	};

public:
	HazeTypeInfoMap(Compiler* compiler);
	~HazeTypeInfoMap();

	x_uint32 ReserveTypeId(const HString& name);
	//void ResolveTypeId(x_uint32 typeId);

	x_uint32 RegisterType(const HString& moduleName, HazeComplexTypeInfo* type, x_uint32 resolvedTypeId = 0);
	x_uint32 RegisterType(const HString& moduleName, x_uint32 functionTypeId, V_Array<x_uint32>&& paramTypeId);

	HazeVariableType GetVarTypeById(x_uint32 typeId);

	const HString* GetClassNameById(x_uint32 typeId);

	const HString* GetEnumName(x_uint32 typeId);

	const TypeInfo* GetTypeInfoById(x_uint32 typeId);

	const HString* GetTypeName(x_uint32 typeId);

	const HazeComplexTypeInfo* GetTypeById(x_uint32 typeId);

	const x_uint32 GetTypeId(const HString& name) const;

	ModuleRefrenceTypeId& GetModuleRefTypeId(const HString& name);

	void GenICode(HAZE_STRING_STREAM& hss);
	void GenModuleReferenceTypeInfo(HAZE_STRING_STREAM& hss, const HString& moduleName);

	// 通用函数
public:
	const x_uint32 GetTypeIdByClassName(const HString& name) const;

public:
	void AddFunctionTypeInfo(x_uint32 typeId, V_Array<x_uint32>& typeAndParams);
	void AddTypeInfo(HString&& name, x_uint32 typeId, HazeComplexTypeInfo* info);

	const HString* GetRegisterTypeModule(const HString& symbol);
	const HString* GetRegisterTypeModule(x_uint32 typeId);
	void RegisterModuleRefTypes(const HString& moduleName, ModuleRefrenceTypeId&& refTypes);
	void RemoveModuleRefTypes(const HString& moduleName);

	void ParseInterFile(HAZE_IFSTREAM& stream);

private:
	x_uint32 GetNewTypeId(const HString& symbol);

	x_uint32 RegisterFunctionParamListType(const HString& moduleName, x_uint32 typeId, V_Array<x_uint32>& paramList);

	void RegisterResolvedType(const HString& moduleName, x_uint32 typeId, HazeComplexTypeInfo* type);

	bool AddModuleRef(const HString& moduleName, x_uint32 typeId);

private:
	Compiler* m_Compiler;

	HashMap<x_uint32, TypeInfo> m_Map;
	HashMap<HString, x_uint32> m_NameCache;
	HashMap<x_int32, V_Array<x_uint32>> m_FunctionInfoMap;
	
	HashMap<HString, ModuleRefrenceTypeId> m_ModuleRefTypes;
};


