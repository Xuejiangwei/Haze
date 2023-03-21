#pragma once

#include "Haze.h"

class HazeCompilerModule;
class HazeCompilerFunction;
class HazeCompilerValue;

class HazeCompilerClass
{
public:
	friend class HazeCompiler;

	HazeCompilerClass(HazeCompilerModule* Module, const HAZE_STRING& Name, std::vector<HazeDefineVariable*>& Data);
	~HazeCompilerClass();

	std::shared_ptr<HazeCompilerFunction> FindFunction(const HAZE_STRING& Name);

	std::shared_ptr<HazeCompilerFunction> AddFunction(std::shared_ptr<HazeCompilerFunction>& Function);

	size_t GetFunctionSize() { return Vector_Function.size(); }

	std::shared_ptr<HazeCompilerValue> GetClassData(const HAZE_STRING& Name);

	bool GetDataName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	void GenClassData_I_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream);

	void GenClassFunction_I_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream);

private:
	HazeCompilerModule* Module;

	HAZE_STRING Name;

	std::unordered_map<HAZE_STRING, unsigned int> HashMap_Data;
	std::vector<std::shared_ptr<HazeCompilerValue>> Vector_Data;

	std::vector<std::shared_ptr<HazeCompilerFunction>> Vector_Function;
	std::unordered_map<HAZE_STRING, unsigned int> HashMap_Function;
};
