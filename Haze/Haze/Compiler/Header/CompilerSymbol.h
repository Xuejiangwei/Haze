#pragma once

class Compiler;
class CompilerClass;
class CompilerModule;
class HazeTypeInfoMap;

struct SymbolHeader
{
	//bool IsResolved = false;
};

struct SymbolTypeHeader : SymbolHeader
{
	HazeValueType Type = HazeValueType::None;
};

struct ClassSymbol : SymbolTypeHeader
{
	bool PublicFirst;
	V_Array<x_uint32> Parents;
	V_Array<Pair<HString, x_uint32>> PublicMembers;
	V_Array<Pair<HString, x_uint32>> PrivateMembers;

	V_Array<struct FunctionSymbol*> Functions;
};

struct EnumSymbol : SymbolTypeHeader
{
	// 名称和值
	V_Array<Pair<HString, int>> Members;
};

struct FunctionSymbol : SymbolHeader
{
	CompilerModule* Module = nullptr;
	bool ClassFunctionIsPublic = false;
	HazeFunctionDesc Desc;
	x_uint32 ClassTypeId = 0;
	x_uint32 FunctionType;
	V_Array<x_uint32> Params;
	V_Array<HString> ParamNames;
};

struct GlobalVariableSymbol //: SymbolHeader
{
	CompilerModule* Module = nullptr;
	x_uint32 TypeId = 0;
	x_uint32 Line = 0;
};

//这里记录与验证类和函数(包括类函数), 等第二遍解析时就会有所有的确定的类型了
class CompilerSymbol
{
	struct ResolveClassData
	{
		Share<CompilerClass> CompClass = nullptr;
		ClassSymbol* SymbolInfo = nullptr;
		bool IsResolved = false;
	};
public:
	CompilerSymbol(Compiler* compiler);
	
	~CompilerSymbol();

	HazeTypeInfoMap* GetTypeInfoMap() { return m_TypeInfo; }

	x_uint32 RegisterSymbol(const HString& name);
	//void RegisterFunctionSymbol(const HString& name, x_uint32 functionType, V_Array<x_uint32>&& params, HazeFunctionDesc desc, const HString* className = nullptr, bool isClassPublic = false);

	void Register_GlobalVariable(const HString& moduleName, const HString& name, x_uint32 typeId, x_uint32 line);
	void Register_Class(const HString& moduleName, const HString& name, V_Array<HString>& parents, V_Array<Pair<HString, x_uint32>>&& publicMembers, V_Array<Pair<HString, x_uint32>>&& privateMembers, bool publicFirst);
	void Register_Enum(const HString& moduleName, const HString& name, V_Array<Pair<HString, int>>&& members);
	void Register_Function(const HString& moduleName, const HString& name, x_uint32 functionType, V_Array<HazeDefineVariable>&& params, HazeFunctionDesc desc, const HString* className = nullptr, bool isClassPublic = false);

	void AddModuleRefSymbol(const HString& moduleName, const HString& symbol);

	/*void ResolveSymbol_Class(const HString& name, class CompilerClass* compilerClass);
	void ResolveSymbol_Enum(const HString& name, class CompilerEnum* compilerEnum);*/
	//void ResolveSymbol_Enum(const HString& name, V_Array<Pair<HString, x_uint32>> members);

	//当第一遍收集完所有符号后，在第二遍解析前，已经可以将所有之前未定的符号，如暂时将枚举当做类，这些都全部纠正过来，可以加个是否正确确定所有类型的标记和log看哪些被修改过来了。
	void IdentifySymbolType();

	bool IsValidSymbol(const HString& symbol);
	x_uint32 GetSymbolTypeId(const HString& symbol);
	const HString* GetSymbolByTypeId(x_uint32 typeId);
	
private:
	bool CheckValidClassInherit(x_uint32 checkTypeId, x_uint32 typeId, const HashMap<x_uint32, ResolveClassData>& classMap);

	bool ResolveCompilerClass(HashMap<x_uint32, ResolveClassData>& classMap, x_uint32 typeId);

private:
	Compiler* m_Compiler;
	HazeTypeInfoMap* m_TypeInfo;

	HashMap<HString, Pair<x_uint32, SymbolTypeHeader*>> m_Symbols;

	HashMap<HString, FunctionSymbol> m_FunctionSymbols;

	HashMap<HString, GlobalVariableSymbol> m_GlobalVariables;
};