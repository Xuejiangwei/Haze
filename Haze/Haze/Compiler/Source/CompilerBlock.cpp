#include "HazePch.h"
#include "CompilerBlock.h"
#include "CompilerHelper.h"
#include "CompilerModule.h"
#include "CompilerValue.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerFunction.h"
#include "CompilerClosureFunction.h"
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

void CompilerBlock::AddPredecessor(Share<CompilerBlock> block)
{
	if (block)
	{
		m_Predecessors.push_back(block);
	}
}

void CompilerBlock::AddSuccessor(Share<CompilerBlock> block1, Share<CompilerBlock> block2)
{
	if (block1)
	{
		m_Successors.push_back(block1);
		block1->AddPredecessor(GetShared());
	}

	if (block2)
	{
		m_Successors.push_back(block2);
		block2->AddPredecessor(GetShared());
	}
}

void CompilerBlock::GenI_Code(HAZE_STRING_STREAM& hss, HashMap<CompilerBlock*, x_uint64>& blockIndex)
{
	hss << HAZE_ENDL;

	if (m_IRCodes.size() > 0)
	{
		blockIndex[this] = blockIndex.size();
	}

	for (x_uint64 i = 0; i < m_IRCodes.size(); i++)
	{
		hss << m_IRCodes[i];
	}

	for (auto& iter : m_ChildBlocks)
	{
		iter->GenI_Code(hss, blockIndex);
	}
}

void CompilerBlock::GenI_Code_FlowGraph(HAZE_STRING_STREAM& hss, HashMap<CompilerBlock*, x_uint64>& blockIndex)
{
	auto iter = blockIndex.find(this);
	if (iter != blockIndex.end())
	{
		hss << iter->second << " ";

		hss << m_Predecessors.size() << " ";
		for (auto it = m_Predecessors.begin(); it != m_Predecessors.end(); it++)
		{
			auto iter = blockIndex.find(it->get());
			if (iter != blockIndex.end())
			{
				hss << iter->second << " ";
			}
		}

		hss << m_Successors.size() << " ";
		for (auto it = m_Successors.begin(); it != m_Successors.end(); it++)
		{
			auto iter = blockIndex.find(it->get());
			if (iter != blockIndex.end())
			{
				hss << iter->second << " ";
			}
		}
		
		hss << HAZE_ENDL;
	}

	for (auto& iter : m_ChildBlocks)
	{
		iter->GenI_Code_FlowGraph(hss, blockIndex);
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
	Share<CompilerValue> refValue, TemplateDefineTypes* params)
{
	for (auto& iter : m_Allocas)
	{
		if (iter.first == defineVar.Name)
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

	auto m = m_ParentFunction->GetModule()->ExistGlobalValue(defineVar.Name);
	if (m)
	{
		HAZE_LOG_ERR_W("局部变量<%s>与<%s>模块全局变量名重复!\n", defineVar.Name.c_str(), m->GetName().c_str());
		return nullptr;
	}

	if (dynamic_cast<CompilerClosureFunction*>(m_ParentFunction) && dynamic_cast<CompilerClosureFunction*>(m_ParentFunction)->ExistRefVariable(defineVar.Name))
	{
		HAZE_LOG_ERR_W("局部变量<%s>存在相同名字的闭包引用!\n", defineVar.Name.c_str());
		return nullptr;
	}

	HazeDataDesc desc = defineVar.Name == TOKEN_THIS ? HazeDataDesc::ClassThis : HazeDataDesc::None;
	Share<CompilerValue> allocaValue = CreateVariable(m_ParentFunction->GetModule(), defineVar.Type, scope, desc, count, refValue, params);
	m_Allocas.push_back({ defineVar.Name, allocaValue });

	m_ParentFunction->AddLocalVariable(allocaValue, line);

	return allocaValue;
}

void CompilerBlock::AddClosureRefValue(Share<CompilerValue> refValue, const HString& name)
{
	for (auto& it : m_Allocas)
	{
		if (it.first == name)
		{
			return;
		}
	}

	m_Allocas.push_back({ name, refValue });
}
