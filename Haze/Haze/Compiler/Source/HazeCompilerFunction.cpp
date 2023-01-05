#include "HazeLog.h"

#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
	: Module(Module), Name(Name), Type(Type)
{
	for (auto& it : Param)
	{
		AddFunctionParam(it);
	}

	ReturnValue = std::make_shared<HazeCompilerValue>(Module, Type, HazeCompilerValue::ValueSection::Local);
	
	FunctionASSCode = HAZE_STRING(HAZE_TEXT("Label_")) + Name + HAZE_TEXT("\n");
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

	VectorLocalVariable.push_back({ Variable.second, std::make_shared<HazeCompilerValue>(Module, Variable.first,HazeCompilerValue::ValueSection::Local, (int)VectorLocalVariable.size()) });

	HAZE_STRING_STREAM SStream;
	HazeCompilerStream(SStream, VectorLocalVariable.back().second.get());

	FunctionASSCode += HAZE_TEXT("Push ") + SStream.str() + HAZE_TEXT(" \n");

	return VectorLocalVariable.back().second;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(const HAZE_STRING& Name)
{
	for (auto& it : VectorParam)
	{
		if (it.first == Name)
		{
			return it.second;
		}
	}

	for (auto& it : VectorLocalVariable)
	{
		if (it.first == Name)
		{
			return it.second;
		}
	}

	return nullptr;
}


void HazeCompilerFunction::FunctionFinish()
{
}

void HazeCompilerFunction::GenCode(HazeCompilerModule* Module)
{
#if HAZE_ASS_ENABLE
	GenASSCode(Module);
#endif // HAZE_ASS_ENABLE

#ifdef HAZE_OP_CODE_ENABLE
	GenOpCode(Module);
#endif // HAZE_OP_CODE_ENABLE

}

void HazeCompilerFunction::GenASSCode(HazeCompilerModule* Module)
{
	//生成函数label
	Module->GenASS_Label(Name);
}

void HazeCompilerFunction::GenOpCode(HazeCompilerModule* Module)
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
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& Variable)
{
	VectorParam.push_back({ Variable.second, std::make_shared<HazeCompilerValue>(Module, Variable.first,HazeCompilerValue::ValueSection::Local,(int)VectorParam.size()) });
}