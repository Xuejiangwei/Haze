#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "Haze.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunctionStack.h"

class HazeCompilerModule;

class HazeCompilerFunction
{
public:
	HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& VectorParam);
	~HazeCompilerFunction();

	std::shared_ptr<HazeCompilerValue> AddLocalVariable(const HazeDefineVariable& Variable);

	bool GeneratorOpCode();

private:
	HazeCompilerModule* Module;

	HAZE_STRING Name;
	HazeDefineType Type;

	std::vector<std::unique_ptr<HazeCompilerValue>> VectorParam; //���ҵ���������

	//ÿ����ʱ�������洢���������������Ĵ���
	std::unordered_map<HAZE_STRING, std::shared_ptr<HazeCompilerValue>> MapLocalVariable;

	HazeCompilerFunctionStack StackFrame;
};
