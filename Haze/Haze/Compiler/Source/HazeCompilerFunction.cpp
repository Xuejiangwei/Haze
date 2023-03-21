#include "HazeLog.h"

#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param, HazeCompilerClass* Class)
	: Module(Module), Name(Name), Type(Type), Class(Class)
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
	auto BB = BBList.front();
	return BB->CreateAlloce(Variable);
}

void HazeCompilerFunction::CreateNew(const HazeDefineData& Data)
{
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::NEW) << " " << (unsigned int)Data.Type << " " <<Data.CustomName << std::endl;
	auto BB = BBList.front();
	BB->PushIRCode(SStream.str());
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(const HAZE_STRING& Name, const HAZE_STRING* MemberName)
{
	std::shared_ptr<HazeCompilerValue> Ret = nullptr;
	for (auto& iter : BBList)
	{
		for (auto& Value : iter->GetAllocaList())
		{
			if (Value.first == Name)
			{
				Ret = Value.second;
			}
			else if (Value.second->GetValue().Type == HazeValueType::Class)
			{
				HAZE_STRING ClassObjectName = Name.substr(0, Value.first.size());
				if (Value.first == ClassObjectName)
				{
					HAZE_STRING MemberName = Name.substr(Value.first.size() + 1);
					Ret = Class->GetClassData(MemberName);
				}
			}
		}
	}

	if (!Ret && Class)
	{
		Ret = Class->GetClassData(Name);
	}

	if (MemberName)
	{
		HazeDefineData DefineData;
		Ret->SetUseClassMember(0, DefineData);
	}

	return Ret;
}

void HazeCompilerFunction::FunctionFinish()
{
	
}

void HazeCompilerFunction::GenI_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream)
{
#if HAZE_I_CODE_ENABLE
	OFStream << GetFunctionLabelHeader() << " " << Name << " " << HAZE_CAST_VALUE_TYPE(Type.Type);
	/*if (!Type.second.empty())
	{
		OFStream << Type.second;
	}
	else
	{
		OFStream << GetValueTypeByToken(Type.first);
	}*/

	OFStream << std::endl;

	//Push所有参数，从右到左, push 参数与返回地址的事由call去做
	for (int i = (int)VectorParam.size() - 1; i >= 0; i--)
	{
		OFStream << GetFunctionParamHeader() << " " << HAZE_CAST_VALUE_TYPE(VectorParam[i].second->GetValue().Type) << " " << VectorParam[i].first << std::endl;
	}

	OFStream << GetFunctionStartHeader() << std::endl;

	size_t ParamCount = VectorParam.size();
	for (auto& it : BBList)
	{
		for (auto& code : it->GetIRCode()) 
		{
			if (ParamCount > 0)
			{
				ParamCount--;
			}
			else
			{
				OFStream << code;
			}
		}
	}

	OFStream << GetFunctionEndHeader() << std::endl;
#endif // HAZE_ASS_ENABLE
}

bool HazeCompilerFunction::GetLocalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	for (auto& iter : BBList)
	{
		for (auto& it : iter->GetAllocaList())
		{
			if (it.second == Value)
			{
				OutName = it.first;
				return true;
			}
		}
	}

	if (Class)
	{
		return Class->GetDataName(Value, OutName);
	}

	return false;
}

bool HazeCompilerFunction::GetFunctionParamNameByIndex(unsigned int Index, HAZE_STRING& OutName)
{
	if (BBList.size() > 0)
	{
		if (BBList.front()->GetAllocaList().size() > Index)
		{
			OutName = BBList.front()->GetAllocaList()[Index].first;
			return true;
		}
	}

	return false;
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& Variable)
{
	VectorParam.push_back({ Variable.Name, std::make_shared<HazeCompilerValue>(Module, Variable.Type, InstructionScopeType::Local) });
}