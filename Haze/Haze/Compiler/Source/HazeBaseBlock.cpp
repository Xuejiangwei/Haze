#include "HazeLog.h"

#include "HazeBaseBlock.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerFunction.h"

HazeBaseBlock::HazeBaseBlock(const HAZE_STRING& name, HazeCompilerFunction* parentFunction, HazeBaseBlock* parentBlock)
	: enable_shared_from_this(*this), m_Name(name), m_ParentFunction(parentFunction), m_ParentBlock(parentBlock), m_LoopEndBlock(nullptr)
{
	m_IRCodes.clear();
	m_Allocas.clear();
	PushIRCode(HAZE_STRING(BLOCK_START) + HAZE_TEXT(" ") + name + HAZE_TEXT("\n"));
}

HazeBaseBlock::~HazeBaseBlock()
{
}

bool HazeBaseBlock::FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName)
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

bool HazeBaseBlock::FindLocalVariableName(const HazeCompilerValue* value, HAZE_STRING& outName)
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
	static HAZE_STRING s_WhileBlockName = BLOCK_WHILE;
	static HAZE_STRING s_ForBlockName = BLOCK_LOOP;

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

void HazeBaseBlock::AddChildBlock(std::shared_ptr<HazeBaseBlock> block)
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

std::shared_ptr<HazeBaseBlock> HazeBaseBlock::CreateBaseBlock(const HAZE_STRING& name, std::shared_ptr<HazeCompilerFunction> Parent, std::shared_ptr<HazeBaseBlock> parentBlock)
{
	auto BB = std::make_shared<HazeBaseBlock>(name, Parent.get(), parentBlock.get());

	if (parentBlock)
	{
		parentBlock->AddChildBlock(BB);
	}

	return BB;
}

void HazeBaseBlock::PushIRCode(const HAZE_STRING& code)
{
	m_IRCodes.push_back(code);
}

std::shared_ptr<HazeCompilerValue> HazeBaseBlock::CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, std::shared_ptr<HazeCompilerValue> refValue,
	std::vector<std::shared_ptr<HazeCompilerValue>> arraySize, std::vector<HazeDefineType>* params)
{
	for (auto& Iter : m_Allocas)
	{
		if (Iter.first == defineVar.Name)
		{
			HAZE_LOG_ERR(HAZE_TEXT("重复添加临时变量 %s !\n"), defineVar.Name.c_str());
			return nullptr;
		}
	}

	std::shared_ptr<HazeCompilerValue> Alloce = CreateVariable(m_ParentFunction->GetModule(), defineVar, HazeVariableScope::Local, HazeDataDesc::None, count,
		refValue, arraySize, params);
	m_Allocas.push_back({ defineVar.Name, Alloce });

	m_ParentFunction->AddLocalVariable(Alloce, line);

	return Alloce;
}