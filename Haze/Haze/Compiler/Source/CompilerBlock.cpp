#include "HazePch.h"
#include "CompilerBlock.h"
#include "Compiler.h"
#include "CompilerHelper.h"
#include "CompilerModule.h"
#include "CompilerValue.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerFunction.h"
#include "CompilerClosureFunction.h"
#include "HazeTokenText.h"

CompilerBlock::CompilerBlock(STDString&& name, CompilerFunction* parentFunction, CompilerBlock* parentBlock)
	: enable_shared_from_this(*this), m_Name(Move(name)), m_ParentFunction(parentFunction), m_ParentBlock(parentBlock), m_LoopEndBlock(nullptr)
{
	m_IRCodes.clear();
	m_Allocas.clear();
	m_Index = parentFunction->GetCurrBlockCount() - 1;
	PushIRCode(STDString(BLOCK_START) + H_TEXT(" ") + m_Name + H_TEXT("\n"));
}

CompilerBlock::~CompilerBlock()
{
}

bool CompilerBlock::FindLocalVariableName(const Share<CompilerValue>& value, HStringView& outName)
{
	for (auto& it : m_Allocas)
	{
		if (it.second == value)
		//if (TrtGetVariableName(it, value.get(), outName))
		{
			outName = it.first;
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
	return m_Name.starts_with(BLOCK_WHILE) || m_Name.starts_with(BLOCK_LOOP);
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

void CompilerBlock::GenI_Code(HAZE_STRING_STREAM& hss)
{
	hss << HAZE_ENDL;
	hss << m_IRCodes[0];

	GenI_Code_FlowGraph(hss);

	for (x_uint64 i = 1; i < m_IRCodes.size(); i++)
	{
		hss << m_IRCodes[i];
	}

	for (auto& iter : m_ChildBlocks)
	{
		iter->GenI_Code(hss);
	}
}

void CompilerBlock::GenI_Code_FlowGraph(HAZE_STRING_STREAM& hss)
{
	hss << m_Index << " ";

	hss << m_Predecessors.size() << " ";
	for (auto it = m_Predecessors.begin(); it != m_Predecessors.end(); it++)
	{
		hss << (*it)->GetIndex() << " ";
	}

	hss << m_Successors.size() << " ";
	for (auto it = m_Successors.begin(); it != m_Successors.end(); it++)
	{
		hss << (*it)->GetIndex() << " ";
	}
		
	hss << HAZE_ENDL;
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

Share<CompilerBlock> CompilerBlock::CreateBaseBlock(STDString&& name,  Share<CompilerFunction> parent, Share<CompilerBlock> parentBlock)
{
	return CreateBaseBlock(Move(name), parent.get(), parentBlock.get());
}

Share<CompilerBlock> CompilerBlock::CreateBaseBlock(STDString&& name, CompilerFunction* Parent, CompilerBlock* parentBlock)
{
	auto BB = MakeShare<CompilerBlock>(Move(name), Parent, parentBlock);

	if (parentBlock)
	{
		parentBlock->AddChildBlock(BB);
	}

	return BB;
}

void CompilerBlock::PushIRCode(const STDString& code)
{
	m_IRCodes.push_back(code);
}

Share<CompilerValue> CompilerBlock::CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, /*HazeVariableScope scope,*/HazeDataDesc desc,
	Share<CompilerValue> refValue, TemplateDefineTypes* params)
{
	for (auto& iter : m_Allocas)
	{
		if (iter.first == defineVar.Name)
		{
			//if (scope == HazeVariableScope::Global)
			if (IsGlobalDesc(desc))
			{
				return iter.second;
				//COMPILER_ERR_MODULE_W("重复全局添加变量<%s>", m_ParentFunction->GetModule()->GetCompiler(), m_ParentFunction->GetModule()->GetName().c_str(), defineVar.Name.c_str());
			}
			else if (IsLocalDesc(desc))
			{
				COMPILER_ERR_MODULE_W("重复添加局部变量<%s>", m_ParentFunction->GetModule()->GetCompiler(), m_ParentFunction->GetModule()->GetName().c_str(), defineVar.Name.c_str());
			}
			else
			{
				COMPILER_ERR_MODULE_W("重复添加变量<%s>且作用域错误", m_ParentFunction->GetModule()->GetCompiler(), m_ParentFunction->GetModule()->GetName().c_str(), defineVar.Name.c_str());
			}
			return nullptr;
		}
	}

	auto m = m_ParentFunction->GetModule()->ExistGlobalValue(defineVar.Name);
	if (m)
	{
		COMPILER_ERR_MODULE_W("局部变量<%s>与<%s>模块全局变量名重复", m_ParentFunction->GetModule()->GetCompiler(), m_ParentFunction->GetModule()->GetName().c_str(), defineVar.Name.c_str(), m->GetName().c_str());
		return nullptr;
	}

	if (dynamic_cast<CompilerClosureFunction*>(m_ParentFunction) && dynamic_cast<CompilerClosureFunction*>(m_ParentFunction)->ExistRefVariable(defineVar.Name))
	{
		COMPILER_ERR_MODULE_W("局部变量<%s>存在相同名字的闭包引用", m_ParentFunction->GetModule()->GetCompiler(), m_ParentFunction->GetModule()->GetName().c_str(), defineVar.Name.c_str());
		return nullptr;
	}

	desc = defineVar.Name == TOKEN_THIS ? HazeDataDesc::ClassThis : desc;
	//HazeDataDesc desc = defineVar.Name == TOKEN_THIS ? HazeDataDesc::ClassThis : HazeDataDesc::None;
	Share<CompilerValue> allocaValue = CreateVariable(m_ParentFunction->GetModule(), defineVar.Type, /*scope,*/ desc, count, refValue, params);
	m_Allocas.push_back({ STDString(defineVar.Name.c_str()), allocaValue });

	m_ParentFunction->AddLocalVariable(allocaValue, line);

	return allocaValue;
}

void CompilerBlock::AddClosureRefValue(Share<CompilerValue> refValue, const STDString& name)
{
	for (auto& it : m_Allocas)
	{
		if (it.first == name)
		{
			return;
		}
	}

	m_Allocas.push_back({ name.c_str(), refValue});
}
