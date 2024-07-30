#pragma once

class HazeCompilerModule;
class HazeCompilerFunction;
class HazeCompilerValue;
class HazeCompilerPointerValue;
class HazeCompilerClassValue;

class HazeCompilerClass
{
	friend class HazeCompiler;
public:

	HazeCompilerClass(HazeCompilerModule* compilerModule, const HString& name, V_Array<HazeCompilerClass*>& parentClass,
		V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<HazeCompilerValue>>>>>& data);

	~HazeCompilerClass();

	Share<HazeCompilerFunction> FindFunction(const HString& m_FunctionName);

	Share<HazeCompilerFunction> AddFunction(Share<HazeCompilerFunction>& function);

	uint64 GetFunctionNum() { return m_Functions.size(); }

	void InitThisValue();

	/*Share<HazeCompilerClassValue> GetThisPointerToValue() { return m_ThisClassValue; }

	Share<HazeCompilerClassValue> GetNewPointerToValue() { return m_NewPointerToValue; }*/

	const HString& GetName() { return m_Name; }

	int GetMemberIndex(const HString& memberName);

	//bool GetThisMemberName(const HazeCompilerValue* value, HString& outName, bool getOffset = false, V_Array<uint64>* offsets = nullptr);

	bool GetMemberName(HazeCompilerClassValue* classValue, const HazeCompilerValue* value, HString& outName, bool getOffset = false, V_Array<uint64>* offsets = nullptr);

	const V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<HazeCompilerValue>>>>>& GetClassMemberData() const { return m_Data; }

	void GenClassData_I_Code(HAZE_STRING_STREAM& hss);

	void GenClassFunction_I_Code(HAZE_STRING_STREAM& hss);

	uint32 GetDataSize();

	uint32 GetAlignSize();

	uint32 GetOffset(uint32 index, Share<HazeCompilerValue> member);

	int GetClassInheritLevel() const;

private:
	void MemoryAlign(uint32 memberNum);

private:
	HazeCompilerModule* m_Module;

	HString m_Name;

	uint32 m_DataSize;

	V_Array<HazeCompilerClass*> m_ParentClass;

	//Share<HazeCompilerClassValue> m_ThisClassValue;			//所有同类对象指向此同一个this Value

	//Share<HazeCompilerClassValue> m_NewPointerToValue;		//所有New出来的对象指向此Value

	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<HazeCompilerValue>>>>> m_Data;

	V_Array<Share<HazeCompilerFunction>> m_Functions;
	HashMap<HString, unsigned int> m_HashMap_Functions;

	V_Array<uint32> m_Offsets;
};
