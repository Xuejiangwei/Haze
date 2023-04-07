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

std::unordered_map<HAZE_STRING, HazeRegister>  HashMap_VirtualRegister =
{
	//{ADD_REGISTER, nullptr},
	//{SUB_REGISTER, nullptr},
	//{MUL_REGISTER, nullptr},
	//{DIV_REGISTER, nullptr},
	{RET_REGISTER, HazeRegister()},
	{NEW_REGISTER, HazeRegister()},
};

HazeRegister* GetVirtualRegister(const HAZE_CHAR* Name)
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
			char* Dst = GetAddressByOperator(Stack, Operator[0]);
			char* Src = GetAddressByOperator(Stack, Operator[1]);
			memcpy(Dst, Src, GetSizeByType(Operator[0].Variable.Type, Stack->VM));
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
				uint32 TempEBP = Stack->EBP;
				Stack->EBP = Stack->ESP;

				//±ê×¼¿â²éÕÒ
				Function.Extra.StdLibFunction(Stack, &Function, Operator[0].Extra.Call.ParamNum);

				Stack->ESP -= (Operator[0].Extra.Call.ParamByteSize + HAZE_ADDRESS_SIZE);

				//Stack->Stack_EBP.push_back(Stack->ESP);

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
			else if (Operator[0].Scope == HazeDataDesc::ClassThis)
			{
				uint64 Address = (uint64)GetAddressByOperator(Stack, Operator[0]);
				memcpy(&Stack->Stack_Main[Stack->ESP], &Address,sizeof(uint64));
			}
			else/* if (Operator[0].Scope == InstructionScopeType::Local || Operator[0].Scope == InstructionScopeType::Global)*/
			{
				if (Operator[0].Extra.Address.BaseAddress + (int)Stack->EBP >= 0)
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
			Stack->ESP -= GetSizeByType(Operator[0].Variable.Type, Stack->VM);
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
			HazeRegister* RetRegister = GetVirtualRegister(RET_REGISTER);
			RetRegister->Type = Operator[0].Variable.Type;

			int Size = GetSizeByType(RetRegister->Type, Stack->VM);

			RetRegister->Data.resize(Size);
			memcpy(RetRegister->Data.begin()._Unwrapped(), GetAddressByOperator(Stack, Operator[0]), Size);
		}


		int FunctionIndex = Stack->VM->GetFucntionIndexByName(Stack->Stack_Function.back());

		Stack->Stack_Function.pop_back();

		int Size = 0;
		for (auto& Iter : Stack->VM->Vector_FunctionTable[FunctionIndex].Vector_Param)
		{
			Size += GetSizeByType(Iter.Type, Stack->VM);
		}

		memcpy(&Stack->PC, &(Stack->Stack_Main[Stack->EBP - 4]), HAZE_ADDRESS_SIZE);

		uint32 TempEBP = Stack->EBP;
		Stack->Stack_EBP.pop_back();
		Stack->EBP = Stack->Stack_EBP.back();
		Stack->ESP = TempEBP - (HAZE_ADDRESS_SIZE + Size);
	}

	static void New(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			HazeRegister* NewRegister = GetVirtualRegister(NEW_REGISTER);
			NewRegister->Type = Operator[0].Variable.Type;

			int Size = GetSizeByType(NewRegister->Type, Stack->VM);

			uint64 Address = (uint64)Stack->VM->Alloca(NewRegister->Type.PrimaryType, Size);

			NewRegister->Data.resize(Size);
			memcpy(NewRegister->Data.begin()._Unwrapped(), &Address, Size);
		}
	}

private:
	static char* GetAddressByOperator(HazeStack* Stack, const InstructionData& Operator)
	{
		static HazeValue ConstantValue;

		if (Operator.Scope == HazeDataDesc::Constant)
		{
			ConstantValue.Type = Operator.Variable.Type.PrimaryType;
			StringToHazeValueNumber(Operator.Variable.Name, ConstantValue);
			return (char*)&ConstantValue.Value;
		}
		else if (Operator.Scope == HazeDataDesc::Global)
		{
			return (char*)&Stack->VM->GetGlobalValue(Operator.Variable.Name)->Value;
		}
		else if (Operator.Scope == HazeDataDesc::ConstantString)
		{
			return (char*)&Operator.Extra.Index;
		}
		else if (IsRegisterScope(Operator.Scope))
		{
			HazeRegister* Register = GetVirtualRegister(Operator.Variable.Name.c_str());
			return Register->Data.begin()._Unwrapped();
		}
		else if (Operator.Scope == HazeDataDesc::ClassMember_Local_Public && Operator.AddressType == InstructionAddressType::Pointer_Offset)
		{
			uint64 Address;
			memcpy(&Address, &Stack->Stack_Main[Stack->EBP + Operator.Extra.Address.BaseAddress], sizeof(uint64));

			return (char*)Address + Operator.Extra.Address.Offset;
		}
		else /*if (Operator.Scope == InstructionScopeType::Local || Operator.Scope == InstructionScopeType::Temp)*/
		{
			return &Stack->Stack_Main[Stack->EBP + Operator.Extra.Address.BaseAddress];
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