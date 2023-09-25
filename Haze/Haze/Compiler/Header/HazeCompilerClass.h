#pragma once

#include "Haze.h"

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

	std::shared_ptr<HazeCompilerFunction> AddFunction(std::shared_ptr<HazeCompilerFunction>& Function);

	uint64 GetFunctionNum() { return m_Functions.size(); }

	void InitThisValue();

	std::shared_ptr<HazeCompilerPointerValue> GetThisPointerValue() { return ThisPointerValue; }

	std::shared_ptr<HazeCompilerClassValue> GetThisPointerToValue() { return ThisClassValue; }

	std::shared_ptr<HazeCompilerClassValue> GetNewPointerToValue() { return NewPointerToValue; }

	const HAZE_STRING& GetName() { return m_Name; }

	int GetMemberIndex(const HAZE_STRING& MemberName);

	bool GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	bool GetMemberName(const HazeCompilerValue* Value, HAZE_STRING& OutName);

	bool GetMemberName(HazeCompilerClassValue* ClassValue, const HazeCompilerValue* Value, HAZE_STRING& OutName);

	const std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& GetClassMemberData() const { return m_Data; }

	void GenClassData_I_Code(HAZE_STRING_STREAM& SStream);

	void GenClassFunction_I_Code(HAZE_STRING_STREAM& SStream);

	uint32 GetDataSize();

	uint32 GetAlignSize();

	uint32 GetOffset(uint32 Index, std::shared_ptr<HazeCompilerValue> Member);

private:
	void MemoryAlign(uint32 MemberNum);

private:
	HazeCompilerModule* m_Module;

	HAZE_STRING m_Name;

	uint32 m_DataSize;

	std::vector<HazeCompilerClass*> m_ParentClass;

	std::shared_ptr<HazeCompilerPointerValue> ThisPointerValue;
	std::shared_ptr<HazeCompilerClassValue> ThisClassValue;			//所有同类对象指向此同一个this Value

	std::shared_ptr<HazeCompilerClassValue> NewPointerToValue;		//所有New出来的对象指向此Value

	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>> m_Data;

	std::vector<std::shared_ptr<HazeCompilerFunction>> m_Functions;
	std::unordered_map<HAZE_STRING, unsigned int> m_HashMap_Functions;

	std::vector<uint32> Vector_Offset;
};
