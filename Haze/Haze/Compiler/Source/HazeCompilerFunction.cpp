#include "HazeLog.h"

#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param, HazeCompilerClass* Class)
	: Module(Module), Name(Name), Type(Type), OwnerClass(Class)
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

void HazeCompilerFunction::CreateNew(const HazeDefineType& Data)
{
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::NEW) << " " << (unsigned int)Data.PrimaryType << " " <<Data.CustomName << std::endl;
	auto BB = BBList.front();
	BB->PushIRCode(SStream.str());
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(const HAZE_STRING& VariableName)
{
	std::shared_ptr<HazeCompilerValue> Ret = nullptr;
	for (auto& iter : BBList)
	{
		for (auto& Value : iter->GetAllocaList())
		{
			if (Value.first == VariableName)
			{
				Ret = Value.second;
			}
			else if (Value.second->GetValue().Type == HazeValueType::Class)
			{
				HAZE_STRING ClassObjectName = VariableName.substr(0, Value.first.size());
				if (Value.first == ClassObjectName)
				{
					HAZE_STRING MemberName = VariableName.substr(Value.first.size() + 1);

					auto ClassVlaue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value.second);
					Ret = ClassVlaue->GetMember(MemberName);
				}
			}
		}
	}

	if (!Ret && OwnerClass)
	{
		Ret = OwnerClass->GetThisValue()->GetMember(VariableName);
	}

	/*if (MemberName)
	{
		HazeDefineData DefineData;
		Ret->SetUseClassMember(0, DefineData);
	}*/

	return Ret;
}

void HazeCompilerFunction::FunctionFinish()
{
	if (Type.PrimaryType == HazeValueType::Void || Name == HAZE_MAIN_FUNCTION_TEXT)
	{
		HAZE_STRING_STREAM SStream;
		SStream << GetInstructionString(InstructionOpCode::RET) << " " << HAZE_CAST_VALUE_TYPE(HazeValueType::Void) << std::endl;
		GetBaseBlockList().front()->PushIRCode(SStream.str());
	}
}

void HazeCompilerFunction::GenI_Code(HAZE_OFSTREAM& OFStream)
{
#if HAZE_I_CODE_ENABLE
	OFStream << GetFunctionLabelHeader() << " " << Name << " " << HAZE_CAST_VALUE_TYPE(Type.PrimaryType);
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
		OFStream << GetFunctionParamHeader() << " " << VectorParam[i].first << " " << HAZE_CAST_VALUE_TYPE(VectorParam[i].second->GetValue().Type);

		if (VectorParam[i].second->IsPointer())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(VectorParam[i].second);
			if (PointerValue->IsPointerBase())
			{
				OFStream << " " << HAZE_CAST_VALUE_TYPE(PointerValue->GetPointerType().PrimaryType);
			}
			else if(PointerValue->IsPointerClass())
			{
				OFStream << " " << PointerValue->GetPointerType().CustomName;
			}
		}
		else if (VectorParam[i].second->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(VectorParam[i].second);
			OFStream << " " << ClassValue->GetOwnerClass()->GetName();
		}

		OFStream << std::endl;
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
			else if (it.second->GetValue().Type == HazeValueType::PointerClass && Value->IsClassMember())
			{
				auto Pointer = std::dynamic_pointer_cast<HazeCompilerPointerValue>(it.second);
				if ((void*)Pointer->GetPointerValue() != OwnerClass)
				{
					auto Class = dynamic_cast<HazeCompilerClassValue*>(Pointer->GetPointerValue());
					Class->GetMemberName(Value, OutName);
					if (!OutName.empty())
					{
						OutName = it.first + HAZE_CLASS_POINTER_ATTR + OutName;
						return true;
					}
				}
			}
			else if (it.second->GetValue().Type == HazeValueType::Class && Value->IsClassMember())
			{
				auto Class = std::dynamic_pointer_cast<HazeCompilerClassValue>(it.second);
				Class->GetMemberName(Value, OutName);
				if (!OutName.empty())
				{
					OutName = it.first + HAZE_CLASS_ATTR + OutName;
					if (!Value->IsClassPublicMember())
					{
						HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("can not access non public member data %s\n"), OutName.c_str());
					}
					return true;
				}
			}
		}
	}

	if (OwnerClass)
	{
		OwnerClass->GetMemberName(Value, OutName);
		return true;
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
	VectorParam.push_back({ Variable.Name, CreateVariable(Module, Variable, HazeDataDesc::Local) });
}