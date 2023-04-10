#include "HazeBaseBlock.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"

HazeBaseBlock::HazeBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* Parent) : Name(Name)
{
	ParentFunction = Parent;
	IsFinish = false;
	IRCode.clear();
	PushIRCode(HAZE_STRING(BLOCK_START) + HAZE_TEXT(" ") + Name + HAZE_TEXT("\n"));
}

HazeBaseBlock::~HazeBaseBlock()
{
}

void HazeBaseBlock::FinishBlock(std::shared_ptr<HazeBaseBlock> TopBlock)
{
	for (int i = (int)BlockAllocaList.size() - 1; i >= 0; i--)
	{
		HAZE_STRING_STREAM SStream;
		SStream << GetInstructionString(InstructionOpCode::POP) << " " << HAZE_CAST_VALUE_TYPE(BlockAllocaList[i].second->GetValue().Type)
			<< " " << BlockAllocaList[i].first << " " << HAZE_CAST_VALUE_TYPE(BlockAllocaList[i].second->GetScope()) << std::endl;
		PushIRCode(SStream.str());

		BlockAllocaList.pop_back();
	}

	IsFinish = true;
}

std::shared_ptr<HazeBaseBlock> HazeBaseBlock::CreateBaseBlock(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerFunction> Parent, std::shared_ptr<HazeBaseBlock> InsertBefore)
{
	std::shared_ptr<HazeBaseBlock> BB = std::make_shared<HazeBaseBlock>(Name, Parent.get());
	
	if (Parent)
	{
		auto& BBList = const_cast<std::list<std::shared_ptr<HazeBaseBlock>>&>(Parent->GetBaseBlockList());

		if (InsertBefore)
		{
			for (auto iter = BBList.begin(); iter != BBList.end(); ++iter)
			{
				if (*iter == InsertBefore)
				{
					BBList.insert(iter, BB);
					break;
				}
			}
		}
		else
		{
			BBList.push_back(BB);
		}
	}

	return BB;
}

void HazeBaseBlock::PushIRCode(const HAZE_STRING& Code)
{
	IRCode.push_back(Code);
}

void HazeBaseBlock::MergeJmpIRCode(std::shared_ptr<HazeBaseBlock> BB)
{
	auto& Code = BB->GetIRCode();
	if (Code.size() > 1)
	{
		HAZE_STRING_STREAM HSS;
		HSS << " " << Code.size() - 1 << std::endl;
		IRCode.back() += HSS.str();
	}
}

void HazeBaseBlock::CopyIRCode(std::shared_ptr<HazeBaseBlock> BB)
{
	auto& Code = BB->GetIRCode();
	if (Code.size() > 1)
	{
		IRCode.insert(IRCode.end(), ++Code.begin(), Code.end());		//skip block name
	}
}

void HazeBaseBlock::ClearTempIRCode()
{
	for (int i = (int)BlockAllocaList.size() - 1; i >= 0; i--)
	{
		if (BlockAllocaList[i].second->IsTemp())
		{
			HAZE_STRING_STREAM SStream;
			SStream << GetInstructionString(InstructionOpCode::POP) << " " << HAZE_CAST_VALUE_TYPE(BlockAllocaList[i].second->GetValue().Type)
				<< " " << BlockAllocaList[i].first << " " << (uint32)HazeDataDesc::Temp << std::endl;
			PushIRCode(SStream.str());

			BlockAllocaList.pop_back();
		}
		else if (!BlockAllocaList[i].second->IsTemp())
		{
			return;
		}
	}
}

std::shared_ptr<HazeCompilerValue> HazeBaseBlock::CreateAlloce(const HazeDefineVariable& Define)
{
	std::shared_ptr<HazeCompilerValue> Alloce = CreateVariable(ParentFunction->GetModule(), Define, HazeDataDesc::Local);
	BlockAllocaList.push_back({ Define.Name, Alloce });

	HAZE_STRING_STREAM SStream;
	StreamCompilerValue(SStream, InstructionOpCode::PUSH, Alloce, Define.Name.c_str());

	PushIRCode(SStream.str());

	return Alloce;
}

std::shared_ptr<HazeCompilerValue> HazeBaseBlock::CreateTempAlloce(const HazeDefineVariable& Define)
{
	std::shared_ptr<HazeCompilerValue> Alloce = CreateVariable(ParentFunction->GetModule(), Define, HazeDataDesc::Temp);
	BlockAllocaList.push_back({ Define.Name, Alloce });

	HAZE_STRING_STREAM SStream;
	StreamCompilerValue(SStream, InstructionOpCode::PUSH, Alloce, Define.Name.c_str());

	PushIRCode(SStream.str());

	return Alloce;
}
