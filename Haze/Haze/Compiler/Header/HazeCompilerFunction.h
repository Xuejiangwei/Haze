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

	HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param);
	~HazeCompilerFunction();


	std::shared_ptr<HazeCompilerValue> GetLocalVariable(HAZE_STRING& Name);

	bool GenASSCode(HazeCompilerModule* Module);

	bool GeneratorOpCode(HazeCompilerModule* Module);

private:
	void AddFunctionParam(const HazeDefineVariable& Variable);

	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(const HazeDefineVariable& Variable);

private:
	HazeCompilerModule* Module;

	HAZE_STRING Name;
	HazeDefineType Type;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorParam; //从右到左加入参数

	//每个临时变量都存储起来，当作许多个寄存器
	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorLocalVariable;

	HazeCompilerFunctionStack StackFrame;
};
