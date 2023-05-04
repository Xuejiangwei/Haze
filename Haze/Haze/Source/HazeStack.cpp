#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "MemoryPool.h"
#include "MemoryBucket.h"

extern std::unordered_map<HAZE_STRING, HazeValue*> HashMap_VirtualRegister;
extern std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> HashMap_InstructionProcessor;

thread_local static std::unique_ptr<MemoryPool> Pool = std::make_unique<MemoryPool>();

HazeStack::HazeStack(HazeVM* VM) : VM(VM)
{
	PC = 0;
	EBP = 0;

	Stack_Main.resize(HAZE_VM_STACK_SIZE);
	InitRegisterToStack();

	/*std::cout << MemoryPool::RoundUp(1) << std::endl;
	std::cout << MemoryPool::RoundUp(8) << std::endl;
	std::cout << MemoryPool::RoundUp(16) << std::endl;
	std::cout << MemoryPool::RoundUp(32) << std::endl;
	std::cout << MemoryPool::RoundUp(64) << std::endl;
	std::cout << MemoryPool::RoundUp(128) << std::endl;
	std::cout << MemoryPool::RoundUp(256) << std::endl;
	std::cout << MemoryPool::RoundUp(512) << std::endl;*/

	std::cout << MemoryPool::Index(4) << std::endl;
	std::cout << MemoryPool::Index(8) << std::endl;
	std::cout << MemoryPool::Index(16) << std::endl;
	std::cout << MemoryPool::Index(32) << std::endl;
	std::cout << MemoryPool::Index(64) << std::endl;
	std::cout << MemoryPool::Index(128) << std::endl;
	std::cout << MemoryPool::Index(256) << std::endl;
	std::cout << MemoryPool::Index(512) << std::endl;
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

void HazeStack::PreMainFunction()
{
	int FaultPC = -2;
	memcpy(&Stack_Main[ESP], &FaultPC, HAZE_ADDRESS_SIZE);
	ESP += HAZE_ADDRESS_SIZE;
	EBP = ESP;

	Stack_EBP.push_back(ESP - HAZE_ADDRESS_SIZE);
}

void HazeStack::PushMainFuntion()
{
	ESP += HAZE_ADDRESS_SIZE;
	EBP = ESP;

	auto& MainFunction = VM->GetFunctionByName(HAZE_MAIN_FUNCTION_TEXT);
	OnCall(&MainFunction, 0);
	PCStepInc();

	ESP -= HAZE_ADDRESS_SIZE;
	EBP -= HAZE_ADDRESS_SIZE;

	/*if (MainFunction.Vector_Variable.size() > 0)
	{
		ESP += MainFunction.Vector_Variable.back().Offset + MainFunction.Vector_Variable.back().Size;
	}*/
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

void HazeStack::OnCall(const FunctionData* Info, int ParamSize)
{
	EBP = ESP;
	Stack_Frame.push_back(HazeStackFrame(Info, ParamSize));
	Stack_EBP.push_back(ESP - (HAZE_ADDRESS_SIZE + ParamSize));

	if (Info->Vector_Variable.size() > Info->Vector_Param.size())
	{
		ESP += Info->Vector_Variable.back().Offset + GetSizeByType(Info->Vector_Variable.back().Variable.Type, VM);
	}

	PC = Info->Extra.FunctionDescData.InstructionStartAddress;
	--PC;
}

void HazeStack::OnRet()
{
	memcpy(&PC, &(Stack_Main[EBP - HAZE_ADDRESS_SIZE]), HAZE_ADDRESS_SIZE);

	//uint32 TempEBP = EBP;
	ESP = Stack_EBP.back();
	Stack_EBP.pop_back();
	EBP = Stack_EBP.back();
	//ESP = TempEBP - (HAZE_ADDRESS_SIZE + Stack_Frame.back().FunctionParamSize);

	Stack_Frame.pop_back();
}

void* HazeStack::Alloca(unsigned int Size)
{
	void* Ret = Pool->Alloca(Size);
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
		Pool->ReleaseAll();
	}
}
