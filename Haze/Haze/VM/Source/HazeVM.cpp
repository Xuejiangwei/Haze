#include "HazeVM.h"
#include "HazeLog.h"

#include "Parse.h"
#include "HazeCompiler.h"
#include "BackendParse.h"
#include "HazeExecuteFile.h"

#include "HazeDebugger.h"
#include "HazeStack.h"
#include "GarbageCollection.h"


HazeVM::HazeVM(HazeGenType GenType) : GenType(GenType)
{
	VMStack = std::make_unique<HazeStack>(this);
	GC = std::make_unique<GarbageCollection>(this);
	
	Compiler = std::make_unique<HazeCompiler>(this);
}

HazeVM::~HazeVM()
{
}

void HazeVM::InitVM(std::vector<ModulePair> Vector_ModulePath)
{
	for (auto& iter : Vector_ModulePath)
	{
		ParseFile(iter.first, iter.second);
	}

#define HAZE_BACKEND_PARSE_ENABLE	1
	{

#if HAZE_BACKEND_PARSE_ENABLE

		BackendParse BP(this);
		BP.Parse();

#endif

	}

#define HAZE_LOAD_OP_CODE_ENABLE	1

	{

#if HAZE_LOAD_OP_CODE_ENABLE

		HazeExecuteFile ExeFile(ExeFileType::In);
		ExeFile.ReadExecuteFile(this);

#endif
	}

	Compiler.release();
	HashSet_RefModule.clear();

	if (!VMDebugger)
	{
		VMDebugger = std::make_unique<HazeDebugger>(this);
		VMDebugger->SetHook(&HazeVM::Hook, HazeDebugger::DebuggerHookType::Instruction | HazeDebugger::DebuggerHookType::Line);
	}
}

void HazeVM::LoadStandardLibrary(std::vector<ModulePair> Vector_ModulePath)
{
}

void HazeVM::StartMainFunction()
{
	VMDebugger->AddBreakPoint(HAZE_TEXT("五子棋"), 58);

	auto Iter = HashMap_FunctionTable.find(HAZE_MAIN_FUNCTION_TEXT);
	if (Iter != HashMap_FunctionTable.end())
	{
		VMStack->Start(Vector_FunctionTable[Iter->second].Extra.FunctionDescData.InstructionStartAddress);
	}
}

//void HazeVM::ParseString(const HAZE_STRING& String)
//{
//	Parse P(Compiler.get());
//	P.InitializeString(String);
//	P.ParseContent();
//}

void HazeVM::ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName)
{
	bool PopCurrModule = false;
	if (Compiler->InitializeCompiler(ModuleName))
	{
		PopCurrModule = true;
		Parse P(Compiler.get());
		P.InitializeFile(FilePath);
		P.ParseContent();
		Compiler->FinishModule();
	}

	HashSet_RefModule.insert(ModuleName);

	if (PopCurrModule)
	{
		Compiler->PopCurrModule();
	}
}

const HAZE_STRING* HazeVM::GetModuleNameByCurrFunction()
{
	for (size_t i = 0; i < Vector_FunctionTable.size(); i++)
	{
		if (&Vector_FunctionTable[i] == VMStack->GetCurrFrame().FunctionInfo)
		{
			for (auto& Iter : Vector_ModuleData)
			{
				if (Iter.FunctionIndex.first <= i && i < Iter.FunctionIndex.second) 
				{
					return &Iter.Name;
				}
			}
		}
	}

	return nullptr;
}

int HazeVM::GetFucntionIndexByName(const HAZE_STRING& Name)
{
	auto Iter = HashMap_FunctionTable.find(Name);
	if (Iter == HashMap_FunctionTable.end())
	{
		return -1;
	}
	return Iter->second;
}

const FunctionData& HazeVM::GetFunctionByName(const HAZE_STRING& Name)
{
	int Index = GetFucntionIndexByName(Name);
	return Vector_FunctionTable[Index];
}

HazeValue* HazeVM::GetGlobalValue(const HAZE_STRING& Name)
{
	for (auto& Iter : Vector_GlobalData)
	{
		if (Iter.Name == Name)
		{
			return &Iter.Value;
		}
	}

	return nullptr;
}

ClassData* HazeVM::FindClass(const HAZE_STRING& ClassName)
{
	for (auto& Iter : Vector_ClassTable)
	{
		if (Iter.Name == ClassName)
		{
			return &Iter;
		}
	}

	return nullptr;
}

uint32 HazeVM::GetClassSize(const HAZE_STRING& ClassName)
{
	auto Class = FindClass(ClassName);
	return Class ? Class->Size : 0;
}

void HazeVM::OnExecLine(uint32 Line)
{
	VMDebugger->OnExecLine(Line);
}

void HazeVM::InstructionExecPost()
{
#define HAZE_INS_LOG 0

#if HAZE_INS_LOG

	HAZE_LOG_INFO(HAZE_TEXT("执行指令<%s> <%s>\n"), GetInstructionString(Vector_Instruction[VMStack->GetCurrPC()].InsCode), 
		Vector_Instruction[VMStack->GetCurrPC()].Operator[0].Variable.Name.c_str());

#endif

}

void HazeVM::Hook(HazeVM* VM)
{
	HAZE_LOG_INFO(HAZE_TEXT("已命中断点\n"));
}
