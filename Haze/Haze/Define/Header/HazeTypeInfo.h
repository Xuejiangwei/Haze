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
	typedef BaseTemplate GlobalVariable;

	struct Class : public HazeComplexTypeInfoBase
	{
		char NameAddress[8];
		//x_HChar* ClassName;

		void SetName(const STDString* name)
		{
			memcpy(NameAddress, &name, sizeof(name));
		}

		const STDString* GetString() const
		{
			STDString* str;
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
		GlobalVariable _GlobalVariable;
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
#define GLOBAL_VAR_TYPE_INFO(INFO, TYPE) TYPE_INFO_VAR(INFO); { INFO._GlobalVariable.BaseType = TYPE.BaseType; INFO._Array.TypeId1 = TYPE.TypeId; }
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
		STDString Name;
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

	x_uint32 ReserveTypeId(const STDString& name);
	//void ResolveTypeId(x_uint32 typeId);

	x_uint32 RegisterType(const STDString& moduleName, HazeComplexTypeInfo* type, x_uint32 resolvedTypeId = 0);
	x_uint32 RegisterType(const STDString& moduleName, x_uint32 functionTypeId, V_Array<x_uint32>&& paramTypeId);
	x_uint32 RegisterSymbol(const STDString& moduleName, const STDString& symbol, HazeComplexTypeInfo* type);

	HazeVariableType GetVarTypeById(x_uint32 typeId);

	const STDString* GetClassNameById(x_uint32 typeId);

	const STDString* GetEnumName(x_uint32 typeId);

	const TypeInfo* GetTypeInfoById(x_uint32 typeId);

	const STDString* GetTypeName(x_uint32 typeId);

	const HazeComplexTypeInfo* GetTypeById(x_uint32 typeId);

	const x_uint32 GetTypeId(const STDString& name) const;

	ModuleRefrenceTypeId& GetModuleRefTypeId(const STDString& name);

	void GenICode(HAZE_STRING_STREAM& hss);
	void GenModuleReferenceTypeInfo(HAZE_STRING_STREAM& hss, const STDString& moduleName);

	// 通用函数
public:
	const x_uint32 GetTypeIdByClassName(const STDString& name) const;

public:
	void AddFunctionTypeInfo(x_uint32 typeId, V_Array<x_uint32>& typeAndParams);
	void AddTypeInfo(STDString&& name, x_uint32 typeId, HazeComplexTypeInfo* info);

	const STDString* GetRegisterTypeModule(const STDString& symbol);
	const STDString* GetRegisterTypeModule(x_uint32 typeId);
	void RegisterModuleRefTypes(const STDString& moduleName, ModuleRefrenceTypeId&& refTypes);
	void RemoveModuleRefTypes(const STDString& moduleName);

	void ParseInterFile(HAZE_IFSTREAM& stream);

private:
	x_uint32 GetNewTypeId(const STDString& symbol);

	x_uint32 RegisterFunctionParamListType(const STDString& moduleName, x_uint32 typeId, V_Array<x_uint32>& paramList);

	void RegisterResolvedType(const STDString& moduleName, x_uint32 typeId, HazeComplexTypeInfo* type);

	bool AddModuleRef(const STDString& moduleName, x_uint32 typeId);

private:
	Compiler* m_Compiler;

	HashMap<x_uint32, TypeInfo> m_Map;
	HashMap<STDString, x_uint32> m_NameCache;
	HashMap<x_int32, V_Array<x_uint32>> m_FunctionInfoMap;
	
	HashMap<STDString, ModuleRefrenceTypeId> m_ModuleRefTypes;
};


