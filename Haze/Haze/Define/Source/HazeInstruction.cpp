#include "Haze.h"
#include "HazeVM.h"
#include "HazeStack.h"

std::unordered_map<HAZE_STRING, HazeValue*>  HashMap_VirtualRegister =
{
	//{ADD_REGISTER, nullptr},
	//{SUB_REGISTER, nullptr},
	//{MUL_REGISTER, nullptr},
	//{DIV_REGISTER, nullptr},
	{RET_REGISTER, nullptr},
};

HazeValue* GetVirtualRegister(const HAZE_CHAR* Name)
{
	auto Iter = HashMap_VirtualRegister.find(Name);
	if (Iter != HashMap_VirtualRegister.end())
	{
		return Iter->second;
	}
	return nullptr;
}

HazeValue* GetInstructionValue(const InstructionData& InsData)
{
	return nullptr;
}

class InstructionProcessor
{
public:
	static void Mov(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{

			HazeValue Value;
			Value.Type = Operator[1].Type;
			if (Operator[1].Scope == InstructionScopeType::Constant)
			{
				StringToHazeValueNumber(Operator[1].Name, Value);
				memcpy(GetAddressByOperator(Stack, Operator[0]), &Value.Value, GetSize(Operator[0].Type));
			}
			else if (Operator[1].Scope == InstructionScopeType::Local)
			{
				memcpy(GetAddressByOperator(Stack, Operator[0]), &Stack->Stack_Main[Stack->EBP + Operator[1].IndexOrOffset], GetSize(Operator[0].Type));
			}
			else if (Operator[1].Scope == InstructionScopeType::Register)
			{
				HazeValue* RetRegister = GetVirtualRegister(RET_REGISTER);
				int Size = GetSize(RetRegister->Type);

				memcpy(GetAddressByOperator(Stack, Operator[0]) , &RetRegister->Value, Size);
			}
		}
	}

	static void Call(HazeStack* Stack)
	{
		//EBP = PC;
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			memcpy(&Stack->Stack_Main[Stack->ESP - HAZE_PUSH_ADDRESS_SIZE], &Stack->PC, HAZE_PUSH_ADDRESS_SIZE);

			Stack->EBP = Stack->ESP;
			Stack->Stack_Function.push_back(Operator[0].Name);

			int FunctionIndex = Stack->VM->GetFucntionIndexByName(Operator[0].Name);
			int Size = 0;
			for (auto& Iter : Stack->VM->Vector_FunctionTable[FunctionIndex].Vector_Param)
			{
				Size += GetSize(Iter.second.Type);
			}

			Stack->Stack_EBP.push_back(Stack->ESP - (HAZE_PUSH_ADDRESS_SIZE + Size));

			unsigned int Index = Stack->VM->GetFucntionIndexByName(Operator[0].Name);
			Stack->PC = Stack->VM->Vector_FunctionTable[Index].InstructionStartAddress;
			--Stack->PC;
		}
	}

	static void Push(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			HazeValueType Type = (HazeValueType)Operator[0].Type;
			int Size = GetSize(Type);

			if (Operator[0].Scope == InstructionScopeType::Constant)
			{
				int N = StringToInt<int>(Operator[0].Name);
				memcpy(&Stack->Stack_Main[Stack->ESP], &N, Size);
			}
			/*else if (Operator[0].Scope == InstructionScopeType::Address)
			{
				memcpy(&Stack->Stack_Main[Stack->ESP], &Stack->PC, Size);
			}*/
			else
			{
				memset(&Stack->Stack_Main[Stack->ESP], 0, Size);
			}


			Stack->ESP += Size;
			/*if (Operator[0].Scope == InstructionScopeType::Global)
			{

			}
			else if (Operator[0].Scope == InstructionScopeType::Local)
			{
				HazeValueType Type = (HazeValueType)Operator[0].Type;
				int Size = GetSize(Type);

				memset(&Stack_Main[ESP], 0, Size);

				ESP += Size;
			}*/
		}
	}

	static void Pop(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			Stack->ESP -= GetSize(Operator[0].Type);
		}
	}

	static void Add(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Type))
			{
				AddValueByType(Operator[0].Type, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Ret(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			HazeValue* RetRegister = GetVirtualRegister(RET_REGISTER);
			RetRegister->Type = (HazeValueType)Operator[0].Type;

			int Size = GetSize(RetRegister->Type);

			memcpy(&RetRegister->Value, GetAddressByOperator(Stack, Operator[0]), Size);
		}

		int FunctionIndex = Stack->VM->GetFucntionIndexByName(Stack->Stack_Function.back());

		int Size = 0;
		for (auto& Iter : Stack->VM->Vector_FunctionTable[FunctionIndex].Vector_Param)
		{
			Size += GetSize(Iter.second.Type);
		}

		memcpy(&Stack->PC, &(Stack->Stack_Main[Stack->EBP - 4]), HAZE_PUSH_ADDRESS_SIZE);

		Stack->Stack_EBP.pop_back();
		Stack->EBP = Stack->Stack_EBP.back();
		Stack->ESP -= (HAZE_PUSH_ADDRESS_SIZE + Size);
	}

private:
	static char* GetAddressByOperator(HazeStack* Stack, const InstructionData& Operator)
	{
		if (Operator.Scope == InstructionScopeType::Global)
		{
			return (char*)&Stack->VM->GetGlobalValue(Operator.Name)->Value;
		}
		else if (Operator.Scope == InstructionScopeType::Local || Operator.Scope == InstructionScopeType::Temp)
		{
			return &Stack->Stack_Main[Stack->EBP + Operator.IndexOrOffset];
		}

		return nullptr;
	}
};

std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> HashMap_InstructionProcessor =
{
	{InstructionOpCode::MOV, &InstructionProcessor::Mov},
	{InstructionOpCode::CALL, &InstructionProcessor::Call},
	{InstructionOpCode::PUSH, &InstructionProcessor::Push},
	{InstructionOpCode::POP, &InstructionProcessor::Pop},
	{InstructionOpCode::ADD, &InstructionProcessor::Add},
	{InstructionOpCode::RET, &InstructionProcessor::Ret},
};