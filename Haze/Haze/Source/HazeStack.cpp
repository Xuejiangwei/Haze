#include "HazeVM.h"
#include "HazeStack.h"

extern std::unordered_map<HAZE_STRING, HazeValue*> HashMap_VirtualRegister;
extern std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> HashMap_InstructionProcessor;

HazeStack::HazeStack(HazeVM* VM) : VM(VM)
{
	PC = 0;
	EBP = 0;

	Stack_Main.resize(HAZE_VM_STACK_SIZE);
	InitRegisterToStack();
}

HazeStack::~HazeStack()
{
}


void HazeStack::Start(unsigned int Address)
{
	PC = Address;

	PushMainFuntion();

	while (PC < VM->Vector_Instruction.size())
	{
#ifdef _DEBUG
		auto Iter = HashMap_InstructionProcessor.find(VM->Vector_Instruction[PC].InsCode);
		if (Iter != HashMap_InstructionProcessor.end())
		{
			Iter->second(this);
		}
		else
		{
			return;
		}
#else
		HashMap_InstructionProcessor[Vector_Instruction[PC].InsCode](this);
#endif // DEBUG

		PCStepInc();
	}
}

//void HazeStack::PushVariableStack(HazeDefineVariable* Variable)
//{
//	Stack_Frame.back().FunctionData.Vector_LocalParam.push_back(Variable);
//}
//
//void HazeStack::PopVariableStack(int Num)
//{
//	uint64 NewSize = Stack_Frame.back().FunctionData.Vector_LocalParam.size() - Num;
//	Stack_Frame.back().FunctionData.Vector_LocalParam.resize(NewSize);
//}

void HazeStack::JmpTo(const InstructionData& Data)
{
	auto& Function = Stack_Frame.back().FunctionInfo;
	int Address = Function->Extra.FunctionDescData.InstructionStartAddress + Data.Extra.Jmp.StartAddress;
	memcpy(&PC, &Address, sizeof(PC));

	PC--;
}

void HazeStack::PushLoopStack()
{
	Stack_Frame.back().Vector_LoopStack.push_back(PC);
}

void HazeStack::PopLoopStack()
{
	auto& Stack = Stack_Frame.back().Vector_LoopStack;
	PC = Stack.back();
	Stack.pop_back();
}

void HazeStack::PCStepInc()
{
	++PC;
}

void HazeStack::PushMainFuntion()
{
	int FaultPC = -2;
	memcpy(&Stack_Main[ESP], &FaultPC, HAZE_ADDRESS_SIZE);
	
	ESP += HAZE_ADDRESS_SIZE;

	Stack_EBP.push_back(EBP);
	Stack_EBP.push_back(ESP);

	EBP = ESP;

	auto& MainFunction = VM->GetFunctionByName(HAZE_MAIN_FUNCTION_TEXT);
	OnCall(&MainFunction);
}

void HazeStack::InitRegisterToStack()
{
	/*for (auto& i : HashMap_VirtualRegister)
	{
		memset(&Stack_Main[ESP], 0, sizeof(HazeValue));

		i.second = (HazeValue*)&Stack_Main[ESP];

		ESP += sizeof(HazeValue);
	}*/

}

void HazeStack::OnCall(const FunctionData* Info)
{
	Stack_Frame.push_back(HazeStackFrame(Info));
}

void HazeStack::OnRet()
{
	Stack_Frame.pop_back();
}
