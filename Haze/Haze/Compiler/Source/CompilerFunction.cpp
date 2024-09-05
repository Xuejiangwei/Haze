#include "HazePch.h"
#include "Compiler.h"
#include "CompilerHelper.h"
#include "CompilerModule.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClass.h"
#include "CompilerFunction.h"
#include "CompilerBlock.h"
#include "HazeLogDefine.h"

CompilerFunction::CompilerFunction(CompilerModule* compilerModule, const HString& name, 
	HazeDefineType& type, V_Array<HazeDefineVariable>& params, CompilerClass* compilerClass)
	: m_Module(compilerModule), m_Name(name), m_Type(type), m_OwnerClass(compilerClass), m_CurrBlockCount(0), 
		m_CurrVariableCount(0), m_StartLine(0), m_EndLine(0)
{
	for (int i = (int)params.size() - 1; i >= 0; i--)
	{
		AddFunctionParam(params[i]);
	}
}

CompilerFunction::~CompilerFunction()
{
}

void CompilerFunction::SetStartEndLine(uint32 startLine, uint32 endLine)
{
	m_StartLine = startLine;
	m_EndLine = endLine;
#if HAZE_DEBUG_ENABLE
	HAZE_LOG_ERR_W("SetStartEndLine %s %d %d\n", m_Name.c_str(), startLine, endLine);
#endif // HAZE_DEBUG_ENABLE
}

Share<CompilerValue> CompilerFunction::CreateLocalVariable(const HazeDefineVariable& Variable, int line, Share<CompilerValue> refValue,
	uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	auto block = m_Module->GetCompiler()->GetInsertBlock();
	return block->CreateAlloce(Variable, line, ++m_CurrVariableCount, refValue, arrayDimension, params);
}

Share<CompilerValue> CompilerFunction::CreateNew(const HazeDefineType& data, Share<CompilerValue> assignTo, V_Array<Share<CompilerValue>>* countValue)
{
	HAZE_STRING_STREAM hss;

	if (countValue)
	{
		for (uint64 i = 0; i < countValue->size(); i++)
		{
			GenIRCode(hss, m_Module, InstructionOpCode::PUSH, nullptr, countValue->at(i));
		}
	}

	GenIRCode(hss, m_Module, InstructionOpCode::NEW, assignTo, m_Module->GetCompiler()->GetConstantValueUint64(countValue->size()),
		nullptr, &data);

	if (countValue)
	{
		for (uint64 i = 0; i < countValue->size(); i++)
		{
			GenIRCode(hss, m_Module, InstructionOpCode::POP, nullptr, countValue->at(i));
		}
	}
	
	auto block = m_Module->GetCompiler()->GetInsertBlock();
	block->PushIRCode(hss.str());

	//auto ret = m_Module->GetCompiler()->GetNewRegister(m_Module, data);

	return assignTo;
}

Share<CompilerValue> CompilerFunction::CreateTempRegister(const HazeDefineType& type, uint64 arrayDimension)
{
	int offset = 0;
	for (auto& var : m_TempRegisters)
	{
		if (var.Value->GetValueType() == type && var.Value.use_count() == 1)
		{
			if (var.Value->IsArray())
			{
				if (DynamicCast<CompilerArrayValue>(var.Value)->GetArrayDimension() == arrayDimension)
				{
					return var.Value;
				}
			}
			else
			{
				return var.Value;
			}

		}

		++offset;
	}

	Share<CompilerValue> v = nullptr;
	if (IsArrayType(type.PrimaryType))
	{
		assert(arrayDimension > 0);
		v = MakeShare<CompilerArrayValue>(nullptr, type, HazeVariableScope::Local,
			HazeDataDesc::RegisterTemp, 0, arrayDimension);
	}
	else if (IsClassType(type.PrimaryType))
	{
		v = MakeShare<CompilerClassValue>(nullptr, type, HazeVariableScope::Local, HazeDataDesc::RegisterTemp, 0);
	}
	else
	{
		v = MakeShare<CompilerValue>(nullptr, type, HazeVariableScope::Local, 
			HazeDataDesc::RegisterTemp, 0);
	}

	m_TempRegisters.push_back({ v , offset });
	return v;
}

void CompilerFunction::TryClearTempRegister()
{
	for (auto& var : m_TempRegisters)
	{
		if (var.Value.use_count() == 1 && !var.HasClear)
		{
			var.HasClear = true;
			
			if (var.Value->IsAdvance())
			{
				m_Module->GetCompiler()->CreateMov(var.Value, m_Module->GetCompiler()->GetConstantValueUint64(0));
			}
		}
	}
}

Share<CompilerValue> CompilerFunction::GetLocalVariable(const HString& variableName)
{
	Share<CompilerValue> ret = nullptr;

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
			else if (value.second->IsClassThis())
			{
				auto memberValue = DynamicCast<CompilerClassValue>(value.second)->GetMember(variableName);
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

	return ret;
}

void CompilerFunction::FunctionFinish()
{
	if (m_Type.PrimaryType == HazeValueType::Void)
	{
		HAZE_STRING_STREAM hss;
		GenIRCode(hss, GetModule(), InstructionOpCode::RET, nullptr, nullptr, nullptr, nullptr);
		hss << std::endl;
		m_Module->GetCompiler()->GetInsertBlock()->PushIRCode(hss.str());
	}
}

void CompilerFunction::GenI_Code(HAZE_STRING_STREAM& hss)
{
	hss << GetFunctionLabelHeader() << " " << m_Name << " ";

	if (!m_Type.StringStreamTo(hss))
	{
		HAZE_LOG_ERR_W("����<%s>���ͽ���ʧ��,�����м�������!\n", m_Name.c_str());
		return;
	}

	hss << std::endl;

	//Push���в��������ҵ���, push �����뷵�ص�ַ������function callȥ��
	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		hss << GetFunctionParamHeader() << " " << m_Params[i].first << " ";

		if (!m_Params[i].second->GetValueType().StringStreamTo(hss))
		{
			HAZE_LOG_ERR_W("����<%s>�Ĳ���<%s>���ͽ���ʧ��,�����м�������!\n", m_Name.c_str(), m_Params[i].first.c_str());
			return;
		}

		hss << std::endl;
	}

	HString LocalVariableName;
	int size = -HAZE_ADDRESS_SIZE;

	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		if (!FindLocalVariableName(m_LocalVariables[i].first.get(), LocalVariableName))
		{
			HAZE_LOG_ERR_W("����<%s>�����м�������δ���ҵ�������ʱ����!\n", m_Name.c_str());
			return;
		}
		size -= m_LocalVariables[i].first->GetValueType().GetTypeSize();
		
		hss << HAZE_LOCAL_VARIABLE_HEADER << " " << size << " " << LocalVariableName;
		HazeCompilerStream(hss, m_LocalVariables[i].first, false);

		hss << " " << m_LocalVariables[i].first->GetSize() << " " << m_LocalVariables[i].second << std::endl;
	}

	size = 0;

	for (size_t i = m_Params.size(); i < m_LocalVariables.size(); i++)
	{
		FindLocalVariableName(m_LocalVariables[i].first.get(), LocalVariableName);
		hss << HAZE_LOCAL_VARIABLE_HEADER << " " << size << " " << LocalVariableName;
		HazeCompilerStream(hss, m_LocalVariables[i].first, false);
		hss << " " << m_LocalVariables[i].first->GetValueType().GetTypeSize() << " " << m_LocalVariables[i].second << std::endl;
		size += m_LocalVariables[i].first->GetValueType().GetTypeSize();
	}

	for (uint64 i = 0; i < m_TempRegisters.size(); i++)
	{
		hss << HAZE_LOCAL_TEMP_REGISTER_HEADER << " " << HAZE_LOCAL_TEMP_REGISTER << i << " "
			<< size + m_TempRegisters[i].Offset * 8 << " ";
		m_TempRegisters[i].Value->GetValueType().StringStreamTo(hss);
		hss << std::endl;
	}

	hss << GetFunctionStartHeader() << " " << m_StartLine << std::endl;

	m_EntryBlock->GenI_Code(hss);

	hss << std::endl << GetFunctionEndHeader() << " " << m_EndLine << std::endl << std::endl;

	m_EntryBlock->ClearLocalVariable();
}

HString CompilerFunction::GenDafaultBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_DEFAULT << ++m_CurrBlockCount;
	return hss.str();
}

HString CompilerFunction::GenIfThenBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_IF_THEN << ++m_CurrBlockCount;
	return hss.str();
}

HString CompilerFunction::GenElseBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_ELSE << ++m_CurrBlockCount;
	return hss.str();
}

HString CompilerFunction::GenLoopBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_LOOP << ++m_CurrBlockCount;
	return hss.str();
}

HString CompilerFunction::GenWhileBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_WHILE << ++m_CurrBlockCount;
	return hss.str();
}

HString CompilerFunction::GenForBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR << ++m_CurrBlockCount;
	return hss.str();
}

HString CompilerFunction::GenForConditionBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR_CONDITION << ++m_CurrBlockCount;
	return hss.str();
}

HString CompilerFunction::GenForStepBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR_STEP << ++m_CurrBlockCount;
	return hss.str();
}

bool CompilerFunction::FindLocalVariableName(const CompilerValue* value, HString& outName, bool getOffset, V_Array<Pair<uint64, CompilerValue*>>* offsets)
{
	if (m_EntryBlock->FindLocalVariableName(value, outName, getOffset, offsets))
	{
		return true;
	}

	for (uint64 i = 0; i < m_TempRegisters.size(); i++)
	{
		if (m_TempRegisters[i].Value.get() == value)
		{
			outName = HAZE_LOCAL_TEMP_REGISTER + String2WString(ToString(i));
			return true;
		}
	}

	return false;
}

bool CompilerFunction::HasExceptThisParam() const
{
	if (m_OwnerClass)
	{
		if (m_Params.size() > 0)
		{
			if (m_Params[0].second->IsClassThis())
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

void CompilerFunction::AddLocalVariable(Share<CompilerValue> value, int line)
{
	m_LocalVariables.push_back({ value, line });
}

const HazeDefineType& CompilerFunction::GetParamTypeByIndex(uint64 index)
{
	if (index < m_Params.size())
	{
		return m_Params[index].second->GetValueType();
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("����<%s>�������󣬻�ú����ĵ�<%d>����������", m_Name.c_str(), m_Params.size() - 1 - index);
		}
		
		return m_Params[0].second->GetValueType();
	}
}

const HazeDefineType& CompilerFunction::GetParamTypeLeftToRightByIndex(uint64 index)
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
		if (index > 0 && IsMultiVariableTye(m_Params[0].second->GetValueType().PrimaryType))
		{
			return m_Params[0].second->GetValueType();
		}
		else
		{
			COMPILER_ERR_W("����<%s>�������ң���ú����ĵ�<%d>����������", m_Name.c_str(), index);
			return HazeDefineType();
		}
	}
}

const HazeDefineType& CompilerFunction::GetThisParam()
{
	if (m_OwnerClass)
	{
		return m_Params[m_Params.size() - 1].second->GetValueType();
	}
	else
	{
		COMPILER_ERR_W("����<%s>�����ຯ��", m_Name.c_str());
		return HazeDefineType();
	}
}

void CompilerFunction::AddFunctionParam(const HazeDefineVariable& variable)
{
	m_Module->BeginCreateFunctionParamVariable();
	m_Params.push_back({ variable.Name, CreateVariable(m_Module, variable.Type, HazeVariableScope::Local, HazeDataDesc::None, 0) });
	m_Module->EndCreateFunctionParamVariable();
}