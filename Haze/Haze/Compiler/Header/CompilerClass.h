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
		V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<CompilerValue>>>>>& data);

	~CompilerClass();

	Share<CompilerFunction> FindFunction(const HString& m_FunctionName);

	Share<CompilerFunction> AddFunction(Share<CompilerFunction>& function);

	uint64 GetFunctionNum() { return m_Functions.size(); }

	void InitThisValue();

	/*Share<HazeCompilerClassValue> GetThisPointerToValue() { return m_ThisClassValue; }

	Share<HazeCompilerClassValue> GetNewPointerToValue() { return m_NewPointerToValue; }*/

	const HString& GetName() { return m_Name; }

	int GetMemberIndex(const HString& memberName);

	//bool GetThisMemberName(const HazeCompilerValue* value, HString& outName, bool getOffset = false, V_Array<uint64>* offsets = nullptr);

	bool GetMemberName(CompilerClassValue* classValue, const CompilerValue* value, HString& outName, bool getOffset = false, V_Array<Pair<uint64, CompilerValue*>>* = nullptr);

	const V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<CompilerValue>>>>>& GetClassMemberData() const { return m_Data; }

	const CompilerValue* GetMemberValue(uint64 index);

	void GenClassData_I_Code(HAZE_STRING_STREAM& hss);

	void GenClassFunction_I_Code(HAZE_STRING_STREAM& hss);

	uint32 GetDataSize();

	uint32 GetAlignSize();

	uint32 GetOffset(uint32 index, Share<CompilerValue> member);

	int GetClassInheritLevel() const;

private:
	void MemoryAlign(uint32 memberNum);

private:
	CompilerModule* m_Module;

	HString m_Name;

	uint32 m_DataSize;

	V_Array<CompilerClass*> m_ParentClass;

	//Share<HazeCompilerClassValue> m_ThisClassValue;			//所有同类对象指向此同一个this Value

	//Share<HazeCompilerClassValue> m_NewPointerToValue;		//所有New出来的对象指向此Value

	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<CompilerValue>>>>> m_Data;

	V_Array<Share<CompilerFunction>> m_Functions;
	HashMap<HString, unsigned int> m_HashMap_Functions;

	V_Array<uint32> m_Offsets;
};
