#include "HazeVM.h"
#include "HazeLog.h"

#include "Parse.h"
#include "HazeCompiler.h"
#include "BackendParse.h"
#include "HazeExecuteFile.h"

#include "HazeDebuggerServer.h"
#include "HazeDebugger.h"

#include "HazeStack.h"
#include "HazeMemory.h"

extern std::unique_ptr<HazeDebugger> Debugger;

HazeVM::HazeVM(HazeRunType GenType) : GenType(GenType)
{
	VMStack = std::make_unique<HazeStack>(this);
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

	if (IsDebug())
	{
		HazeDebuggerServer::InitDebuggerServer(this);
		//VMDebugger = std::make_unique<HazeDebugger>(this);
		//VMDebugger->SetHook(&HazeVM::Hook, HazeDebugger::DebuggerHookType::Instruction | HazeDebugger::DebuggerHookType::Line);
	}
}

void HazeVM::LoadStandardLibrary(std::vector<ModulePair> Vector_ModulePath)
{
}

void HazeVM::StartMainFunction()
{
	//VMDebugger->AddBreakPoint(HAZE_TEXT("五子棋"), 68);

	HazeMemory::GetMemory()->SetVM(this);

	auto Iter = HashMap_FunctionTable.find(HAZE_MAIN_FUNCTION_TEXT);
	if (Iter != HashMap_FunctionTable.end())
	{
		VMStack->Start(Vector_FunctionTable[Iter->second].FunctionDescData.InstructionStartAddress);
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

void* HazeVM::GetGlobalValue(const HAZE_STRING& Name)
{
	static HAZE_STRING ObjectName;
	static HAZE_STRING MemberName;

	auto Pos = Name.find(HAZE_CLASS_POINTER_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		ObjectName = Name.substr(0, Pos);
		MemberName = Name.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());

		for (auto& Iter : Vector_GlobalData)
		{
			if (Iter.Name == ObjectName)
			{
				auto Class = FindClass(Iter.GetType().CustomName);
				if (Class)
				{
					for (size_t i = 0; i < Class->Vector_Member.size(); i++)
					{
						if (Class->Vector_Member[i].Variable.Name == MemberName)
						{
							uint64 Address = 0;
							memcpy(&Address, Iter.Value.Value.Pointer, sizeof(Address));
							return (char*)(Address + Class->Vector_Member[i].Offset);
						}
					}
				}
			}
		}
	}

	Pos = Name.find(HAZE_CLASS_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		ObjectName = Name.substr(0, Pos);
		MemberName = Name.substr(Pos + HAZE_STRING(HAZE_CLASS_ATTR).size());

		for (auto& Iter : Vector_GlobalData)
		{
			if (Iter.Name == ObjectName)
			{
				auto Class = FindClass(Iter.GetType().CustomName);
				if (Class)
				{
					for (size_t i = 0; i < Class->Vector_Member.size(); i++)
					{
						if (Class->Vector_Member[i].Variable.Name == MemberName)
						{
							return (char*)Iter.Address + Class->Vector_Member[i].Offset;
						}
					}
				}
			}
		}
	}
	else
	{
		for (auto& Iter : Vector_GlobalData)
		{
			if (Iter.Name == Name)
			{
				return &Iter.Value.Value;
			}
		}
	}

	return nullptr;
}

char* HazeVM::GetGlobalValueByIndex(uint32 Index)
{
	if (Index < Vector_GlobalData.size())
	{
		if (IsClassType(Vector_GlobalData[Index].Type.PrimaryType))
		{
			return (char*)Vector_GlobalData[Index].Address;
		}
		else
		{
			return (char*)(&Vector_GlobalData[Index].Value);
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
	if (Debugger)
	{
		Debugger->OnExecLine(Line);
	}
}

void HazeVM::InstructionExecPost()
{
#define HAZE_INS_LOG 0

#if HAZE_INS_LOG

	HAZE_LOG_INFO(HAZE_TEXT("执行指令<%s> <%s>\n"), GetInstructionString(Vector_Instruction[VMStack->GetCurrPC()].InsCode),
		Vector_Instruction[VMStack->GetCurrPC()].Operator[0].Variable.Name.c_str());

#endif
}

uint32 HazeVM::GetNextLine(uint32 CurrLine)
{
	uint32 StartAddress = VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	uint32 InstructionNum = VMStack->GetCurrFrame().FunctionInfo->InstructionNum;
	for (size_t i = VMStack->GetCurrPC(); i < StartAddress + InstructionNum; i++)
	{
		if (Vector_Instruction[i].InsCode == InstructionOpCode::LINE && Vector_Instruction[i].Operator[0].Extra.Line > CurrLine)
		{
			return Vector_Instruction[i].Operator[0].Extra.Line;
		}
	}

	return CurrLine + 1;
}

uint32 HazeVM::GetCurrCallFunctionLine()
{
	uint32 StartAddress = VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	for (size_t i = VMStack->GetCurrPC(); i >= StartAddress; i--)
	{
		if (Vector_Instruction[i].InsCode == InstructionOpCode::LINE)
		{
			return Vector_Instruction[i].Operator[0].Extra.Line;
		}
	}

	return 0;
}

void HazeVM::Hook(HazeVM* VM)
{
	HAZE_LOG_INFO(HAZE_TEXT("已命中断点\n"));
}