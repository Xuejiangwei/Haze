#include "HazeLog.h"

#include "HazeBaseBlock.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerFunction.h"

HazeBaseBlock::HazeBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* ParentFunction, HazeBaseBlock* ParentBlock) 
	: enable_shared_from_this(*this), Name(Name), ParentFunction(ParentFunction), ParentBlock(ParentBlock), LoopEndBlock(nullptr)
{
	Vector_IRCode.clear();
	Vector_Alloca.clear();
	PushIRCode(HAZE_STRING(BLOCK_START) + HAZE_TEXT(" ") + Name + HAZE_TEXT("\n"));
}

HazeBaseBlock::~HazeBaseBlock()
{
}

bool HazeBaseBlock::FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	for (auto& it : Vector_Alloca)
	{
		if (TrtGetVariableName(ParentFunction, it, Value, OutName))
		{
			return true;
		}
	}

	for (auto& Iter : List_ChildBlock)
	{
		if (Iter->FindLocalVariableName(Value, OutName))
		{
			return true;
		}
	}

	return false;
}

bool HazeBaseBlock::FindLocalVariableName(const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	for (auto& it : Vector_Alloca)
	{
		if (TrtGetVariableName(ParentFunction, it, Value, OutName))
		{
			return true;
		}
	}

	for (auto& Iter : List_ChildBlock)
	{
		if (Iter->FindLocalVariableName(Value, OutName))
		{
			return true;
		}
	}

	return false;
}

HazeBaseBlock* HazeBaseBlock::FindLoopBlock()
{
	auto Block = this;
	while (Block)
	{
		if (Block->IsLoopBlock())
		{
			return Block;
		}

		Block = Block->GetParentBlock();
	}

	return nullptr;
}

bool HazeBaseBlock::IsLoopBlock() const
{
	static HAZE_STRING WhileBlockName = BLOCK_WHILE;
	static HAZE_STRING ForBlockName = BLOCK_LOOP;

	if (Name.length() >= WhileBlockName.length() && Name.substr(0, WhileBlockName.length()) == WhileBlockName)
	{
		return true;
	}

	if (Name.length() >= ForBlockName.length() && Name.substr(0, ForBlockName.length()) == ForBlockName)
	{
		return true;
	}

	return false;
}

void HazeBaseBlock::AddChildBlock(std::shared_ptr<HazeBaseBlock> Block)
{
	List_ChildBlock.push_back(Block);
}

void HazeBaseBlock::GenI_Code(HAZE_STRING_STREAM& SStream)
{
	SStream << std::endl;

	for (size_t i = 0; i < Vector_IRCode.size(); i++)
	{
		SStream << Vector_IRCode[i];
	}

	for (auto& Iter : List_ChildBlock)
	{
		Iter->GenI_Code(SStream);
	}
}

void HazeBaseBlock::ClearLocalVariable()
{
	for (auto& Iter : Vector_Alloca)
	{
		Iter.second.reset();
	}

	Vector_Alloca.clear();

	for (auto& Iter : List_ChildBlock)
	{
		Iter->ClearLocalVariable();
	}
}

std::shared_ptr<HazeBaseBlock> HazeBaseBlock::CreateBaseBlock(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerFunction> Parent, std::shared_ptr<HazeBaseBlock> ParentBlock)
{
	auto BB = std::make_shared<HazeBaseBlock>(Name, Parent.get(), ParentBlock.get());
	
	if (ParentBlock)
	{
		ParentBlock->AddChildBlock(BB);
	}

	return BB;
}

void HazeBaseBlock::PushIRCode(const HAZE_STRING& Code)
{
	Vector_IRCode.push_back(Code);
}

std::shared_ptr<HazeCompilerValue> HazeBaseBlock::CreateAlloce(const HazeDefineVariable& Define, int Count, std::shared_ptr<HazeCompilerValue> RefValue,
	std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize, std::vector<HazeDefineType>* Vector_Param)
{
	for (auto& Iter : Vector_Alloca)
	{
		if (Iter.first == Define.Name)
		{
			HAZE_LOG_ERR(HAZE_TEXT("重复添加临时变量 %s !\n"), Define.Name.c_str());
			return nullptr;
		}
	}

	std::shared_ptr<HazeCompilerValue> Alloce = CreateVariable(ParentFunction->GetModule(), Define, HazeVariableScope::Local, HazeDataDesc::None, Count, 
		RefValue, ArraySize, Vector_Param);
	Vector_Alloca.push_back({ Define.Name, Alloce });

	ParentFunction->AddLocalVariable(Alloce);

	return Alloce;
}
