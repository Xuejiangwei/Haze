#pragma once

class CompilerModule;
class CompilerFunction;
class CompilerValue;
class HazeCompilerPointerValue;
class CompilerClassValue;

class CompilerClass
{
	friend class Compiler;
public:

	CompilerClass(CompilerModule* compilerModule, const HString& name, V_Array<CompilerClass*>& parentClass,
		V_Array<Pair<HString, Share<CompilerValue>>>& data);

	~CompilerClass();

	Share<CompilerFunction> FindFunction(const HString& functionName, const HString* nameSpace);

	Share<CompilerFunction> AddFunction(Share<CompilerFunction>& function);

	uint64 GetFunctionNum() { return m_Functions.size(); }

	const HString& GetName() { return m_Name; }

	int GetMemberIndex(const HString& memberName, const HString* nameSpace);

	int GetMemberIndex(const V_Array<HString>& classNames, const HString& memberName);

	//int GetMemberIndex(CompilerValue* value);

	//bool GetThisMemberName(const HazeCompilerValue* value, HString& outName, bool getOffset = false, V_Array<uint64>* offsets = nullptr);

	//bool GetMemberName(CompilerClassValue* classValue, const CompilerValue* value, HString& outName, bool getOffset = false, V_Array<Pair<uint64, CompilerValue*>>* = nullptr);

	const V_Array<Pair<HString, Share<CompilerValue>>>& GetClassMemberData() const { return m_Data; }

	//const CompilerValue* GetMemberValue(uint64 index);

	void GenClassData_I_Code(HAZE_STRING_STREAM& hss);

	void GenClassFunction_I_Code(HAZE_STRING_STREAM& hss);

	uint32 GetDataSize() { return m_DataSize; }

	uint32 GetMemberCount() const { return m_MemberCount; }

	uint32 GetAlignSize();

	uint32 GetOffset(uint32 index, Share<CompilerValue> member);

	//int GetClassInheritLevel() const;

	V_Array<Share<CompilerValue>> CreateVariableCopyClassMember(CompilerModule* compilerModule, HazeVariableScope scope);

	bool IsInheritClass(CompilerClass* c) const;

	static bool HasCommomInheritClass(CompilerClass* c1, CompilerClass* c2);

private:
	void MemoryAlign(uint32 memberNum);

	void GenClassData_I_CodeToHss(HAZE_STRING_STREAM& hss, uint32& offset);

private:
	CompilerModule* m_Module;
	V_Array<CompilerClass*> m_ParentClass;

	HString m_Name;
	uint32 m_DataSize;
	uint32 m_MemberCount;

	V_Array<Pair<HString, Share<CompilerValue>>> m_Data;

	V_Array<Share<CompilerFunction>> m_Functions;
	HashMap<HString, unsigned int> m_HashMap_Functions;

	V_Array<uint32> m_Offsets;
};
