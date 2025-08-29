#include "HazePch.h"
#include "Compiler.h"
#include "CompilerHelper.h"
#include "Compiler.h"
#include "CompilerSymbol.h"
#include "CompilerModule.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerElementValue.h"
#include "CompilerStringValue.h"
#include "CompilerClass.h"
#include "CompilerFunction.h"
#include "CompilerBlock.h"
#include "HazeLogDefine.h"
#include "CompilerClosureFunction.h"

#include "ASTBase.h"

CompilerFunction::CompilerFunction(CompilerModule* compilerModule, const STDString& name, const HazeVariableType& type, 
	V_Array<HazeDefineVariable>& params, HazeFunctionDesc desc, CompilerClass* compilerClass)
	: m_Module(compilerModule), m_Name(name.c_str()), m_Type(type), m_OwnerClass(compilerClass), m_CurrBlockCount(0), 
		m_CurrVariableCount(0), m_StartLine(0), m_EndLine(0), m_Desc(desc)
{
	for (int i = (int)params.size() - 1; i >= 0; i--)
	{
		AddFunctionParam(params[i]);
	}

	InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, this, nullptr));
	m_DescType = GetFunctionTypeByLibraryType(m_Module->GetModuleLibraryType());
}

CompilerFunction::~CompilerFunction()
{
}

void CompilerFunction::SetStartEndLine(x_uint32 startLine, x_uint32 endLine)
{
	m_StartLine = startLine;
	m_EndLine = endLine;
#if HAZE_DEBUG_ENABLE
	HAZE_LOG_ERR_W("SetStartEndLine %s %d %d\n", m_Name.c_str(), startLine, endLine);
#endif // HAZE_DEBUG_ENABLE
}

Share<CompilerValue> CompilerFunction::GetParamVariableRightToLeft(x_uint32 index)
{
	return m_Params[index].Value;
}

Share<CompilerValue> CompilerFunction::GetParamVariableLeftToRight(x_uint32 index)
{
	if (m_OwnerClass)
	{
		index += 1;
	}

	if (index < m_Params.size())
	{
		return m_Params[m_Params.size() - 1 - index].Value;
	}
	else
	{
		if (index > 0 && IsMultiVariableType(m_Params[0].Value->GetBaseType()))
		{
			return m_Params[0].Value;
		}
		else
		{
			COMPILER_ERR_W("函数<%s>从左往右，获得函数的第<%d>个参数错误", m_Name.c_str(), index);
			return m_Params[0].Value;
		}
	}

	return nullptr;
}

Share<CompilerValue> CompilerFunction::CreateGlobalVariable(const HazeDefineVariable& variable, int line, Share<CompilerValue> refValue, TemplateDefineTypes* params)
{
	return m_EntryBlock->CreateAlloce(variable, line, ++m_CurrVariableCount, HazeVariableScope::Global, refValue, params);
}

Share<CompilerValue> CompilerFunction::CreateLocalVariable(const HazeDefineVariable& variable, int line, Share<CompilerValue> refValue)
{
	auto block = m_Module->GetCompiler()->GetInsertBlock();
	return block->CreateAlloce(variable, line, ++m_CurrVariableCount, HazeVariableScope::Local, refValue);
}

void CompilerFunction::PushDefaultParam(x_uint64 pushCount)
{
	for (x_uint64 i = 0; i < pushCount; i++)
	{
		m_Module->GetCompiler()->CreatePush(m_Params[i].DefaultParamAST->CodeGen(m_Params[i].Value));
	}
}

//Share<CompilerValue> CompilerFunction::CreateNew(const HazeDefineType& data, V_Array<Share<CompilerValue>>* countValue)
//{
//	HAZE_STRING_STREAM hss;
//
//	if (countValue)
//	{
//		for (uint64 i = 0; i < countValue->size(); i++)
//		{
//			GenIRCode(hss, m_Module, InstructionOpCode::PUSH, nullptr, countValue->at(i));
//		}
//	}
//
//	auto tempRegister = GetModule()->GetCompiler()->GetTempRegister(data, countValue ? countValue->size() : 0);
//	GenIRCode(hss, m_Module, InstructionOpCode::NEW, tempRegister, m_Module->GetCompiler()->GetConstantValueUint64(countValue->size()),
//		nullptr, &data);
//
//	if (countValue)
//	{
//		for (uint64 i = 0; i < countValue->size(); i++)
//		{
//			GenIRCode(hss, m_Module, InstructionOpCode::POP, nullptr, countValue->at(i));
//		}
//	}
//	
//	auto block = m_Module->GetCompiler()->GetInsertBlock();
//	block->PushIRCode(hss.str());
//
//	//auto ret = m_Module->GetCompiler()->GetNewRegister(m_Module, data);
//
//	return tempRegister;
//}

Share<CompilerValue> CompilerFunction::CreateTempRegister(const HazeVariableType& type)
{
	int offset = 0;
	for (auto& var : m_TempRegisters)
	{
		if (var.Value->GetVariableType() == type && var.Value.use_count() == 1)
		{
			return var.Value;
		}

		++offset;
	}

	Share<CompilerValue> v = nullptr;


	switch (type.BaseType)
	{
		case HazeValueType::Refrence:
			HAZE_LOG_ERR_W("创建错误类型的临时变量\n");
			break;
		default:
			v = CreateVariable(m_Module, type, HazeVariableScope::Local, HazeDataDesc::RegisterTemp, 0);
			break;
	}

	m_TempRegisters.push_back({ (STDString(HAZE_LOCAL_TEMP_REGISTER) + String2WString(ToString(m_TempRegisters.size()))), v , offset });

	/*if (IsStringType(type.BaseType) && str)
	{
		auto prueStr = MakeShare<CompilerStringValue>(m_Module, HazeValueType::StringPure, HazeVariableScope::Local, HazeDataDesc::None, 0);
		prueStr->SetPureString(str);

		v = m_Module->GetCompiler()->CreateMov(v, prueStr);
	}*/

	if (!v)
	{
		COMPILER_ERR_W("未能创建<%s>类型的临时变量", m_Module->GetCompiler()->GetCompilerSymbol()->GetSymbolByTypeId(type.TypeId)->c_str());
	}

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
				m_Module->GetCompiler()->CreateMov(var.Value, m_Module->GetCompiler()->GetConstantValueUint64(0), false);
			}
		}
	}
}

Share<CompilerValue> CompilerFunction::GetLocalVariable(const STDString& variableName, STDString* nameSpace)
{
	Share<CompilerValue> ret = nullptr;

	auto currBlock = m_Module->GetCompiler()->GetFunctionInsertBlock().get();
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
				auto memberValue = DynamicCast<CompilerClassValue>(value.second)->GetMember(variableName, nameSpace);
				if (memberValue)
				{
					ret = MakeShare<CompilerElementValue>(m_Module, value.second, memberValue);
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

STDString CompilerFunction::GetRealName() const
{
	return m_OwnerClass ? GetHazeClassFunctionName(m_OwnerClass->GetName(), m_Name) : m_Name;
}

x_uint32 CompilerFunction::GetFunctionPointerTypeId()
{
	V_Array<x_uint32> params(m_Params.size());
	for (x_uint64 i = 0; i < params.size(); i++)
	{
		params[i] = m_Params[i].Value->GetTypeId();
	} 
	return m_Module->GetCompiler()->GetCompilerSymbol()->GetTypeInfoMap()->RegisterType(m_Module->GetName(), m_Type.TypeId, Move(params));
}

void CompilerFunction::SetParamDefaultAST(Unique<ASTBase>& ast)
{
	m_Params[(m_Params.size() - 1) - m_Module->GetCreateFunctionParamVariable()].DefaultParamAST = Move(ast);
}

void CompilerFunction::FunctionFinish()
{
	// 构造函数也返回空
	if (m_Type.BaseType == HazeValueType::Void || (m_OwnerClass && m_OwnerClass->GetName() == m_Name))
	{
		HAZE_STRING_STREAM hss;
		GenIRCode(hss, GetModule(), InstructionOpCode::RET, nullptr, nullptr, nullptr, nullptr);
		hss << HAZE_ENDL;
		m_Module->GetCompiler()->GetInsertBlock()->PushIRCode(hss.str());
	}
}

void CompilerFunction::ParseIntermediate(HAZE_IFSTREAM& stream, CompilerModule* m)
{
	if (m)
	{
		STDString str;
		stream >> str;
	}
}

void CompilerFunction::GenI_Code(HAZE_STRING_STREAM& hss)
{
	if (m_OwnerClass)
	{
		hss << GetClassFunctionLabelHeader() << " " << CAST_DESC(m_DescType) << " ";
		hss << m_OwnerClass->GetName() << " " << (x_uint32)m_Desc;
	}
	else
	{
		hss << GetFunctionLabelHeader() << " " << CAST_DESC(m_DescType);
	}

	hss << " " << GetRealName() << " ";

	if (!m_Type.StringStreamTo(hss))
	{
		HAZE_LOG_ERR_W("函数<%s>类型解析失败,生成中间代码错误!\n", m_Name.c_str());
		return;
	}

	hss << HAZE_ENDL;

	//Push所有参数，从右到左, push 参数与返回地址的事由function call去做
	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		hss << GetFunctionParamHeader() << " " << m_Params[i].Name << " ";

		if (!m_Params[i].Value->GetVariableType().StringStreamTo(hss))
		{
			HAZE_LOG_ERR_W("函数<%s>的参数<%s>类型解析失败,生成中间代码错误!\n", m_Name.c_str(), m_Params[i].Name.c_str());
			return;
		}

		hss << HAZE_ENDL;
	}

	HStringView LocalVariableName;
	int size = -HAZE_ADDRESS_SIZE;

	for (int i = (int)m_Params.size() - 1; i >= 0; i--)
	{
		if (!FindLocalVariableName(m_LocalVariables[i].first, LocalVariableName))
		{
			HAZE_LOG_ERR_W("函数<%s>生成中间代码错误，未能找到参数临时变量!\n", m_Name.c_str());
			return;
		}
		size -= m_LocalVariables[i].first->GetVariableType().GetTypeSize();
		
		hss << HAZE_LOCAL_VARIABLE_HEADER << " " << size << " " << LocalVariableName << HAZE_LOCAL_VARIABLE_CONBINE << m_LocalVariables[i].first->GetCount();
		HazeCompilerStream(hss, m_LocalVariables[i].first, false);

		hss << " " << m_LocalVariables[i].first->GetSize() << " " << m_LocalVariables[i].second << HAZE_ENDL;
	}

	size = 0;

	for (size_t i = m_Params.size(); i < m_LocalVariables.size(); i++)
	{
		FindLocalVariableName(m_LocalVariables[i].first, LocalVariableName);
		hss << HAZE_LOCAL_VARIABLE_HEADER << " " << size << " " << LocalVariableName << HAZE_LOCAL_VARIABLE_CONBINE << m_LocalVariables[i].first->GetCount();
		HazeCompilerStream(hss, m_LocalVariables[i].first, false);
		hss << " " << m_LocalVariables[i].first->GetVariableType().GetTypeSize() << " " << m_LocalVariables[i].second << HAZE_ENDL;
		size += m_LocalVariables[i].first->GetVariableType().GetTypeSize();
	}

	for (x_uint64 i = 0; i < m_TempRegisters.size(); i++)
	{
		hss << HAZE_LOCAL_TEMP_REGISTER_HEADER << " " << HAZE_LOCAL_TEMP_REGISTER << i << " " << size + m_TempRegisters[i].Offset * sizeof(HazeValue) << " ";
		m_TempRegisters[i].Value->GetVariableType().StringStreamTo(hss);
		hss << HAZE_ENDL;
	}

	if (dynamic_cast<CompilerClosureFunction*>(this))
	{
		dynamic_cast<CompilerClosureFunction*>(this)->GenI_Code_RefVariable(hss);
	}

	hss << GetFunctionStartHeader() << " " << m_StartLine << HAZE_ENDL;

	HashMap<CompilerBlock*, x_uint64> blockIndex;
	m_EntryBlock->GenI_Code(hss, blockIndex);
	
	hss << GetFunctionEndHeader() << " " << m_EndLine << HAZE_ENDL << HAZE_ENDL;

	hss << GetBlockFlowHeader() << " " << blockIndex.size() << HAZE_ENDL;
	m_EntryBlock->GenI_Code_FlowGraph(hss, blockIndex);
	hss << HAZE_ENDL;

	m_EntryBlock->ClearLocalVariable();
}

STDString CompilerFunction::GenDafaultBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_DEFAULT << ++m_CurrBlockCount;
	return hss.str();
}

STDString CompilerFunction::GenIfThenBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_IF_THEN << ++m_CurrBlockCount;
	return hss.str();
}

STDString CompilerFunction::GenElseBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_ELSE << ++m_CurrBlockCount;
	return hss.str();
}

STDString CompilerFunction::GenLoopBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_LOOP << ++m_CurrBlockCount;
	return hss.str();
}

STDString CompilerFunction::GenWhileBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_WHILE << ++m_CurrBlockCount;
	return hss.str();
}

STDString CompilerFunction::GenForBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR << ++m_CurrBlockCount;
	return hss.str();
}

STDString CompilerFunction::GenForConditionBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR_CONDITION << ++m_CurrBlockCount;
	return hss.str();
}

STDString CompilerFunction::GenForStepBlockName()
{
	HAZE_STRING_STREAM hss;
	hss << BLOCK_FOR_STEP << ++m_CurrBlockCount;
	return hss.str();
}

bool CompilerFunction::FindLocalVariableName(const Share<CompilerValue> value, HStringView& outName)
{
	if (m_EntryBlock->FindLocalVariableName(value, outName))
	{
		return true;
	}

	for (auto& it : m_TempRegisters)
	{
		if (it.Value == value)
		{
			outName = it.Name;
			return true;

		}
	}

	return false;
}

bool CompilerFunction::FindLocalVariableIndex(const Share<CompilerValue> value, InstructionOpId& outIndex)
{
	for (x_uint64 i = 0; i < m_LocalVariables.size(); i++)
	{
		if (value == m_LocalVariables[i].first)
		{
			outIndex.Id = i;
			return true;
		}
	}

	for (x_uint64 i = 0; i < m_TempRegisters.size(); i++)
	{
		if (value == m_TempRegisters[i].Value)
		{
			outIndex.Id = i;
			return true;
		}
	}

	return false;
}

int CompilerFunction::FindLocalVariableIndex(const Share<CompilerValue> value)
{
	for (x_uint64 i = 0; i < m_Params.size(); i++)
	{
		if (m_Params[i].Value == value)
		{
			return (int)i;
		}
	}

	for (x_uint64 i = 0; i < m_LocalVariables.size(); i++)
	{
		if (m_LocalVariables[i].first == value)
		{
			return (int)(i + m_Params.size());
		}
	}

	return -1;
}

bool CompilerFunction::HasExceptThisParam() const
{
	if (m_OwnerClass)
	{
		if (m_Params.size() > 0)
		{
			if (m_Params[0].Value->IsClassThis())
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

HazeVariableType CompilerFunction::GetParamTypeByIndex(x_uint64 index)
{
	if (index < m_Params.size())
	{
		return m_Params[index].Value->GetVariableType();
	}
	else
	{
		if (index > 0)
		{
			COMPILER_ERR_W("函数<%s>从右往左，获得函数的第<%d>个参数错误", m_Name.c_str(), m_Params.size() - 1 - index);
		}
		
		return m_Params[0].Value->GetVariableType();
	}
}

HazeVariableType CompilerFunction::GetParamTypeLeftToRightByIndex(x_uint64 index)
{
	if (m_OwnerClass)
	{
		index += 1;
	}

	if (index < m_Params.size())
	{
		return m_Params[m_Params.size() - 1 - index].Value->GetVariableType();
	}
	else
	{
		if (index > 0 && IsMultiVariableType(m_Params[0].Value->GetBaseType()))
		{
			return m_Params[0].Value->GetVariableType();
		}
		else
		{
			COMPILER_ERR_W("函数<%s>从左往右，获得函数的第<%d>个参数错误", m_Name.c_str(), index);
			return m_Params[0].Value->GetVariableType();
		}
	}
}

const x_uint64 CompilerFunction::GetParamCount() const
{
	return m_OwnerClass ? m_Params.size() - 1 : m_Params.size();
}

const x_uint64 CompilerFunction::GetDefaultParamCount() const
{
	x_uint64 count = 0;
	for (auto& param : m_Params)
	{
		if (param.DefaultParamAST)
		{
			count++;
		}
	}

	return count;
}

Share<CompilerClassValue> CompilerFunction::GetThisLocalVariable()
{
	auto currBlock = m_Module->GetCompiler()->GetInsertBlock().get();
	while (currBlock)
	{
		for (auto& value : currBlock->GetAllocaList())
		{
			if (value.second->IsClassThis())
			{
				return DynamicCast<CompilerClassValue>(value.second);
			}
		}

		currBlock = currBlock->GetParentBlock();
	}

	return nullptr;
}

void CompilerFunction::AddFunctionParam(const HazeDefineVariable& variable)
{
	//m_Module->BeginCreateFunctionParamVariable();
	m_Params.push_back({ variable.Name.data(), CreateVariable(m_Module, variable.Type, HazeVariableScope::Local, HazeDataDesc::None, 0), nullptr});
	//m_Module->EndCreateFunctionParamVariable();
}