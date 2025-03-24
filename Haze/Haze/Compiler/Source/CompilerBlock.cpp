#include "HazePch.h"
#include "CompilerBlock.h"
#include "CompilerHelper.h"
#include "CompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerFunction.h"
#include "HazeTokenText.h"

CompilerBlock::CompilerBlock(const HString& name, CompilerFunction* parentFunction, CompilerBlock* parentBlock)
	: enable_shared_from_this(*this), m_Name(name), m_ParentFunction(parentFunction), m_ParentBlock(parentBlock), m_LoopEndBlock(nullptr)
{
	m_IRCodes.clear();
	m_Allocas.clear();
	PushIRCode(HString(BLOCK_START) + H_TEXT(" ") + name + H_TEXT("\n"));
}

CompilerBlock::~CompilerBlock()
{
}

bool CompilerBlock::FindLocalVariableName(const Share<CompilerValue>& value, HString& outName)
{
	for (auto& it : m_Allocas)
	{
		if (TrtGetVariableName(m_ParentFunction, it, value.get(), outName))
		{
			return true;
		}
	}

	for (auto& iter : m_ChildBlocks)
	{
		if (iter->FindLocalVariableName(value, outName))
		{
			return true;
		}
	}

	return false;
}

CompilerBlock* CompilerBlock::FindLoopBlock()
{
	auto block = this;
	while (block)
	{
		if (block->IsLoopBlock())
		{
			return block;
		}

		block = block->GetParentBlock();
	}

	return nullptr;
}

bool CompilerBlock::IsLoopBlock() const
{
	static HString s_WhileBlockName = BLOCK_WHILE;
	static HString s_ForBlockName = BLOCK_LOOP;

	if (m_Name.length() >= s_WhileBlockName.length() && m_Name.substr(0, s_WhileBlockName.length()) == s_WhileBlockName)
	{
		return true;
	}

	if (m_Name.length() >= s_ForBlockName.length() && m_Name.substr(0, s_ForBlockName.length()) == s_ForBlockName)
	{
		return true;
	}

	return false;
}

void CompilerBlock::AddChildBlock(Share<CompilerBlock> block)
{
	m_ChildBlocks.push_back(block);
}

void CompilerBlock::GenI_Code(HAZE_STRING_STREAM& hss)
{
	hss << std::endl;

	for (size_t i = 0; i < m_IRCodes.size(); i++)
	{
		hss << m_IRCodes[i];
	}

	for (auto& iter : m_ChildBlocks)
	{
		iter->GenI_Code(hss);
	}
}

void CompilerBlock::ClearLocalVariable()
{
	for (auto& iter : m_Allocas)
	{
		iter.second.reset();
	}

	m_Allocas.clear();

	for (auto& iter : m_ChildBlocks)
	{
		iter->ClearLocalVariable();
	}
}

Share<CompilerBlock> CompilerBlock::CreateBaseBlock(const HString& name, Share<CompilerFunction> Parent, Share<CompilerBlock> parentBlock)
{
	auto BB = MakeShare<CompilerBlock>(name, Parent.get(), parentBlock.get());

	if (parentBlock)
	{
		parentBlock->AddChildBlock(BB);
	}

	return BB;
}

void CompilerBlock::PushIRCode(const HString& code)
{
	m_IRCodes.push_back(code);
}

Share<CompilerValue> CompilerBlock::CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, HazeVariableScope scope,
	Share<CompilerValue> refValue, uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	for (auto& Iter : m_Allocas)
	{
		if (Iter.first == defineVar.Name)
		{
			if (scope == HazeVariableScope::Global)
			{
				HAZE_LOG_ERR_W("重复全局添加变量<%s> !\n", defineVar.Name.c_str());

			}
			else if (scope == HazeVariableScope::Local)
			{
				HAZE_LOG_ERR_W("重复添加局部变量<%s> !\n", defineVar.Name.c_str());

			}
			else
			{
				HAZE_LOG_ERR_W("重复添加变量<%s>且作用域错误 !\n", defineVar.Name.c_str());
			}
			return nullptr;
		}
	}

	HazeDataDesc desc = defineVar.Name == TOKEN_THIS ? HazeDataDesc::ClassThis : HazeDataDesc::None;
	Share<CompilerValue> Alloce = CreateVariable(m_ParentFunction->GetModule(), defineVar.Type, scope, desc, count,
		refValue, arrayDimension, params);
	m_Allocas.push_back({ defineVar.Name, Alloce });

	m_ParentFunction->AddLocalVariable(Alloce, line);

	return Alloce;
}