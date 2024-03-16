#pragma once

#include "HazeHeader.h"

class HazeCompilerModule;
class HazeCompilerFunction;
class HazeCompilerValue;
class HazeCompilerPointerValue;
class HazeCompilerClassValue;

class HazeCompilerClass
{
public:
	friend class HazeCompiler;

	HazeCompilerClass(HazeCompilerModule* compilerModule, const HAZE_STRING& name, std::vector<HazeCompilerClass*>& parentClass,
		std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& data);

	~HazeCompilerClass();

	std::shared_ptr<HazeCompilerFunction> FindFunction(const HAZE_STRING& m_FunctionName);

	std::shared_ptr<HazeCompilerFunction> AddFunction(std::shared_ptr<HazeCompilerFunction>& function);

	uint64 GetFunctionNum() { return m_Functions.size(); }

	void InitThisValue();

	std::shared_ptr<HazeCompilerPointerValue> GetThisPointerValue() { return m_ThisPointerValue; }

	std::shared_ptr<HazeCompilerClassValue> GetThisPointerToValue() { return m_ThisClassValue; }

	std::shared_ptr<HazeCompilerClassValue> GetNewPointerToValue() { return m_NewPointerToValue; }

	const HAZE_STRING& GetName() { return m_Name; }

	int GetMemberIndex(const HAZE_STRING& memberName);

	bool GetMemberName(const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName);

	bool GetMemberName(const HazeCompilerValue* value, HAZE_STRING& outName);

	bool GetMemberName(HazeCompilerClassValue* classValue, const HazeCompilerValue* value, HAZE_STRING& outName);

	const std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& GetClassMemberData() const { return m_Data; }

	void GenClassData_I_Code(HAZE_STRING_STREAM& hss);

	void GenClassFunction_I_Code(HAZE_STRING_STREAM& hss);

	uint32 GetDataSize();

	uint32 GetAlignSize();

	uint32 GetOffset(uint32 index, std::shared_ptr<HazeCompilerValue> member);

	int GetClassInheritLevel() const;

private:
	void MemoryAlign(uint32 memberNum);

private:
	HazeCompilerModule* m_Module;

	HAZE_STRING m_Name;

	uint32 m_DataSize;

	std::vector<HazeCompilerClass*> m_ParentClass;

	std::shared_ptr<HazeCompilerPointerValue> m_ThisPointerValue;
	std::shared_ptr<HazeCompilerClassValue> m_ThisClassValue;			//所有同类对象指向此同一个this Value

	std::shared_ptr<HazeCompilerClassValue> m_NewPointerToValue;		//所有New出来的对象指向此Value

	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>> m_Data;

	std::vector<std::shared_ptr<HazeCompilerFunction>> m_Functions;
	std::unordered_map<HAZE_STRING, unsigned int> m_HashMap_Functions;

	std::vector<uint32> m_Offsets;
};
