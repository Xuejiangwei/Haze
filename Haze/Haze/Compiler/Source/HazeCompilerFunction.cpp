#include "HazeLog.h"

#include "HazeCompilerHelper.h"
#include "HazeCompilerFunction.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
	: Module(Module), Name(Name), Type(Type)
{
	for (auto& it : Param)
	{
		AddFunctionParam(it);
	}
}

HazeCompilerFunction::~HazeCompilerFunction()
{
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::CreateLocalVariable(const HazeDefineVariable& Variable)
{
	for (auto& it : VectorLocalVariable)
	{
		if (it.first == Variable.second)
		{
			HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加局部变量重复"));
			return nullptr;
		}
	}

	VectorLocalVariable.push_back({ Variable.second, std::make_shared<HazeCompilerValue>(Module, Variable.first) });

	return VectorLocalVariable.back().second;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(HAZE_STRING& Name)
{
	for (auto& it : VectorLocalVariable)
	{
		if (it.first == Name)
		{
			return it.second;
		}
	}

	return nullptr;
}

bool HazeCompilerFunction::GenASSCode(HazeCompilerModule* Module)
{
	//生成函数label
	//FS << HAZE_TEXT("label_") <<Name << std::endl;

	//Push所有参数，从右到左
	for (int i = (int)VectorParam.size() - 1; i >= 0; i--)
	{
		PushAssCode(Module, VectorParam[i].second.get());
	}

	//Push返回地址
	//FS << "Push " << -1 << std::endl;

	//Push临时变量
	//FS << "Push " << -2 << std::endl;

	return false;
}

bool HazeCompilerFunction::GeneratorOpCode(HazeCompilerModule* Module)
{
	//生成函数label

	//Push所有参数，从右到左
	for (auto& it : VectorParam)
	{
		PushAssCode(Module, it.second.get());
	}

	//Push返回地址
	//FS << "Push " << -1 << std::endl;

	//Push临时变量
	//FS << "Push " << -2 << std::endl;

	return true;
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& Variable)
{
	VectorParam.push_back({ Variable.second, std::make_shared<HazeCompilerValue>(Module, Variable.first) });
}