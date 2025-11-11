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
	V_Array<Pair<STDString, x_uint32>> PublicMembers;
	V_Array<Pair<STDString, x_uint32>> PrivateMembers;

	V_Array<struct FunctionSymbol*> Functions;
};

struct EnumSymbol : SymbolTypeHeader
{
	// 名称和值
	V_Array<Pair<STDString, int>> Members;
};

struct FunctionSymbol : SymbolHeader
{
	CompilerModule* Module = nullptr;
	bool ClassFunctionIsPublic = false;
	x_uint32 FunctionId = 0;
	HazeFunctionDesc Desc;
	x_uint32 ClassTypeId = 0;
	x_uint32 FunctionType;
	V_Array<x_uint32> Params;
	V_Array<STDString> ParamNames;
};

struct GlobalVariableSymbol //: SymbolHeader
{
	CompilerModule* Module = nullptr;
	x_uint64 VarId = 0;
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

	x_uint32 RegisterSymbol(const STDString& name);

	void Register_GlobalVariable(const STDString& moduleName, const STDString& name, HazeVariableType type, x_uint32 line);
	void Register_Class(const STDString& moduleName, const STDString& name, V_Array<STDString>& parents, V_Array<Pair<STDString, x_uint32>>&& publicMembers, V_Array<Pair<STDString, x_uint32>>&& privateMembers, bool publicFirst);
	void Register_Enum(const STDString& moduleName, const STDString& name, V_Array<Pair<STDString, int>>&& members);
	void Register_Function(const STDString& moduleName, const STDString& name, x_uint32 functionType, V_Array<HazeDefineVariableView>&& params, HazeFunctionDesc desc, const STDString* className = nullptr, bool isClassPublic = false);

	void AddModuleRefSymbol(const STDString& moduleName, const STDString& symbol);

	//当第一遍收集完所有符号后，在第二遍解析前，已经可以将所有之前未定的符号，如暂时将枚举当做类，这些都全部纠正过来，可以加个是否正确确定所有类型的标记和log看哪些被修改过来了。
	void IdentifySymbolType();

	bool IsValidSymbol(const STDString& symbol);
	bool IsValidClassSymbol(const STDString& symbol);
	x_uint32 GetSymbolTypeId(const STDString& symbol);
	x_uint32 GetGlobalVariableId(const STDString& name);
	x_uint32 GetFunctionId(const STDString& name);
	const STDString* GetSymbolByTypeId(x_uint32 typeId);
	
private:
	bool CheckValidClassInherit(x_uint32 checkTypeId, x_uint32 typeId, const HashMap<x_uint32, ResolveClassData>& classMap);

	bool ResolveCompilerClass(HashMap<x_uint32, ResolveClassData>& classMap, x_uint32 typeId);

private:
	Compiler* m_Compiler;
	HazeTypeInfoMap* m_TypeInfo;

	HashMap<STDString, Pair<x_uint32, SymbolTypeHeader*>> m_Symbols;

	HashMap<STDString, FunctionSymbol> m_FunctionSymbols;

	HashMap<STDString, GlobalVariableSymbol> m_GlobalVariables;
};