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

	HazeCompilerClass(HazeCompilerModule* Module, const HAZE_STRING& Name, std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& Data);

	~HazeCompilerClass();

	std::shared_ptr<HazeCompilerFunction> FindFunction(const HAZE_STRING& FunctionName);

	std::shared_ptr<HazeCompilerFunction> AddFunction(std::shared_ptr<HazeCompilerFunction>& Function);

	uint64 GetFunctionSize() { return Vector_Function.size(); }

	void InitThisValue();

	std::shared_ptr<HazeCompilerPointerValue> GetThisPointerValue() { return ThisPointerValue; }

	std::shared_ptr<HazeCompilerClassValue> GetThisPointerToValue() { return ThisClassValue; }

	std::shared_ptr<HazeCompilerClassValue> GetNewPointerToValue() { return NewPointerToValue; }

	const HAZE_STRING& GetName() { return Name; }

	uint64 GetMemberIndex(const HAZE_STRING& MemberName);

	bool GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	bool GetMemberName(const HazeCompilerValue* Value, HAZE_STRING& OutName);

	bool GetMemberName(HazeCompilerClassValue* ClassValue, const HazeCompilerValue* Value, HAZE_STRING& OutName);

	const std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& GetClassMemberData() const { return Vector_Data; }

	//const HazeDefineVariable* GetClassMemberData(const HAZE_STRING& MemberName) const;

	//bool GetDataName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	void GenClassData_I_Code(HAZE_STRING_STREAM& SStream);

	void GenClassFunction_I_Code(HAZE_STRING_STREAM& SStream);

	uint32 GetDataSize();

	uint32 GetAlignSize();

	uint32 GetOffset(uint32 Index, std::shared_ptr<HazeCompilerValue> Member);

private:
	void MemoryAlign(uint32 MemberNum);

private:
	HazeCompilerModule* Module;

	HAZE_STRING Name;

	uint32 DataSize;

	std::shared_ptr<HazeCompilerPointerValue> ThisPointerValue;
	std::shared_ptr<HazeCompilerClassValue> ThisClassValue;		//所有同类对象指向此同一个this Value

	std::shared_ptr<HazeCompilerClassValue> NewPointerToValue;		//所有New出来的对象指向此Value

	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>> Vector_Data;

	std::vector<std::shared_ptr<HazeCompilerFunction>> Vector_Function;
	std::unordered_map<HAZE_STRING, unsigned int> HashMap_Function;

	std::vector<uint32> Vector_Offset;
};
