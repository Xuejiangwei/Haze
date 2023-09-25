#include "HazeLog.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, const HAZE_STRING& m_Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param, HazeCompilerClass* Class)
	: Module(Module), m_Name(m_Name), Type(Type), OwnerClass(Class), CurrBlockCount(0), CurrVariableCount(0), m_StartLine(0), m_EndLine(0)
{
	for (int i = (int)Param.size() - 1; i >= 0; i--)
	{
		AddFunctionParam(Param[i]);
	}
}

HazeCompilerFunction::~HazeCompilerFunction()
{
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::CreateLocalVariable(const HazeDefineVariable& Variable, int Line, std::shared_ptr<HazeCompilerValue> RefValue,
	std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize, std::vector<HazeDefineType>* Vector_Param)
{
	auto BB = Module->GetCompiler()->GetInsertBlock();
	return BB->CreateAlloce(Variable, Line, ++CurrVariableCount, RefValue, m_ArraySize, Vector_Param);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::CreateNew(const HazeDefineType& Data)
{
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::NEW) << " " << (unsigned int)Data.PrimaryType;
	if (!Data.CustomName.empty())
	{
		SStream << " " << Data.CustomName;
	}

	SStream<< " "<< CAST_SCOPE(HazeVariableScope::Local) << " " << CAST_DESC(HazeDataDesc::RegisterNew) << std::endl;
	
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
		for (auto& m_Value : CurrBlock->GetAllocaList())
		{
			if (m_Value.first == VariableName)
			{
				Ret = m_Value.second;
				break;
			}
			else if (m_Value.second->GetValueType().PrimaryType == HazeValueType::Class)
			{
				auto MemberValue = GetObjectMember(Module, VariableName);
				if (MemberValue)
				{
					Ret = MemberValue;
					break;
				}
			}
			else if (m_Value.second->GetValueType().PrimaryType == HazeValueType::PointerClass)
			{
				auto MemberValue = GetObjectMember(Module, VariableName);
				if (MemberValue)
				{
					Ret = MemberValue;
					break;
				}
			}
		}

		if (Ret)
		{
			break;
		}
		CurrBlock = CurrBlock->GetParentBlock();
	}

	if (!Ret && OwnerClass)
	{
		Ret = std::dynamic_pointer_cast<HazeCompilerClassValue>(OwnerClass->GetThisPointerToValue())->GetMember(VariableName);
	}

	return Ret;
}

void HazeCompilerFunction::FunctionFinish()
{
	if (Type.PrimaryType == HazeValueType::Void || m_Name == HAZE_MAIN_FUNCTION_TEXT)
	{
		HAZE_STRING_STREAM SStream;
		SStream << GetInstructionString(InstructionOpCode::RET) << " " << HAZE_TEXT("Void") << " " << CAST_SCOPE(HazeVariableScope::None) << " "
			<< CAST_DESC(HazeDataDesc::None) << " " << CAST_TYPE(HazeValueType::Void) << std::endl;
		Module->GetCompiler()->GetInsertBlock()->PushIRCode(SStream.str());
	}
}

void HazeCompilerFunction::GenI_Code(HAZE_STRING_STREAM& SStream)
{
#if HAZE_I_CODE_ENABLE
	SStream << GetFunctionLabelHeader() << " " << m_Name << " ";

	if (!Type.StringStreamTo(SStream))
	{
		HAZE_LOG_ERR(HAZE_TEXT("����<%s>���ͽ���ʧ��,�����м�������!\n"), m_Name.c_str());
		return;
	}

	SStream << std::endl;

	//Push���в��������ҵ���, push �����뷵�ص�ַ������function callȥ��
	for (int i = (int)VectorParam.size() - 1; i >= 0; i--)
	{
		SStream << GetFunctionParamHeader() << " " << VectorParam[i].first << " ";

		if (!VectorParam[i].second->GetValueType().StringStreamTo(SStream))
		{
			HAZE_LOG_ERR(HAZE_TEXT("����<%s>�Ĳ���<%s>���ͽ���ʧ��,�����м�������!\n"), m_Name.c_str(), VectorParam[i].first.c_str());
			return;
		}

		SStream << std::endl;
	}

	HAZE_STRING LocalVariableName;
	int Size = -HAZE_ADDRESS_SIZE;

	for (int i = (int)VectorParam.size() - 1; i >= 0; i--)
	{
		FindLocalVariableName(Vector_LocalVariable[i].first, LocalVariableName);
		SStream << HAZE_LOCAL_VARIABLE_HEADER << " " << LocalVariableName;
		HazeCompilerStream(SStream, Vector_LocalVariable[i].first, false);

		Size -= Vector_LocalVariable[i].first->GetSize();
		SStream << " " << Size << " " << Vector_LocalVariable[i].first->GetSize() << " " << Vector_LocalVariable[i].second << std::endl;
	}

	Size = 0;

	for (size_t i = VectorParam.size(); i < Vector_LocalVariable.size(); i++)
	{
		FindLocalVariableName(Vector_LocalVariable[i].first, LocalVariableName);
		SStream << HAZE_LOCAL_VARIABLE_HEADER << " " << LocalVariableName;
		HazeCompilerStream(SStream, Vector_LocalVariable[i].first, false);
		SStream << " " << Size << " " << Vector_LocalVariable[i].first->GetSize() << " " << Vector_LocalVariable[i].second << std::endl;
		Size += Vector_LocalVariable[i].first->GetSize();
	}

	SStream << GetFunctionStartHeader() << " " << m_StartLine << std::endl;

	EntryBlock->GenI_Code(SStream);

	SStream << std::endl << GetFunctionEndHeader() << " " << m_EndLine << std::endl << std::endl;

	EntryBlock->ClearLocalVariable();

#endif // HAZE_ASS_ENABLE
}

HAZE_STRING HazeCompilerFunction::GenDafaultBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_DEFAULT << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenIfThenBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_IF_THEN << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenElseBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_ELSE << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenLoopBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_LOOP << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenWhileBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_WHILE << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenForBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_FOR << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenForConditionBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_FOR_CONDITION << ++CurrBlockCount;
	return HSS.str();
}

HAZE_STRING HazeCompilerFunction::GenForStepBlockName()
{
	HAZE_STRING_STREAM HSS;
	HSS << BLOCK_FOR_STEP << ++CurrBlockCount;
	return HSS.str();
}

bool HazeCompilerFunction::FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& m_Value, HAZE_STRING& OutName)
{
	if (EntryBlock->FindLocalVariableName(m_Value, OutName))
	{
		return true;
	}
	else if (OwnerClass)
	{
		return OwnerClass->GetMemberName(m_Value, OutName);
	}

	return false;
}

bool HazeCompilerFunction::FindLocalVariableName(const HazeCompilerValue* m_Value, HAZE_STRING& OutName)
{
	if (EntryBlock->FindLocalVariableName(m_Value, OutName))
	{
		return true;
	}
	else if (OwnerClass)
	{
		return OwnerClass->GetMemberName(m_Value, OutName);
	}

	return false;
}

void HazeCompilerFunction::AddLocalVariable(std::shared_ptr<HazeCompilerValue> m_Value, int Line)
{
	Vector_LocalVariable.push_back({ m_Value, Line });
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& Variable)
{
	VectorParam.push_back({ Variable.m_Name, CreateVariable(Module, Variable, HazeVariableScope::Local, HazeDataDesc::None, 0) });
}