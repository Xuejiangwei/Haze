#include "HazeBaseBlock.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"

HazeBaseBlock::HazeBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* Parent) : Name(Name)
{
	ParentFunction = Parent;

	IRCode.clear();
}

HazeBaseBlock::~HazeBaseBlock()
{
}

std::shared_ptr<HazeBaseBlock> HazeBaseBlock::CreateBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* Parent, std::shared_ptr<HazeBaseBlock> InsertBefore)
{
	std::shared_ptr<HazeBaseBlock> BB = std::make_shared<HazeBaseBlock>(Name, Parent);
	
	if (Parent)
	{
		auto& BBList = Parent->GetBaseBlockList();

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

void HazeBaseBlock::ClearTempIRCode()
{
	for (size_t i = BlockAllocaList.size() - 1; i >= 0; i--)
	{
		if (BlockAllocaList[i].second->IsTemp())
		{
			HAZE_STRING_STREAM SStream;
			SStream << GetInstructionString(InstructionOpCode::POP) << " " << HAZE_CAST_VALUE_TYPE(BlockAllocaList[i].second->GetValue().Type)
				<< " " << BlockAllocaList[i].first << " " << (unsigned int)HazeDataDesc::Temp << std::endl;
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
