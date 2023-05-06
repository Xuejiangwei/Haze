#include "HazeLog.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param, HazeCompilerClass* Class)
	: Module(Module), Name(Name), Type(Type), OwnerClass(Class)
{
	for (int i = (int)Param.size() - 1; i >= 0; i--)
	{
		AddFunctionParam(Param[i]);
	}

	CurrBlockCount = 0;
	CurrVariableCount = 0;
}

HazeCompilerFunction::~HazeCompilerFunction()
{
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::CreateLocalVariable(const HazeDefineVariable& Variable, std::shared_ptr<HazeCompilerValue> ArraySizeOrRef, std::vector<HazeDefineType>* Vector_Param)
{
	auto BB = Module->GetCompiler()->GetInsertBlock();
	return BB->CreateAlloce(Variable, ++CurrVariableCount, ArraySizeOrRef, Vector_Param);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::CreateNew(const HazeDefineType& Data)
{
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::NEW) << " " << (unsigned int)Data.PrimaryType << " " << Data.CustomName << std::endl;
	auto BB = Module->GetCompiler()->GetInsertBlock();
	BB->PushIRCode(SStream.str());

	return Module->GetCompiler()->GetNewRegister(Module, Data);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(const HAZE_STRING& VariableName)
{
	std::shared_ptr<HazeCompilerValue> Ret = nullptr;

	auto CurrBlock = Module->GetCompiler()->GetInsertBlock().get();
	while (CurrBlock)
	{
		for (auto& Value : CurrBlock->GetAllocaList())
		{
			if (Value.first == VariableName)
			{
				Ret = Value.second;
				break;
			}
			else if (Value.second->GetValueType().PrimaryType == HazeValueType::Class)
			{
				auto MemberValue = GetObjectMember(Module, VariableName);
				if (MemberValue)
				{
					Ret = MemberValue;
					break;
				}
			}
			else if (Value.second->GetValueType().PrimaryType == HazeValueType::PointerClass)
			{
				auto MemberValue = GetObjectMember(Module, VariableName);
				if (MemberValue)
				{
					Ret = MemberValue;
					break;
				}
			}
		}

		CurrBlock = CurrBlock->GetParentBlock();
	}


	if (!Ret && OwnerClass)
	{
		Ret = GetObjectMember(GetModule(), VariableName);
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
		EntryBlock->PushIRCode(SStream.str());
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
		OFStream << GetFunctionParamHeader() << " " << VectorParam[i].first << " " << HAZE_CAST_VALUE_TYPE(VectorParam[i].second->GetValueType().PrimaryType);

		if (VectorParam[i].second->IsPointer())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(VectorParam[i].second);
			if (PointerValue->IsPointerBase())
			{
				OFStream << " " << HAZE_CAST_VALUE_TYPE(PointerValue->GetValueType().PrimaryType);
			}
			else if(PointerValue->IsPointerClass())
			{
				OFStream << " " << PointerValue->GetValueType().CustomName;
			}
		}
		else if (VectorParam[i].second->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(VectorParam[i].second);
			OFStream << " " << ClassValue->GetOwnerClassName();
		}

		OFStream << std::endl;
	}


	/*size_t ParamCount = VectorParam.size();
	for (auto& it : BBList)
	{
		bool WriteBlockName = false;
		for (auto& code : it->GetIRCode()) 
		{
			if (!WriteBlockName)
			{
				WriteBlockName = true;
				OFStream << code;
			}
			else if (ParamCount > 0)
			{
				ParamCount--;
			}
			else
			{
				OFStream << code;
			}
		}
	}*/

	HAZE_STRING LocalVariableName;
	int Size = -HAZE_ADDRESS_SIZE;

	for (int i = (int)VectorParam.size() - 1; i >= 0; i--)
	{
		FindLocalVariableName(Vector_LocalVariable[i], LocalVariableName);
		OFStream << HAZE_LOCAL_VARIABLE_HEADER << " " << LocalVariableName;
		HazeCompilerOFStream(OFStream, Vector_LocalVariable[i]);

		Size -= Vector_LocalVariable[i]->GetSize();
		OFStream << " " << Size << " " << Vector_LocalVariable[i]->GetSize() << std::endl;
	}

	Size = 0;

	for (size_t i = VectorParam.size(); i < Vector_LocalVariable.size(); i++)
	{
		FindLocalVariableName(Vector_LocalVariable[i], LocalVariableName);
		OFStream << HAZE_LOCAL_VARIABLE_HEADER << " " << LocalVariableName;
		HazeCompilerOFStream(OFStream, Vector_LocalVariable[i]);
		OFStream << " " << Size << " " << Vector_LocalVariable[i]->GetSize() << std::endl;
		Size += Vector_LocalVariable[i]->GetSize();
	}

	OFStream << GetFunctionStartHeader() << std::endl;

	EntryBlock->GenI_Code(OFStream);

	OFStream << GetFunctionEndHeader() << std::endl << std::endl;
#endif // HAZE_ASS_ENABLE
}

HAZE_STRING HazeCompilerFunction::GenIfBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << HAZE_TEXT("IfBlock") << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenElseBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << HAZE_TEXT("ElseBlock") << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenWhileBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << HAZE_TEXT("WhileBlock") << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenForBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << HAZE_TEXT("ForBlock") << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenForConditionBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << HAZE_TEXT("ForConditionBlock") << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenForEndBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << HAZE_TEXT("ForBlockEnd") << ++CurrBlockCount;
	return HSS.str();
}

//std::shared_ptr<HazeBaseBlock> HazeCompilerFunction::GetTopBaseBlock()
//{
//	for (auto Iter = BBList.rbegin(); Iter != BBList.rend(); Iter++)
//	{
//		if (!(*Iter)->BlockIsFinish())
//		{
//			return *Iter;
//		}
//	}
//
//	return nullptr;
//}

bool HazeCompilerFunction::FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	if (EntryBlock->FindLocalVariableName(Value, OutName))
	{
		return true;
	}
	else if (OwnerClass)
	{
		return OwnerClass->GetMemberName(Value, OutName);
	}

	return false;
}

bool HazeCompilerFunction::FindLocalVariableName(const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	if (EntryBlock->FindLocalVariableName(Value, OutName))
	{
		return true;
	}
	else if (OwnerClass)
	{
		return OwnerClass->GetMemberName(Value, OutName);
	}

	return false;
}

//bool HazeCompilerFunction::GetFunctionParamNameByIndex(unsigned int Index, HAZE_STRING& OutName)
//{
//	if (BBList.size() > 0)
//	{
//		if (BBList.front()->GetAllocaList().size() > Index)
//		{
//			OutName = BBList.front()->GetAllocaList()[Index].first;
//			return true;
//		}
//	}
//
//	return false;
//}

void HazeCompilerFunction::AddLocalVariable(std::shared_ptr<HazeCompilerValue> Value)
{
	Vector_LocalVariable.push_back(Value);
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& Variable)
{
	VectorParam.push_back({ Variable.Name, CreateVariable(Module, Variable, HazeDataDesc::Local, 0) });
}