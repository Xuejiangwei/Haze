#include "HazeLog.h"

#include "HazeCompilerFunction.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& VectorParam)
	: Module(Module), Name(Name), Type(Type)
{
	for (auto it : VectorParam)
	{
	}
}

HazeCompilerFunction::~HazeCompilerFunction()
{
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::AddLocalVariable(const HazeDefineVariable& Variable)
{
	auto It = MapLocalVariable.find(Variable.second);
	if (It != MapLocalVariable.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加局部变量重复"));
		return nullptr;
	}

	MapLocalVariable[Variable.second] = std::make_shared<HazeCompilerValue>(Module, Variable.first);

	return MapLocalVariable[Variable.second];
}

bool HazeCompilerFunction::GeneratorOpCode()
{
	//生成函数label

	//Push所有参数，从右到左
	for (auto& it : VectorParam)
	{
	}

	//Push返回地址

	//Push临时变量

	return true;
}