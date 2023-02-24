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

std::shared_ptr<HazeCompilerValue> HazeBaseBlock::CreateAlloce(const HazeDefineVariable& Define)
{
	std::shared_ptr<HazeCompilerValue> Alloce = std::make_shared<HazeCompilerValue>(ParentFunction->GetModule(), Define.first, HazeCompilerValue::ValueSection::Local);
	BlockAllocaList.push_back({ Define.second, Alloce });

	HAZE_STRING_STREAM SStream;
	SStream << HAZE_TEXT("Push ") << Define.second << " " << Alloce->GetValue().Type << std::endl;

	IRCode.push_back(SStream.str());

	return Alloce;
}
