#include "HazeStack.h"

static std::unordered_map<HAZE_STRING, HazeValue*>  HashMap_VirtualRegister = 
{
	{ADD_REGISTER, nullptr},
	{SUB_REGISTER, nullptr},
	{MUL_REGISTER, nullptr},
	{DIV_REGISTER, nullptr},
	{RET_REGISTER, nullptr},
};

HazeStack::HazeStack()
{
	PC = 0;
	EBP = 0;

	Stack_Main.resize(HAZE_VM_STACK_SIZE);

	InitRegisterToStack();
}

HazeStack::~HazeStack()
{
}

HazeValue* HazeStack::GetVirtualRegister(const HAZE_CHAR* Name)
{
	auto Iter = HashMap_VirtualRegister.find(Name);
	if (Iter != HashMap_VirtualRegister.end())
	{
		return Iter->second;
	}
	return nullptr;
}

void HazeStack::Call()
{
	EBP = PC;
}

void HazeStack::Push(const std::vector<InstructionData>& Operator)
{
	if (Operator.size() == 1)
	{
		if (Operator[0].Scope == InstructionScopeType::Global)
		{

		}
		else if (Operator[0].Scope == InstructionScopeType::Local)
		{
			HazeValueType Type = (HazeValueType)Operator[0].Type;
			int Size = GetSize(Type);

			memset(&Stack_Main[PC], 0, Size);

			PC += Size;
		}
	}
}

void HazeStack::Pop()
{
	PC--;
}

void HazeStack::Add(const std::vector<InstructionData>& Operator)
{
	if (Operator.size() == 2)
	{
		HazeValue* AddRegister = GetVirtualRegister(ADD_REGISTER);

		HazeValue* Value1 = GetInstructionValue(Operator[0]);
		HazeValue* Value2 = GetInstructionValue(Operator[1]);

	}
	else
	{

	}
}

void HazeStack::InitRegisterToStack()
{
	for (auto& i : HashMap_VirtualRegister)
	{
		memset(&Stack_Main[PC], 0, sizeof(HazeValue));

		i.second = (HazeValue*)&Stack_Main[PC];

		PC += sizeof(HazeValue);
	}
}

HazeValue* HazeStack::GetInstructionValue(const InstructionData& InsData)
{
	return nullptr;
}