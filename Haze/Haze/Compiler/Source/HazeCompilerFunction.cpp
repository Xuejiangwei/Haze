#include "HazePch.h"
#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"
#include "HazeLogDefine.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* compilerModule, const HString& name, 
	HazeDefineType& type, V_Array<HazeDefineVariable>& params, HazeCompilerClass* compilerClass)
	: m_Module(compilerModule), m_Name(name), m_Type(type), m_OwnerClass(compilerClass), m_CurrBlockCount(0), 
		m_CurrVariableCount(0), m_StartLine(0), m_EndLine(0)
{
	for (int i = (int)params.size() - 1; i >= 0; i--)
	{
		AddFunctionParam(params[i]);
	}
}

HazeCompilerFunction::~HazeCompilerFunction()
{
}

void HazeCompilerFunction::SetStartEndLine(uint32 startLine, uint32 endLine)
{
	m_StartLine = startLine;
	m_EndLine = endLine;
#if HAZE_DEBUG_ENABLE
	HAZE_LOG_ERR_W("SetStartEndLine %s %d %d\n", m_Name.c_str(), startLine, endLine);
#endif // HAZE_DEBUG_ENABLE
}

Share<HazeCompilerValue> HazeCompilerFunction::CreateLocalVariable(const HazeDefineVariable& Variable, 
	int line, Share<HazeCompilerValue> refValue,V_Array<Share<HazeCompilerValue>> arraySize,
	V_Array<HazeDefineType>* params)
{
	auto block = m_Module->GetCompiler()->GetInsertBlock();
	return block->CreateAlloce(Variable, line, ++m_CurrVariableCount, refValue, arraySize, params);
}

Share<HazeCompilerValue> HazeCompilerFunction::CreateNew(const HazeDefineType& data, Share<HazeCompilerValue> countValue)
{
	HAZE_STRING_STREAM hss;
	hss << GetInstructionString(InstructionOpCode::NEW) << " " << NEW_REGISTER << " " << CAST_TYPE(data.PrimaryType) << " ";
	if (data.CustomName->empty())
	{
		hss << CAST_TYPE(data.SecondaryType);
	}
	else
	{
		hss << data.CustomName;
	}

	hss << " " << CAST_SCOPE(HazeVariableScope::Local) << " " << CAST_DESC(HazeDataDesc::RegisterNew) << " ";
	if (countValue)
	{
		GenVariableHzic(m_Module, hss, countValue);
	}
	else
	{
		GenVariableHzic(m_Module, hss, m_Module->GetCompiler()->GetConstantValueUint64(0));
	}

	hss << std::endl;
	
	auto block = m_Module->GetCompiler()->GetInsertBlock();
	block->PushIRCode(hss.str());

	auto ret = m_Module->GetCompiler()->GetNewRegister(m_Module, data);

	return ret;
}

Share<HazeCompilerValue> HazeCompilerFunction::GetLocalVariable(const HString& variableName)
{
	Share<HazeCompilerValue> ret = nullptr;

	auto currBlock = m_Module->GetCompiler()->GetInsertBlock().get();
	while (currBlock)
	{
		for (auto& value : currBlock->GetAllocaList())
		{
			if (value.first == variableName)
			{
				ret = value.second;
				break;
			}
			else if (value.second->GetValueType().PrimaryType == HazeValueType::Class)
			{
				auto memberValue = GetObjectMember(m_Module, variableName);
				if (memberValue)
				{
					ret = memberValue;
					break;
				}
			}
		}

		if (ret)
		{
			break;
		}
		currBlock = currBlock->GetParentBlock();
	}

	if (!ret && m_OwnerClass)
	{
		ret = DynamicCast<HazeCompilerClassValue>(m_OwnerClass->GetThisPointerToValue())->GetMember(variableName);
	}

	return ret;
}

void HazeCompilerFunction::FunctionFinish()
{
	if (m_Type.PrimaryType == HazeValueType::Void)
	{
		HAZE_STRING_STREAM hss;
		hss << GetInstructionString(InstructionOpCode::RET) << " " << H_TEXT("Void") << " " << CAST_SCOPE(HazeVariableScope::None) << " "
			<< CAST_DESC(HazeDataDesc::None) << " " << CAST_TYPE(HazeValueType::Void) << std::endl;
		m_Module->GetCompiler()->GetInsertBlock()->PushIRCode(hss.str());
	}
}

void HazeCompilerFunction::GenI_Code(HAZE_STRING_STREAM& hss)
{
	hss << GetFunctionLabelHeader() << " " << m_Name << " ";

	if (!m_Type.StringStreamTo(hss))
	{
		HAZE_LOG_ERR_W("函数<%s>类型解析失败,生成中间代码错误!\n", m_Name.c_str());
		return;
	}

	hss << std::endl;

	//Push所有参数，从右到左, push 参数与返回地址的事由function call去做
	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		hss << GetFunctionParamHeader() << " " << m_Params[i].first << " ";

		if (!m_Params[i].second->GetValueType().StringStreamTo(hss))
		{
			HAZE_LOG_ERR_W("函数<%s>的参数<%s>类型解析失败,生成中间代码错误!\n", m_Name.c_str(), m_Params[i].first.c_str());
			return;
		}

		hss << std::endl;
	}

	HString LocalVariableName;
	int size = -HAZE_ADDRESS_SIZE;

	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		if (!FindLocalVariableName(m_LocalVariables[i].first, LocalVariableName))
		{
			HAZE_LOG_ERR_W("函数<%s>生成中间代码错误，未能找到参数临时变量!\n", m_Name.c_str());
			return;
		}
		size -= m_LocalVariables[i].first->GetSize();
		
		hss << HAZE_LOCAL_VARIABLE_HEADER << " " << size << " " << LocalVariableName;
		HazeCompilerStream(hss, m_LocalVariables[i].first, false);

		hss << " " << m_LocalVariables[i].first->GetSize() << " " << m_LocalVariables[i].second << std::endl;
	}

	size = 0;

	for (size_t i = m_Params.size(); i < m_LocalVariables.size(); i++)
	{
		FindLocalVariableName(m_LocalVariables[i].first, LocalVariableName);
		hss << HAZE_LOCAL_VARIABLE_HEADER << " " << size << " " << LocalVariableName;
		HazeCompilerStream(hss, m_LocalVariables[i].first, false);
		hss << " " << m_LocalVariables[i].first->GetSize() << " " << m_LocalVariables[i].second << std::endl;
		size += m_LocalVariables[i].first->GetSize();
	}

	hss << GetFunctionStartHeader() << " " << m_StartLine << std::endl;

	m_EntryBlock->GenI_Code(hss);

	hss << std::endl << GetFunctionEndHeader() << " " << m_EndLine << std::endl << std::endl;

	m_EntryBlock->ClearLocalVariable();
}

HString HazeCompilerFunction::GenDafaultBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_DEFAULT << ++m_CurrBlockCount;
	return hss.str();
}

HString HazeCompilerFunction::GenIfThenBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_IF_THEN << ++m_CurrBlockCount;
	return hss.str();
}

HString HazeCompilerFunction::GenElseBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_ELSE << ++m_CurrBlockCount;
	return hss.str();
}

HString HazeCompilerFunction::GenLoopBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_LOOP << ++m_CurrBlockCount;
	return hss.str();
}

HString HazeCompilerFunction::GenWhileBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_WHILE << ++m_CurrBlockCount;
	return hss.str();
}

HString HazeCompilerFunction::GenForBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR << ++m_CurrBlockCount;
	return hss.str();
}

HString HazeCompilerFunction::GenForConditionBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR_CONDITION << ++m_CurrBlockCount;
	return hss.str();
}

HString HazeCompilerFunction::GenForStepBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR_STEP << ++m_CurrBlockCount;
	return hss.str();
}

bool HazeCompilerFunction::FindLocalVariableName(const Share<HazeCompilerValue>& value, HString& outName)
{
	if (m_EntryBlock->FindLocalVariableName(value, outName))
	{
		return true;
	}
	else if (m_OwnerClass)
	{
		return m_OwnerClass->GetMemberName(value, outName);
	}

	return false;
}

bool HazeCompilerFunction::FindLocalVariableName(const HazeCompilerValue* value, HString& outName)
{
	if (m_EntryBlock->FindLocalVariableName(value, outName))
	{
		return true;
	}
	else if (m_OwnerClass)
	{
		return m_OwnerClass->GetMemberName(value, outName);
	}

	return false;
}

bool HazeCompilerFunction::HasExceptThisParam() const
{
	if (m_OwnerClass)
	{
		if (m_Params.size() > 0)
		{
			if (m_Params[0].second->IsCalssThis())
			{
				return m_Params.size() > 1;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		return m_Params.size() > 0;
	}

	return false;
}

void HazeCompilerFunction::AddLocalVariable(Share<HazeCompilerValue> value, int line)
{
	m_LocalVariables.push_back({ value, line });
}

const HazeDefineType& HazeCompilerFunction::GetParamTypeByIndex(uint64 index)
{
	if (index < m_Params.size())
	{
		return m_Params[index].second->GetValueType();
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("函数<%s>从右往左，获得函数的第<%d>个参数错误", m_Name.c_str(), m_Params.size() - 1 - index);
		}
		
		return m_Params[0].second->GetValueType();
	}
}

const HazeDefineType& HazeCompilerFunction::GetParamTypeLeftToRightByIndex(uint64 index)
{
	if (m_OwnerClass)
	{
		index += 1;
	}

	if (index < m_Params.size())
	{
		return m_Params[m_Params.size() - 1 - index].second->GetValueType();
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("函数<%s>从左往右，获得函数的第<%d>个参数错误", m_Name.c_str(), index);
			return m_Params[0].second->GetValueType();
		}
		else
		{
			return HazeDefineType();
		}
	}
}

void HazeCompilerFunction::AddFunctionParam(const HazeDefineVariable& variable)
{
	m_Module->BeginCreateFunctionParamVariable();
	m_Params.push_back({ variable.Name, CreateVariable(m_Module, variable, HazeVariableScope::Local, HazeDataDesc::None, 0) });
	m_Module->EndCreateFunctionParamVariable();
}