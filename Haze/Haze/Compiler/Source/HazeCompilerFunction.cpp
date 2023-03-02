#include "HazeLog.h"

#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param)
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
	auto BB = BBList.front();
	return BB->CreateAlloce(Variable);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(const HAZE_STRING& Name)
{
	for (auto& iter : BBList)
	{
		for (auto& Value : iter->GetAllocaList())
		{
			if (Value.first == Name)
			{
				return Value.second;
			}
		}
	}

	return nullptr;
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

	//Push���в��������ҵ���, push �����뷵�ص�ַ������callȥ��
	for (auto& it : VectorParam)
	{
		OFStream << GetFunctionParamHeader() << " " << HAZE_CAST_VALUE_TYPE(it.second->GetValue().Type) << " "<< it.first << std::endl;
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

bool HazeCompilerFunction::GetLocalVariableName(std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& NameOut)
{
	for (auto& iter : BBList)
	{
		for (auto& it : iter->GetAllocaList())
		{
			if (it.second == Value)
			{
				NameOut = it.first;
				return true;
			}
		}
	}

	return false;
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& Variable)
{
	VectorParam.push_back({ Variable.Name, std::make_shared<HazeCompilerValue>(Module, Variable.Type, InstructionScopeType::Local) });
}