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

	HazeCompilerClass(HazeCompilerModule* Module, const HAZE_STRING& Name, std::vector<std::pair<HazeDataDesc, std::vector<HazeDefineVariable*>>>& Data);

	~HazeCompilerClass();

	std::shared_ptr<HazeCompilerFunction> FindFunction(const HAZE_STRING& FunctionName);

	std::shared_ptr<HazeCompilerFunction> AddFunction(std::shared_ptr<HazeCompilerFunction>& Function);

	size_t GetFunctionSize() { return Vector_Function.size(); }

	void InitThisValue();

	std::shared_ptr<HazeCompilerPointerValue> GetThisPointerValue() { return ThisPointerValue; }

	const HAZE_STRING& GetName() { return Name; }

	size_t GetMemberIndex(const HAZE_STRING& MemberName);

	void GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	const std::vector<std::pair<HazeDataDesc, std::vector<HazeDefineVariable>>>& GetClassMemberData() const { return Vector_Data; }

	const HazeDefineVariable* GetClassMemberData(const HAZE_STRING& MemberName) const;

	//bool GetDataName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	void GenClassData_I_Code(HAZE_OFSTREAM& OFStream);

	void GenClassFunction_I_Code(HAZE_OFSTREAM& OFStream);

	unsigned int GetDataSize();

private:
	HazeCompilerModule* Module;

	HAZE_STRING Name;

	unsigned int DataSize;

	std::shared_ptr<HazeCompilerPointerValue> ThisPointerValue;
	std::shared_ptr<HazeCompilerClassValue> ThisClassValue;		//所有同类对象只想此同一个this Value

	std::vector<std::pair<HazeDataDesc, std::vector<HazeDefineVariable>>> Vector_Data;

	std::vector<std::shared_ptr<HazeCompilerFunction>> Vector_Function;
	std::unordered_map<HAZE_STRING, unsigned int> HashMap_Function;
};
