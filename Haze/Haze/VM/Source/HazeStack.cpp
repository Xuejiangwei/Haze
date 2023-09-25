#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeInstruction.h"
#include "HazeMemory.h"

#include "HazeDebugger.h"

extern std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> HashMap_InstructionProcessor;
extern std::unique_ptr<HazeDebugger> Debugger;

HazeStack::HazeStack(HazeVM* m_VM) : m_VM(m_VM)
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

	Run();

	Debugger->SendProgramEnd();
	GarbageCollection(true, true);
}

void HazeStack::JmpTo(const InstructionData& m_Data)
{
	auto& Function = Stack_Frame.back().FunctionInfo;
	int Address = Function->FunctionDescData.InstructionStartAddress + m_Data.Extra.Jmp.StartAddress;
	memcpy(&PC, &Address, sizeof(PC));

	PC--;
}

void HazeStack::Run(bool IsHazeCall)
{
	while (PC < m_VM->Instructions.size())
	{
		while (m_VM->IsDebug())
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

		auto Iter = HashMap_InstructionProcessor.find(m_VM->Instructions[PC].InsCode);
		if (Iter != HashMap_InstructionProcessor.end())
		{
			Iter->second(this);
		}
		else
		{
			return;
		}

		if (IsHazeCall && Vector_CallHazeStack.size() == 0)
		{
			return;
		}

		PCStepInc();
	}
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
	auto& MainFunction = m_VM->GetFunctionByName(HAZE_MAIN_FUNCTION_TEXT);
	OnCall(&MainFunction, 0);
	PCStepInc();

	/*ESP -= HAZE_ADDRESS_SIZE;
	EBP -= HAZE_ADDRESS_SIZE;

	if (MainFunction.Vector_Variable.size() > 0)
	{
		ESP += MainFunction.Vector_Variable.back().Offset + MainFunction.Vector_Variable.back().Size;
	}*/
}

HazeRegister* HazeStack::GetVirtualRegister(const HAZE_CHAR* m_Name)
{
	auto Iter = HashMap_VirtualRegister.find(m_Name);
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
		Debugger->AddTempBreakPoint(m_VM->GetNextLine(m_VM->GetCurrCallFunctionLine()));
	}

	RegisterData RegisterDara({ GetVirtualRegister(CMP_REGISTER)->m_Data });
	Stack_Frame.push_back(HazeStackFrame(Info, ParamSize, EBP, ESP - (HAZE_ADDRESS_SIZE + ParamSize), RegisterDara));

	EBP = ESP;
	if (Info->Variables.size() > Info->Params.size())
	{
		ESP += Info->Variables.back().Offset + GetSizeByType(Info->Variables.back().Variable.m_Type, m_VM);
	}

	PC = Info->FunctionDescData.InstructionStartAddress;
	--PC;

	AddCallHazeTimes();

	if (Debugger)
	{
		m_VM->OnExecLine(Stack_Frame.back().FunctionInfo->FunctionDescData.StartLine);
	}
}

void HazeStack::OnRet()
{
	if (Debugger)
	{
		m_VM->OnExecLine(Stack_Frame.back().FunctionInfo->FunctionDescData.EndLine);
		while (Debugger->IsPause) 
		{
		}
	}

	memcpy(&PC, &(Stack_Main[EBP - HAZE_ADDRESS_SIZE]), HAZE_ADDRESS_SIZE);
	EBP = Stack_Frame.back().EBP;
	ESP = Stack_Frame.back().ESP;

	memcpy(GetVirtualRegister(CMP_REGISTER)->m_Data.begin()._Unwrapped(), Stack_Frame.back().Register.Cmp_RegisterData.begin()._Unwrapped(),
		Stack_Frame.back().Register.Cmp_RegisterData.size());

	Stack_Frame.pop_back();

	SubCallHazeTimes();
}

void HazeStack::ResetCallHaze()
{
	Vector_CallHazeStack.clear();
	Vector_CallHazeStack.push_back(1);

	Run(true);
}

void HazeStack::AddCallHazeTimes()
{
	if (Vector_CallHazeStack.size() > 0)
	{
		Vector_CallHazeStack.push_back(1);
	}
}

void HazeStack::SubCallHazeTimes()
{
	if (Vector_CallHazeStack.size() > 0)
	{
		Vector_CallHazeStack.pop_back();
	}
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
		HazeMemory::GetMemory()->ForceGC();
	}
}