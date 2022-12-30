#include <iostream>
#include <locale>

#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeCompiler.h"
#include "Parse.h"
#include "HazeLog.h"

HazeVM::HazeVM()
{
	std::wcout.imbue(std::locale("chs"));

	FunctionReturn.Type = HazeValueType::Null;
	Compiler = std::make_unique<HazeCompiler>();
	VMStack = std::make_unique<HazeStack>();
}

HazeVM::~HazeVM()
{
}

void HazeVM::ParseString(const HAZE_STRING& String)
{
	Parse P(this);
	P.InitializeString(String);
	P.ParseContent();
}

void HazeVM::ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName)
{
	if (Compiler->InitializeCompiler(ModuleName))
	{
		Parse P(this);
		P.InitializeFile(FilePath);
		P.ParseContent();
	}
	Compiler->FinishModule();
}

HazeValue* HazeVM::GetVirtualRegister(uint64_t Index)
{
	return VMStack->GetVirtualRegister(Index);
}