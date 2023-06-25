#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeInstruction.h"
#include "HazeMemory.h"
#include "MemoryPool.h"
#include "MemoryBucket.h"
#include "GarbageCollection.h"

#include "HazeDebugger.h"

extern std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> HashMap_InstructionProcessor;
extern std::unique_ptr<HazeDebugger> Debugger;

thread_local static std::unique_ptr<MemoryPool> Pool = std::make_unique<MemoryPool>();

HazeStack::HazeStack(HazeVM* VM) : VM(VM)
{
	PC = 0;
	EBP = 0;

	Stack_Main.resize(HAZE_VM_STACK_SIZE);
	InitStackRegister();
}

HazeStack::~HazeStack()
{
}

void HazeStack::Start(unsigned int Address)
{
	PC = Address;

	PreMainFunction();

	PushMainFuntion();

	while (PC < VM->Vector_Instruction.size())
	{
		while (VM->IsDebug())
		{
			if (!Debugger)
			{
				continue;
			}
			else
			{
				if (Debugger->IsPause)
				{
					continue;
				}
				break;
			}
		}

		auto Iter = HashMap_InstructionProcessor.find(VM->Vector_Instruction[PC].InsCode);
		if (Iter != HashMap_InstructionProcessor.end())
		{
			Iter->second(this);
		}
		else
		{
			return;
		}

		PCStepInc();
	}

	Debugger->SendProgramEnd();
	GarbageCollection(true, true);
}

void HazeStack::JmpTo(const InstructionData& Data)
{
	auto& Function = Stack_Frame.back().FunctionInfo;
	int Address = Function->FunctionDescData.InstructionStartAddress + Data.Extra.Jmp.StartAddress;
	memcpy(&PC, &Address, sizeof(PC));

	PC--;
}

void HazeStack::PCStepInc()
{
	++PC;
}

void HazeStack::PreMainFunction()
{
	//0-4存储FaultPC,Main函数return后读取此PC,然后退出循环
	int FaultPC = -2;
	memcpy(&Stack_Main[ESP], &FaultPC, HAZE_ADDRESS_SIZE);
	ESP = HAZE_ADDRESS_SIZE;
	EBP = 0;
}

void HazeStack::PushMainFuntion()
{
	auto& MainFunction = VM->GetFunctionByName(HAZE_MAIN_FUNCTION_TEXT);
	OnCall(&MainFunction, 0);
	PCStepInc();

	/*ESP -= HAZE_ADDRESS_SIZE;
	EBP -= HAZE_ADDRESS_SIZE;

	if (MainFunction.Vector_Variable.size() > 0)
	{
		ESP += MainFunction.Vector_Variable.back().Offset + MainFunction.Vector_Variable.back().Size;
	}*/
}

HazeRegister* HazeStack::GetVirtualRegister(const HAZE_CHAR* Name)
{
	auto Iter = HashMap_VirtualRegister.find(Name);
	if (Iter != HashMap_VirtualRegister.end())
	{
		return &Iter->second;
	}
	return nullptr;
}

void HazeStack::InitStackRegister()
{
	HashMap_VirtualRegister =
	{
		//{ADD_REGISTER, nullptr},
		//{SUB_REGISTER, nullptr},
		//{MUL_REGISTER, nullptr},
		//{DIV_REGISTER, nullptr},
		{RET_REGISTER, HazeRegister()},
		{NEW_REGISTER, HazeRegister()},
		{CMP_REGISTER, HazeRegister()},

		{TEMP_REGISTER_0, HazeRegister()},
		{TEMP_REGISTER_1, HazeRegister()},
		{TEMP_REGISTER_2, HazeRegister()},
		{TEMP_REGISTER_3, HazeRegister()},
		{TEMP_REGISTER_4, HazeRegister()},
		{TEMP_REGISTER_5, HazeRegister()},
		{TEMP_REGISTER_6, HazeRegister()},
		{TEMP_REGISTER_7, HazeRegister()},
		{TEMP_REGISTER_8, HazeRegister()},
		{TEMP_REGISTER_9, HazeRegister()},
	};
}

void HazeStack::OnCall(const FunctionData* Info, int ParamSize)
{
	if (Debugger)
	{
		Debugger->AddTempBreakPoint(VM->GetNextLine(VM->GetCurrCallFunctionLine()));
	}

	Stack_Frame.push_back(HazeStackFrame(Info, ParamSize, EBP, ESP - (HAZE_ADDRESS_SIZE + ParamSize)));
	
	EBP = ESP;
	if (Info->Vector_Variable.size() > Info->Vector_Param.size())
	{
		ESP += Info->Vector_Variable.back().Offset + GetSizeByType(Info->Vector_Variable.back().Variable.Type, VM);
	}

	PC = Info->FunctionDescData.InstructionStartAddress;
	--PC;
}

void HazeStack::OnRet()
{
	memcpy(&PC, &(Stack_Main[EBP - HAZE_ADDRESS_SIZE]), HAZE_ADDRESS_SIZE);
	EBP = Stack_Frame.back().EBP;
	ESP = Stack_Frame.back().ESP;
	Stack_Frame.pop_back();
}

void* HazeStack::Alloca(uint32 Size)
{
	void* Ret = HazeMemory::Alloca(Size); //Pool->Alloca(Size);
	if (Ret == nullptr)
	{
		HAZE_LOG_ERR(HAZE_TEXT("Memory pool alloca failed!\n"));
	}

	return Ret;
}


void HazeStack::GarbageCollection(bool Force, bool CollectionAll)
{
	if (Force && CollectionAll)
	{
		VM->GC->ForceGC();
	}
}
