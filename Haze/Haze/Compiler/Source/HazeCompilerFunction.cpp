#include "HazeLog.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* m_Module, const HAZE_STRING& m_Name, HazeDefineType& m_Type, std::vector<HazeDefineVariable>& Param, HazeCompilerClass* Class)
	: m_Module(m_Module), m_Name(m_Name), m_Type(m_Type), OwnerClass(Class), CurrBlockCount(0), CurrVariableCount(0), m_StartLine(0), m_EndLine(0)
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
	std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize, std::vector<HazeDefineType>* Params)
{
	auto BB = m_Module->GetCompiler()->GetInsertBlock();
	return BB->CreateAlloce(Variable, Line, ++CurrVariableCount, RefValue, m_ArraySize, Params);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::CreateNew(const HazeDefineType& m_Data)
{
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::NEW) << " " << (unsigned int)m_Data.PrimaryType;
	if (!m_Data.CustomName.empty())
	{
		SStream << " " << m_Data.CustomName;
	}

	SStream<< " "<< CAST_SCOPE(HazeVariableScope::Local) << " " << CAST_DESC(HazeDataDesc::RegisterNew) << std::endl;
	
	auto BB = m_Module->GetCompiler()->GetInsertBlock();
	BB->PushIRCode(SStream.str());

	return m_Module->GetCompiler()->GetNewRegister(m_Module, m_Data);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(const HAZE_STRING& VariableName)
{
	std::shared_ptr<HazeCompilerValue> Ret = nullptr;

	auto CurrBlock = m_Module->GetCompiler()->GetInsertBlock().get();
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
				auto MemberValue = GetObjectMember(m_Module, VariableName);
				if (MemberValue)
				{
					Ret = MemberValue;
					break;
				}
			}
			else if (Value.second->GetValueType().PrimaryType == HazeValueType::PointerClass)
			{
				auto MemberValue = GetObjectMember(m_Module, VariableName);
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
	if (m_Type.PrimaryType == HazeValueType::Void || m_Name == HAZE_MAIN_FUNCTION_TEXT)
	{
		HAZE_STRING_STREAM SStream;
		SStream << GetInstructionString(InstructionOpCode::RET) << " " << HAZE_TEXT("Void") << " " << CAST_SCOPE(HazeVariableScope::None) << " "
			<< CAST_DESC(HazeDataDesc::None) << " " << CAST_TYPE(HazeValueType::Void) << std::endl;
		m_Module->GetCompiler()->GetInsertBlock()->PushIRCode(SStream.str());
	}
}

void HazeCompilerFunction::GenI_Code(HAZE_STRING_STREAM& SStream)
{
#if HAZE_I_CODE_ENABLE
	SStream << GetFunctionLabelHeader() << " " << m_Name << " ";

	if (!m_Type.StringStreamTo(SStream))
	{
		HAZE_LOG_ERR(HAZE_TEXT("函数<%s>类型解析失败,生成中间代码错误!\n"), m_Name.c_str());
		return;
	}

	SStream << std::endl;

	//Push所有参数，从右到左, push 参数与返回地址的事由function call去做
	for (int i = (int)VectorParam.size() - 1; i >= 0; i--)
	{
		SStream << GetFunctionParamHeader() << " " << VectorParam[i].first << " ";

		if (!VectorParam[i].second->GetValueType().StringStreamTo(SStream))
		{
			HAZE_LOG_ERR(HAZE_TEXT("函数<%s>的参数<%s>类型解析失败,生成中间代码错误!\n"), m_Name.c_str(), VectorParam[i].first.c_str());
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

void HazeCompilerFunction::AddLocalVariable(std::shared_ptr<HazeCompilerValue> Value, int Line)
{
	Vector_LocalVariable.push_back({ Value, Line });
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& Variable)
{
	VectorParam.push_back({ Variable.m_Name, CreateVariable(m_Module, Variable, HazeVariableScope::Local, HazeDataDesc::None, 0) });
}