#pragma once

class CompilerModule;
class CompilerFunction;
class CompilerValue;
class HazeCompilerPointerValue;
class CompilerClassValue;
class ASTBase;

class CompilerClass
{
	friend class Compiler;
	friend class CompilerSymbol;
	friend class ASTNew;
public:

	CompilerClass(CompilerModule* compilerModule, const STDString& name, V_Array<CompilerClass*>& parentClass,
		V_Array<Pair<STDString, Share<CompilerValue>>>& data, x_uint32 typeId);

	CompilerClass(CompilerModule* compilerModule, const STDString& name, x_uint32 typeId);

	~CompilerClass();

	Share<CompilerFunction> FindFunction(const STDString& functionName, const STDString* nameSpace);

	Share<CompilerFunction> AddFunction(Share<CompilerFunction>& function);

	x_uint64 GetFunctionNum() { return m_Functions.size(); }

	x_uint32 GetTypeId() const { return m_TypeId; }

	const STDString& GetName() { return m_Name; }

	int GetMemberIndex(const STDString& memberName, const STDString* nameSpace);

	int GetMemberIndex(const V_Array<STDString>& classNames, const STDString& memberName);

	//int GetMemberIndex(CompilerValue* value);

	//bool GetThisMemberName(const HazeCompilerValue* value, HString& outName, bool getOffset = false, V_Array<uint64>* offsets = nullptr);

	//bool GetMemberName(CompilerClassValue* classValue, const CompilerValue* value, HString& outName, bool getOffset = false, V_Array<Pair<uint64, CompilerValue*>>* = nullptr);

	bool HasDefaultDataAST() const { return m_DefaultValueAST.size() > 0; }

	void SetClassMemberDefaultAST(x_int32 memberIndex, Unique<ASTBase>& ast);

	const V_Array<Pair<STDString, Share<CompilerValue>>>& GetClassMemberData() const { return m_Data; }

	//const CompilerValue* GetMemberValue(uint64 index);

	static void ParseIntermediateClass(HAZE_IFSTREAM& stream, CompilerModule* m, V_Array<CompilerClass*>& parents);

	void GenClassData_I_Code(HAZE_STRING_STREAM& hss);

	void GenClassFunction_I_Code(HAZE_STRING_STREAM& hss);

	x_uint32 GetDataSize() { return m_DataSize; }

	x_uint32 GetMemberCount() const { return m_MemberCount; }

	x_uint32 GetAlignSize();

	x_uint32 GetOffset(x_uint32 index, Share<CompilerValue> member);

	//int GetClassInheritLevel() const;

	V_Array<Share<CompilerValue>> CreateVariableCopyClassMember(CompilerModule* compilerModule, HazeVariableScope scope);

	bool IsParentClass(CompilerClass* c) const;

	bool IsParentClass(const HazeVariableType type) const;

	static bool HasCommomInheritClass(CompilerClass* c1, CompilerClass* c2);

private:
	void MemoryAlign(x_uint32 memberNum);
	void InitMemberData();

	void GenClassData_I_CodeToHss(HAZE_STRING_STREAM& hss, x_uint32& offset);

private:
	void ResolveClassParent(V_Array<CompilerClass*>&& parentClass);
	void ResolveClassData(V_Array<Pair<STDString, Share<CompilerValue>>>&& data);

private:
	CompilerModule* m_Module;
	V_Array<CompilerClass*> m_ParentClass;

	STDString m_Name;
	x_uint32 m_DataSize;
	x_uint32 m_MemberCount;

	V_Array<Pair<STDString, Share<CompilerValue>>> m_Data;
	V_Array<Pair<x_int32, Unique<ASTBase>>> m_DefaultValueAST;

	V_Array<Share<CompilerFunction>> m_Functions;
	HashMap<STDString, unsigned int> m_HashMap_Functions;

	V_Array<x_uint32> m_Offsets;

	x_uint32 m_TypeId;
};
