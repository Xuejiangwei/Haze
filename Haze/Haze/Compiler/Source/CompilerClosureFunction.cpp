#include "HazePch.h"
#include "CompilerClosureFunction.h"
#include "CompilerElementValue.h"
#include "CompilerClassValue.h"
#include "CompilerBlock.h"
#include "CompilerModule.h"
#include "Compiler.h"
#include "CompilerHelper.h"

CompilerClosureFunction::CompilerClosureFunction(CompilerModule* compilerModule, const STDString& name, HazeVariableType& type, V_Array<HazeDefineVariable>& params)
	: CompilerFunction(compilerModule, name, type, params, HazeFunctionDesc::Closure)
{
}

CompilerClosureFunction::~CompilerClosureFunction()
{
}

int CompilerClosureFunction::AddRefValue(int variableIndex, Share<CompilerValue> refValue, const STDString& name)
{
	if (!refValue->IsAdvance())
	{
		COMPILER_ERR_W("添加闭包引用<%s>错误, 类型必须为复杂类型", name.c_str());
		return 0;
	}

	for (auto& v : m_RefValues)
	{
		if (v.first == variableIndex)
		{
			return -1;
		}
	}

	m_RefValues.push_back({ variableIndex, (int)m_LocalVariables.size() });
	m_EntryBlock->AddClosureRefValue(refValue, name);
	m_LocalVariables.push_back({ refValue, 0 });

	return (int)m_LocalVariables.size() - 1;
}

void CompilerClosureFunction::GenI_Code_RefVariable(HAZE_STRING_STREAM& hss)
{
	// 引用的外部变量
	for (x_uint64 i = 0; i < m_RefValues.size(); i++)
	{
		hss << CLOSURE_REF_VARIABLE << " " << m_RefValues[i].first << " " << m_RefValues[i].second << HAZE_ENDL;
	}
}

Share<CompilerValue> CompilerClosureFunction::GetLocalVariable(const STDString& name, Share<CompilerBlock> startBlock)
{
	Share<CompilerValue> ret = nullptr;

	auto currBlock = startBlock.get();
	while (currBlock)
	{
		for (auto& value : currBlock->GetAllocaList())
		{
			if (value.first == name)
			{
				ret = value.second;
				break;
			}
			else if (value.second->IsClassThis())
			{
				auto memberValue = DynamicCast<CompilerClassValue>(value.second)->GetMember(name, nullptr);
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

bool CompilerClosureFunction::ExistRefVariable(const STDString& name) const
{
	auto func = m_Module->GetUpOneLevelClosureOrFunction();

	if (DynamicCast<CompilerClosureFunction>(func))
	{
		return m_Module->GetClosureVariable(name, false) != nullptr;
	}
	
	return func->GetLocalVariable(name, nullptr) != nullptr;
}
