#include "Haze.h"
#include "HazeVM.h"
#include "HazeStack.h"

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> Hash_MapStdLib;


static std::unordered_map<HAZE_STRING, InstructionOpCode> HashMap_String2Code =
{
	{HAZE_TEXT("MOV"), InstructionOpCode::MOV },
	{HAZE_TEXT("ADD"), InstructionOpCode::ADD },
	{HAZE_TEXT("SUB"), InstructionOpCode::SUB },
	{HAZE_TEXT("MUL"), InstructionOpCode::MUL },
	{HAZE_TEXT("DIV"), InstructionOpCode::DIV },
	{HAZE_TEXT("MOD"), InstructionOpCode::MOD },
	{HAZE_TEXT("INC"), InstructionOpCode::INC },
	{HAZE_TEXT("DEC"), InstructionOpCode::DEC },

	{HAZE_TEXT("AND"), InstructionOpCode::AND },
	{HAZE_TEXT("OR"), InstructionOpCode::OR },
	{HAZE_TEXT("NOT"), InstructionOpCode::NOT },
	{HAZE_TEXT("XOR"), InstructionOpCode::XOR },
	{HAZE_TEXT("SHL"), InstructionOpCode::SHL },
	{HAZE_TEXT("SHR"), InstructionOpCode::SHR },

	{HAZE_TEXT("PUSH"), InstructionOpCode::PUSH },
	{HAZE_TEXT("POP"), InstructionOpCode::POP },

	{HAZE_TEXT("CALL"), InstructionOpCode::CALL },
	{HAZE_TEXT("RET"), InstructionOpCode::RET },

	{HAZE_TEXT("NEW"), InstructionOpCode::NEW },
};

std::unordered_map<HAZE_STRING, HazeValue>  HashMap_VirtualRegister =
{
	//{ADD_REGISTER, nullptr},
	//{SUB_REGISTER, nullptr},
	//{MUL_REGISTER, nullptr},
	//{DIV_REGISTER, nullptr},
	{RET_REGISTER, HazeValue()},
	{NEW_REGISTER, HazeValue()},
};

HazeValue* GetVirtualRegister(const HAZE_CHAR* Name)
{
	auto Iter = HashMap_VirtualRegister.find(Name);
	if (Iter != HashMap_VirtualRegister.end())
	{
		return &Iter->second;
	}
	return nullptr;
}

bool IsRegisterScope(HazeDataDesc Scope)
{
	return HazeDataDesc::RegisterBegin < Scope && Scope < HazeDataDesc::RegisterEnd;
}

const HAZE_CHAR* GetInstructionString(InstructionOpCode Code)
{
	static std::unordered_map<InstructionOpCode, const HAZE_CHAR*> HashMap_Code2String;

	if (HashMap_Code2String.size() <= 0)
	{
		for (auto& iter : HashMap_String2Code)
		{
			HashMap_Code2String[iter.second] = iter.first.c_str();
		}
	}

	auto iter = HashMap_Code2String.find(Code);
	if (iter != HashMap_Code2String.end())
	{
		return iter->second;
	}

	return HAZE_TEXT("None");
}

InstructionOpCode GetInstructionByString(const HAZE_STRING& String)
{
	auto iter = HashMap_String2Code.find(String);
	if (iter != HashMap_String2Code.end())
	{
		return iter->second;
	}

	return InstructionOpCode::NONE;
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
			Value.Type = Operator[1].Variable.Type.PrimaryType;
			if (Operator[1].Scope == HazeDataDesc::Constant)
			{
				StringToHazeValueNumber(Operator[1].Variable.Name, Value);
				memcpy(GetAddressByOperator(Stack, Operator[0]), &Value.Value, GetSizeByType(Operator[0].Variable.Type, Stack->VM));
			}
			else if (Operator[1].Scope == HazeDataDesc::Local)
			{
				memcpy(GetAddressByOperator(Stack, Operator[0]), GetAddressByOperator(Stack, Operator[1]), GetSizeByType(Operator[0].Variable.Type, Stack->VM));
			}
			else if (IsRegisterScope(Operator[1].Scope))
			{
				HazeValue* Register = GetVirtualRegister(Operator[1].Variable.Name.c_str());
				int Size = GetSizeByHazeType(Register->Type);

				memcpy(GetAddressByOperator(Stack, Operator[0]) , &Register->Value, Size);
			}
			else if (Operator[1].Scope == HazeDataDesc::ConstantString)
			{
				memcpy(GetAddressByOperator(Stack, Operator[0]), &Operator[1].Extra.Index, GetSizeByType(Operator[1].Variable.Type, Stack->VM));
			}
		}
	}

	static void Call(HazeStack* Stack)
	{
		//EBP = PC;
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			memcpy(&Stack->Stack_Main[Stack->ESP - HAZE_ADDRESS_SIZE], &Stack->PC, HAZE_ADDRESS_SIZE);

			int FunctionIndex = Stack->VM->GetFucntionIndexByName(Operator[0].Variable.Name);
			auto& Function = Stack->VM->Vector_FunctionTable[FunctionIndex];
			if (Function.Extra.FunctionDescData.Type == InstructionFunctionType::HazeFunction)
			{
				Stack->EBP = Stack->ESP;
				Stack->Stack_Function.push_back(Operator[0].Variable.Name);

				int Size = 0;
				for (auto& Iter : Function.Vector_Param)
				{
					Size += GetSizeByType(Iter.Type, Stack->VM);
				}

				Stack->Stack_EBP.push_back(Stack->ESP - (HAZE_ADDRESS_SIZE + Size));

				//unsigned int Index = Stack->VM->GetFucntionIndexByName(Operator[0].Variable.Name);
				Stack->PC = Function.Extra.FunctionDescData.InstructionStartAddress;
				--Stack->PC;
			}
			else
			{
				uint TempEBP = Stack->EBP;
				Stack->EBP = Stack->ESP;

				//±ê×¼¿â²éÕÒ
				Function.Extra.StdLibFunction(Stack, &Function, Operator[0].Extra.FunctionCallParamNum);

				int Size = 0;
				for (auto& Iter : Function.Vector_Param)
				{
					Size += GetSizeByHazeType(Iter.Type.PrimaryType);
				}


				Stack->ESP -= (Size + HAZE_ADDRESS_SIZE);

				Stack->Stack_EBP.push_back(Stack->ESP);

				Stack->EBP = TempEBP;
			}
		}
	}

	static void Push(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			int Size = GetSizeByType(Operator[0].Variable.Type, Stack->VM);

			if (Operator[0].Scope == HazeDataDesc::Constant)
			{
				int N = StringToStandardType<int>(Operator[0].Variable.Name);
				memcpy(&Stack->Stack_Main[Stack->ESP], &N, Size);
			}
			else if (Operator[0].Scope == HazeDataDesc::Address)
			{
				memcpy(&Stack->Stack_Main[Stack->ESP], &Stack->PC, Size);
			}
			else/* if (Operator[0].Scope == InstructionScopeType::Local || Operator[0].Scope == InstructionScopeType::Global)*/
			{
				if (Operator[0].Extra.Address + (int)Stack->EBP >= 0)
				{
					memcpy(&Stack->Stack_Main[Stack->ESP], GetAddressByOperator(Stack, Operator[0]), 
						Operator[0].Scope == HazeDataDesc::ConstantString ? Size = sizeof(Operator[0].Extra.Index) : Size);
				}
				else
				{
					memset(&Stack->Stack_Main[Stack->ESP], 0, Size);
				}
				
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
			Stack->ESP -= GetSizeByHazeType(Operator[0].Variable.Type.PrimaryType);
		}
	}

	static void Add(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::ADD, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Sub(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::SUB, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Mul(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::MUL, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Div(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DIV, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Mod(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::MOD, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Inc(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::INC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Dec(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void And(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Or(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Not(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Xor(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Shl(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{

		}
	}

	static void Shr(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{ 
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
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
			RetRegister->Type = (HazeValueType)Operator[0].Variable.Type.PrimaryType;

			int Size = GetSizeByHazeType(RetRegister->Type);

			memcpy(&RetRegister->Value, GetAddressByOperator(Stack, Operator[0]), Size);
		}

		int FunctionIndex = Stack->VM->GetFucntionIndexByName(Stack->Stack_Function.back());

		int Size = 0;
		for (auto& Iter : Stack->VM->Vector_FunctionTable[FunctionIndex].Vector_Param)
		{
			Size += GetSizeByHazeType(Iter.Type.PrimaryType);
		}

		memcpy(&Stack->PC, &(Stack->Stack_Main[Stack->EBP - 4]), HAZE_ADDRESS_SIZE);

		uint TempEBP = Stack->EBP;
		Stack->Stack_EBP.pop_back();
		Stack->EBP = Stack->Stack_EBP.back();
		Stack->ESP = TempEBP - (HAZE_ADDRESS_SIZE + Size);
	}

	static void New(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			HazeValue* NewRegister = GetVirtualRegister(NEW_REGISTER);
			NewRegister->Type = (HazeValueType)Operator[0].Variable.Type.PrimaryType;

			int Size = GetSizeByHazeType(NewRegister->Type);

			NewRegister->Value.Pointer = Stack->VM->Alloca(NewRegister->Type, Size);
		}
	}

private:
	static char* GetAddressByOperator(HazeStack* Stack, const InstructionData& Operator)
	{
		if (Operator.Scope == HazeDataDesc::Global)
		{
			return (char*)&Stack->VM->GetGlobalValue(Operator.Variable.Name)->Value;
		}
		else if (Operator.Scope == HazeDataDesc::ConstantString)
		{
			return (char*)&Operator.Extra.Index;
		}
		else /*if (Operator.Scope == InstructionScopeType::Local || Operator.Scope == InstructionScopeType::Temp)*/
		{
			return &Stack->Stack_Main[Stack->EBP + Operator.Extra.Address];
		}
	}
};

std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> HashMap_InstructionProcessor =
{
	{InstructionOpCode::MOV, &InstructionProcessor::Mov},

	{InstructionOpCode::ADD, &InstructionProcessor::Add},
	{InstructionOpCode::SUB, &InstructionProcessor::Sub},
	{InstructionOpCode::MUL, &InstructionProcessor::Mul},
	{InstructionOpCode::DIV, &InstructionProcessor::Div},
	{InstructionOpCode::MOD, &InstructionProcessor::Mod},
	{InstructionOpCode::INC, &InstructionProcessor::Inc},
	{InstructionOpCode::DEC, &InstructionProcessor::Dec},

	{InstructionOpCode::AND, &InstructionProcessor::And},
	{InstructionOpCode::OR, &InstructionProcessor::Or},
	{InstructionOpCode::NOT, &InstructionProcessor::Not},
	{InstructionOpCode::XOR, &InstructionProcessor::Xor},

	{InstructionOpCode::SHL, &InstructionProcessor::Shl},
	{InstructionOpCode::SHR, &InstructionProcessor::Shr},

	{InstructionOpCode::PUSH, &InstructionProcessor::Push},
	{InstructionOpCode::POP, &InstructionProcessor::Pop},

	{InstructionOpCode::CALL, &InstructionProcessor::Call},
	{InstructionOpCode::RET, &InstructionProcessor::Ret},
	{InstructionOpCode::NEW, &InstructionProcessor::New},
};