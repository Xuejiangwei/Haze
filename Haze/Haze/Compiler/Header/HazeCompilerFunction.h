#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <fstream>

#include "Haze.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunctionStack.h"

class HazeCompilerModule;

class HazeCompilerFunction
{
public:
	friend class HazeCompiler;
	friend class HazeCompilerModule;

	HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);
	~HazeCompilerFunction();

	std::shared_ptr<HazeCompilerValue> GetReturnValue() { return ReturnValue; }

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& Name);

	const HAZE_STRING& GetName() const { return Name; }
	
	void FunctionFinish();

	void GenCode(HazeCompilerModule* Module);

private:
	void AddFunctionParam(const HazeDefineVariable& Variable);

	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(const HazeDefineVariable& Variable);

	void GenASSCode(HazeCompilerModule* Module);

	void GenOpCode(HazeCompilerModule* Module);

private:
	HazeCompilerModule* Module;

	HAZE_STRING FunctionASSCode;

	HAZE_STRING Name;
	HazeDefineType Type;

	std::shared_ptr<HazeCompilerValue> ReturnValue;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorParam; //从右到左加入参数

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorLocalVariable; //每个临时变量都存储起来，当作许多个寄存器

	HazeCompilerFunctionStack StackFrame;
};
