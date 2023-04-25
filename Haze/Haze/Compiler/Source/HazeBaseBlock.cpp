#include "HazeLog.h"

#include "HazeBaseBlock.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerFunction.h"

HazeBaseBlock::HazeBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* ParentFunction, HazeBaseBlock* ParentBlock) 
	: enable_shared_from_this(*this), Name(Name), ParentFunction(ParentFunction), ParentBlock(ParentBlock)
{
	IsFinish = false;
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

void HazeBaseBlock::AddChildBlock(std::shared_ptr<HazeBaseBlock> Block)
{
	List_ChildBlock.push_back(Block);
}

void HazeBaseBlock::SetJmpOut()
{
	HAZE_STRING_STREAM HSS;
	HSS << GetInstructionString(InstructionOpCode::JMPOUT) << std::endl;
	PushIRCode(HSS.str());
}

void HazeBaseBlock::FinishBlock(std::shared_ptr<HazeBaseBlock> MoveFinishPopBlock, bool JmpOut)
{
	if (IsFinish)
	{
		return;
	}

	//auto BB = MoveFinishPopBlock ? MoveFinishPopBlock.get() : this;
	//for (int i = (int)Vector_Alloca.size() - 1; i >= 0; i--)
	//{
	//	HAZE_STRING_STREAM SStream;
	//	SStream << GetInstructionString(InstructionOpCode::POP) << " " << HAZE_CAST_VALUE_TYPE(Vector_Alloca[i].second->GetValue().Type)
	//		<< " " << GetLocalVariableName(Vector_Alloca[i].first, Vector_Alloca[i].second) << " " << HAZE_CAST_VALUE_TYPE(Vector_Alloca[i].second->GetScope()) << std::endl;

	//	//Vector_Alloca.pop_back();

	//	BB->PushIRCode(SStream.str());
	//}

	if (JmpOut)
	{
		SetJmpOut();
	}

	IsFinish = true;
}

void HazeBaseBlock::GenI_Code(HAZE_OFSTREAM& OFStream, int SkipCount)
{
	OFStream << std::endl;

	for (size_t i = 0; i < Vector_IRCode.size(); i++)
	{
		if (i != 0 && SkipCount-- > 0)
		{
			continue;
		}
		OFStream << Vector_IRCode[i];
	}

	for (auto& Iter : List_ChildBlock)
	{
		Iter->GenI_Code(OFStream);
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

void HazeBaseBlock::MergeJmpIRCode(std::shared_ptr<HazeBaseBlock> BB)
{
	auto& Code = BB->GetIRCode();
	if (Code.size() > 1)
	{
		HAZE_STRING_STREAM HSS;
		HSS << " " << Code.size() - 1 << std::endl;
		Vector_IRCode.back() += HSS.str();
	}
}

std::shared_ptr<HazeCompilerValue> HazeBaseBlock::CreateAlloce(const HazeDefineVariable& Define, int Count, std::shared_ptr<HazeCompilerValue> ArraySize)
{
	std::shared_ptr<HazeCompilerValue> Alloce = CreateVariable(ParentFunction->GetModule(), Define, HazeDataDesc::Local, Count, ArraySize);
	Vector_Alloca.push_back({ Define.Name, Alloce });

	/*HAZE_STRING_STREAM SStream;
	HAZE_STRING Name = GetLocalVariableName(Define.Name, Alloce);

	StreamCompilerValue(SStream, InstructionOpCode::PUSH, Alloce, Name.c_str());
	PushIRCode(SStream.str());*/

	ParentFunction->AddLocalVariable(Alloce);

	return Alloce;
}
