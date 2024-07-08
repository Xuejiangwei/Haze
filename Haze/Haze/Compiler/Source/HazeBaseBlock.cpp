#include "HazePch.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerFunction.h"

HazeBaseBlock::HazeBaseBlock(const HString& name, HazeCompilerFunction* parentFunction, HazeBaseBlock* parentBlock)
	: enable_shared_from_this(*this), m_Name(name), m_ParentFunction(parentFunction), m_ParentBlock(parentBlock), m_LoopEndBlock(nullptr)
{
	m_IRCodes.clear();
	m_Allocas.clear();
	PushIRCode(HString(BLOCK_START) + H_TEXT(" ") + name + H_TEXT("\n"));
}

HazeBaseBlock::~HazeBaseBlock()
{
}

bool HazeBaseBlock::FindLocalVariableName(const Share<HazeCompilerValue>& value, HString& outName)
{
	for (auto& it : m_Allocas)
	{
		if (TrtGetVariableName(m_ParentFunction, it, value, outName))
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

bool HazeBaseBlock::FindLocalVariableName(const HazeCompilerValue* value, HString& outName)
{
	for (auto& it : m_Allocas)
	{
		if (TrtGetVariableName(m_ParentFunction, it, value, outName))
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

HazeBaseBlock* HazeBaseBlock::FindLoopBlock()
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

bool HazeBaseBlock::IsLoopBlock() const
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

void HazeBaseBlock::AddChildBlock(Share<HazeBaseBlock> block)
{
	m_ChildBlocks.push_back(block);
}

void HazeBaseBlock::GenI_Code(HAZE_STRING_STREAM& hss)
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

void HazeBaseBlock::ClearLocalVariable()
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

Share<HazeBaseBlock> HazeBaseBlock::CreateBaseBlock(const HString& name, Share<HazeCompilerFunction> Parent, Share<HazeBaseBlock> parentBlock)
{
	auto BB = MakeShare<HazeBaseBlock>(name, Parent.get(), parentBlock.get());

	if (parentBlock)
	{
		parentBlock->AddChildBlock(BB);
	}

	return BB;
}

void HazeBaseBlock::PushIRCode(const HString& code)
{
	m_IRCodes.push_back(code);
}

Share<HazeCompilerValue> HazeBaseBlock::CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, Share<HazeCompilerValue> refValue,
	V_Array<Share<HazeCompilerValue>> arraySize, V_Array<HazeDefineType>* params)
{
	for (auto& Iter : m_Allocas)
	{
		if (Iter.first == defineVar.Name)
		{
			HAZE_LOG_ERR_W("重复添加临时变量 %s !\n", defineVar.Name.c_str());
			return nullptr;
		}
	}

	Share<HazeCompilerValue> Alloce = CreateVariable(m_ParentFunction->GetModule(), defineVar, HazeVariableScope::Local, HazeDataDesc::None, count,
		refValue, arraySize, params);
	m_Allocas.push_back({ defineVar.Name, Alloce });

	m_ParentFunction->AddLocalVariable(Alloce, line);

	return Alloce;
}