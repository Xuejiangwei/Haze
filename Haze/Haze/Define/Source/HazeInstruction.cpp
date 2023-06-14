#include "Haze.h"
#include "HazeLog.h"
#include "HazeVM.h"
#include "HazeStack.h"

#define HAZE_CALL_LOG				0

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> Hash_MapStdLib;

static std::unordered_map<HAZE_STRING, InstructionOpCode> HashMap_String2Code =
{
	{HAZE_TEXT("MOV"), InstructionOpCode::MOV },
	{HAZE_TEXT("MOVPV"), InstructionOpCode::MOVPV },
	{HAZE_TEXT("MOVTOPV"), InstructionOpCode::MOVTOPV },
	{HAZE_TEXT("LEA"), InstructionOpCode::LEA },
	{HAZE_TEXT("ADD"), InstructionOpCode::ADD },
	{HAZE_TEXT("SUB"), InstructionOpCode::SUB },
	{HAZE_TEXT("MUL"), InstructionOpCode::MUL },
	{HAZE_TEXT("DIV"), InstructionOpCode::DIV },
	{HAZE_TEXT("MOD"), InstructionOpCode::MOD },

	{HAZE_TEXT("NEG"), InstructionOpCode::NEG },
	{HAZE_TEXT("NOT"), InstructionOpCode::NOT },
	{HAZE_TEXT("INC"), InstructionOpCode::INC },
	{HAZE_TEXT("DEC"), InstructionOpCode::DEC},

	{HAZE_TEXT("BIT_AND"), InstructionOpCode::BIT_AND },
	{HAZE_TEXT("BIT_OR"), InstructionOpCode::BIT_OR },
	{HAZE_TEXT("BIT_NEG"), InstructionOpCode::BIT_NEG },
	{HAZE_TEXT("BIT_XOR"), InstructionOpCode::BIT_XOR },
	{HAZE_TEXT("SHL"), InstructionOpCode::SHL },
	{HAZE_TEXT("SHR"), InstructionOpCode::SHR },

	{HAZE_TEXT("ADD_ASSIGN"), InstructionOpCode::ADD_ASSIGN },
	{HAZE_TEXT("SUB_ASSIGN"), InstructionOpCode::SUB_ASSIGN },
	{HAZE_TEXT("MUL_ASSIGN"), InstructionOpCode::MUL_ASSIGN },
	{HAZE_TEXT("DIV_ASSIGN"), InstructionOpCode::DIV_ASSIGN },
	{HAZE_TEXT("MOD_ASSIGN"), InstructionOpCode::MOD_ASSIGN },
	{HAZE_TEXT("BIT_AND_ASSIGN"), InstructionOpCode::BIT_AND_ASSIGN },
	{HAZE_TEXT("BIT_OR_ASSIGN"), InstructionOpCode::BIT_OR_ASSIGN },
	{HAZE_TEXT("BIT_XOR_ASSIGN"), InstructionOpCode::BIT_XOR_ASSIGN },
	{HAZE_TEXT("SHL_ASSIGN"), InstructionOpCode::SHL_ASSIGN },
	{HAZE_TEXT("SHR_ASSIGN"), InstructionOpCode::SHR_ASSIGN },

	{HAZE_TEXT("PUSH"), InstructionOpCode::PUSH },
	{HAZE_TEXT("POP"), InstructionOpCode::POP },

	{HAZE_TEXT("CALL"), InstructionOpCode::CALL },
	{HAZE_TEXT("RET"), InstructionOpCode::RET },

	{HAZE_TEXT("NEW"), InstructionOpCode::NEW },

	{HAZE_TEXT("CMP"), InstructionOpCode::CMP },
	{HAZE_TEXT("JMP"), InstructionOpCode::JMP },
	{HAZE_TEXT("JNE"), InstructionOpCode::JNE },
	{HAZE_TEXT("JNG"), InstructionOpCode::JNG },
	{HAZE_TEXT("JNL"), InstructionOpCode::JNL },
	{HAZE_TEXT("JE"), InstructionOpCode::JE },
	{HAZE_TEXT("JG"), InstructionOpCode::JG },
	{HAZE_TEXT("JL"), InstructionOpCode::JL },


	{HAZE_TEXT("LINE"), InstructionOpCode::LINE },
};

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

bool IsJmpOpCode(InstructionOpCode Code)
{
	return Code >= InstructionOpCode::JMP && Code <= InstructionOpCode::JL;
}

bool IsClassMember(HazeDataDesc Scope)
{
	return Scope >= HazeDataDesc::ClassMember_Local_Public && Scope <= HazeDataDesc::ClassMember_Local_Protected;
}


class InstructionProcessor
{
public:
	static void Mov(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			void* Dst = GetAddressByOperator(Stack, Operator[0]);
			const void* Src = GetAddressByOperator(Stack, Operator[1]);
			memcpy(Dst, Src, GetSizeByType(Operator[0].Variable.Type, Stack->VM));
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void MovPV(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			void* Dst = GetAddressByOperator(Stack, Operator[0]);
			const void* Src = GetAddressByOperator(Stack, Operator[1]);

			uint64 Address = 0;
			memcpy(&Address, Src, sizeof(Address));
			memcpy(Dst, (void*)Address, GetSizeByType(Operator[0].Variable.Type, Stack->VM));
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void MovToPV(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			void* Dst = GetAddressByOperator(Stack, Operator[0]);
			const void* Src = GetAddressByOperator(Stack, Operator[1]);

			uint64 Address = 0;
			memcpy(&Address, Dst, sizeof(Address));
			memcpy((void*)Address, Src, GetSizeByType(Operator[1].Variable.Type, Stack->VM));
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Lea(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
 			void* Dst = GetAddressByOperator(Stack, Operator[0]);
			uint64 Address = (uint64)GetAddressByOperator(Stack, Operator[1]);
			memcpy(Dst, &Address, GetSizeByType(Operator[0].Variable.Type, Stack->VM));
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

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
				memcpy(&Stack->Stack_Main[Stack->ESP], &Address, sizeof(uint64));
			}
			else/* if (Operator[0].Scope == InstructionScopeType::Local || Operator[0].Scope == InstructionScopeType::Global)*/
			{
				if (Operator[0].Extra.Address.BaseAddress + (int)Stack->EBP >= 0)
				{
					//Size = Operator[0].Scope == HazeDataDesc::ConstantString ? Size = sizeof(Operator[0].Extra.Index) : Size;
					memcpy(&Stack->Stack_Main[Stack->ESP], GetAddressByOperator(Stack, Operator[0]), Size);
				}
				else
				{
					memset(&Stack->Stack_Main[Stack->ESP], 0, Size);
				}
				
			}

			Stack->ESP += Size;
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Pop(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			Stack->ESP -= GetSizeByType(Operator[0].Variable.Type, Stack->VM);
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Add(HazeStack* Stack)
	{
		BinaryOperator(Stack);

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Sub(HazeStack* Stack)
	{
		BinaryOperator(Stack);

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Mul(HazeStack* Stack)
	{
		BinaryOperator(Stack);

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Div(HazeStack* Stack)
	{
		BinaryOperator(Stack);

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

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

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Neg(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::NEG, GetAddressByOperator(Stack, Operator[0]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Not(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::NOT, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Inc(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				OperatorValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::INC, GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Dec(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				OperatorValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DEC, GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Bit_And(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_AND, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Bit_Or(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_OR, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Bit_Xor(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_XOR, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Add_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::ADD_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Sub_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::SUB_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Mul_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::MUL_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Div_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::DIV_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Mod_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::MOD_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Bit_And_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_AND_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Bit_Or_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_OR_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Bit_Neg(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			if (IsIntegerType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_NEG, GetAddressByOperator(Stack, Operator[0]), GetAddressByOperator(Stack, Operator[0]));
			}
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("bir neg operator error!\n"));
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Bit_Xor_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_XOR_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Shl_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::SHL_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Shr_Assign(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::SHR_ASSIGN, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Shl(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType) && IsIntegerType(Operator[1].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::SHL, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Shr(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{ 
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, InstructionOpCode::SHR, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Call(HazeStack* Stack)
	{
		//EBP = PC;
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() >= 1)
		{
#if HAZE_CALL_LOG
			HAZE_LOG_INFO(HAZE_TEXT("µ÷ÓÃº¯Êý<%s>\n"), Operator[0].Variable.Name.c_str());
#endif

#if HAZE_DEBUG_ENABLE

			Stack->VM->InstructionExecPost();

#endif

			memcpy(&Stack->Stack_Main[Stack->ESP - HAZE_ADDRESS_SIZE], &Stack->PC, HAZE_ADDRESS_SIZE);

			if (Operator[0].Variable.Type.PrimaryType == HazeValueType::PointerFunction)
			{
				void* Value = GetAddressByOperator(Stack, Operator[1]);
				uint64 FunctionAddress;
				memcpy(&FunctionAddress, Value, sizeof(FunctionAddress));
				Stack->OnCall((FunctionData*)FunctionAddress, Operator[0].Extra.Call.ParamByteSize);
			}
			else
			{
				int FunctionIndex = Stack->VM->GetFucntionIndexByName(Operator[0].Variable.Name);
				auto& Function = Stack->VM->Vector_FunctionTable[FunctionIndex];

				if (Function.Extra.FunctionDescData.Type == InstructionFunctionType::HazeFunction)
				{
					Stack->OnCall(&Function, Operator[0].Extra.Call.ParamByteSize);
				}
				else
				{
					uint32 TempEBP = Stack->EBP;
					Stack->EBP = Stack->ESP;

					Function.Extra.StdLibFunction(Stack, &Function, Operator[0].Extra.Call.ParamNum);

					Stack->ESP -= (Operator[0].Extra.Call.ParamByteSize + HAZE_ADDRESS_SIZE);
					Stack->EBP = TempEBP;
				}
			}
		}
	}

	static void Ret(HazeStack* Stack)
	{
#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			HazeRegister* RetRegister = Stack->GetVirtualRegister(RET_REGISTER);
			RetRegister->Type = Operator[0].Variable.Type;

			int Size = GetSizeByType(RetRegister->Type, Stack->VM);

			RetRegister->Data.resize(Size);
			memcpy(RetRegister->Data.begin()._Unwrapped(), GetAddressByOperator(Stack, Operator[0]), Size);
		}
		
		Stack->OnRet();
	}

	static void New(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			HazeRegister* NewRegister = Stack->GetVirtualRegister(NEW_REGISTER);
			NewRegister->Type = Operator[0].Variable.Type;

			int Size = GetSizeByType(NewRegister->Type, Stack->VM);

			uint64 Address = (uint64)Stack->Alloca(Size);

			NewRegister->Data.resize(Size);
			memcpy(NewRegister->Data.begin()._Unwrapped(), &Address, Size);
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Cmp(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			HazeRegister* CmpRegister = Stack->GetVirtualRegister(CMP_REGISTER);
			CompareValueByType(GetStrongerType(Operator[0].Variable.Type.PrimaryType, Operator[1].Variable.Type.PrimaryType),
				CmpRegister, GetAddressByOperator(Stack, Operator[0]), GetAddressByOperator(Stack, Operator[1]));
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Jmp(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			JmpToOperator(Stack, Operator[0]);
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Jne(HazeStack* Stack)
	{
#define REGISTER_EQUAL(R) R->Data[0] == 1
#define REGISTER_GREATER(R) R->Data[1] == 1
#define REGISTER_LESS(R) R->Data[2] == 1

		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			HazeRegister* CmpRegister = Stack->GetVirtualRegister(CMP_REGISTER);

			if (!REGISTER_EQUAL(CmpRegister))
			{
				JmpToOperator(Stack, Operator[0]);
			}
			else
			{
				JmpToOperator(Stack, Operator[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Jng(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			HazeRegister* CmpRegister = Stack->GetVirtualRegister(CMP_REGISTER);

			if (!REGISTER_GREATER(CmpRegister))
			{
				JmpToOperator(Stack, Operator[0]);
			}
			else
			{
				JmpToOperator(Stack, Operator[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Jnl(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			HazeRegister* CmpRegister = Stack->GetVirtualRegister(CMP_REGISTER);

			if (!REGISTER_LESS(CmpRegister))
			{
				JmpToOperator(Stack, Operator[0]);
			}
			else
			{
				JmpToOperator(Stack, Operator[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Je(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			HazeRegister* CmpRegister = Stack->GetVirtualRegister(CMP_REGISTER);

			if (REGISTER_EQUAL(CmpRegister))
			{
				JmpToOperator(Stack, Operator[0]);
			}
			else
			{
				JmpToOperator(Stack, Operator[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Jg(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			HazeRegister* CmpRegister = Stack->GetVirtualRegister(CMP_REGISTER);

			if (REGISTER_GREATER(CmpRegister))
			{
				JmpToOperator(Stack, Operator[0]);
			}
			else
			{
				JmpToOperator(Stack, Operator[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Jl(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 2)
		{
			HazeRegister* CmpRegister = Stack->GetVirtualRegister(CMP_REGISTER);

			if (REGISTER_LESS(CmpRegister))
			{
				JmpToOperator(Stack, Operator[0]);
			}
			else
			{
				JmpToOperator(Stack, Operator[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif

	}

	static void Line(HazeStack* Stack)
	{
		const auto& Operator = Stack->VM->Vector_Instruction[Stack->PC].Operator;
		if (Operator.size() == 1)
		{
			Stack->VM->OnExecLine(Operator[0].Extra.Line);
		}

#if HAZE_DEBUG_ENABLE

		Stack->VM->InstructionExecPost();

#endif
	}

private:
	static void* const GetAddressByOperator(HazeStack* Stack, const InstructionData& Operator)
	{
#define HAZE_VM_GET_ADDRESS_LOG 0

#if HAZE_VM_GET_ADDRESS_LOG
		HAZE_STRING_STREAM HSS;
		HSS << "Address " << Operator.Variable.Name << " ";
#endif

		void* Ret = nullptr;
		thread_local static HazeVariable ConstantValue;
		thread_local static uint64 TempAddress;

		if (Operator.Scope == HazeDataDesc::Constant || Operator.Scope == HazeDataDesc::NullPtr)
		{
			auto& Type = const_cast<HazeDefineType&>(ConstantValue.GetType());
			auto& Value = const_cast<HazeValue&>(ConstantValue.GetValue());

			Type.PrimaryType = Operator.Variable.Type.PrimaryType;
			StringToHazeValueNumber(Operator.Variable.Name, Type.PrimaryType, Value);
			Ret = GetBinaryPointer(Type.PrimaryType, Value);
		}
		else if (Operator.Scope == HazeDataDesc::Global)
		{
			Ret = (void*)&Stack->VM->GetGlobalValue(Operator.Variable.Name)->Value;
		}
		else if (Operator.Scope == HazeDataDesc::ConstantString)
		{
			TempAddress = (uint64)&Stack->GetVM()->GetHazeStringByIndex(Operator.Extra.Index);
			Ret = &TempAddress;
		}
		else if (IsRegisterScope(Operator.Scope))
		{
			HazeRegister* Register = Stack->GetVirtualRegister(Operator.Variable.Name.c_str());

			if (Register->Type != Operator.Variable.Type)
			{
				Register->Type = Operator.Variable.Type;
				Register->Data.resize(GetSizeByType(Operator.Variable.Type, Stack->VM));
			}

			Ret = Register->Data.begin()._Unwrapped();
		}
		else if (IsClassMember(Operator.Scope) && Operator.AddressType == InstructionAddressType::Pointer_Offset)
		{
			memcpy(&TempAddress, &Stack->Stack_Main[Stack->EBP + Operator.Extra.Address.BaseAddress], sizeof(uint64));

			Ret = (char*)TempAddress + Operator.Extra.Address.Offset;

#if HAZE_VM_GET_ADDRESS_LOG
			HSS << Stack->EBP << " " << Stack->ESP << " " << Operator.Extra.Address.BaseAddress << " " << Address << " ";
#endif
		}
		else if (Operator.Scope == HazeDataDesc::ArrayElement)
		{
			Ret = &Stack->Stack_Main[Stack->EBP + Operator.Extra.Address.BaseAddress + Operator.Extra.Address.Offset];
		}
		else if (Operator.Scope == HazeDataDesc::FunctionAddress)
		{
			TempAddress = (uint64)((void*)&Stack->VM->GetFunctionByName(Operator.Variable.Name));
			Ret = &TempAddress;
		}
		else /*if (Operator.Scope == InstructionScopeType::Local)*/
		{
			Ret = &Stack->Stack_Main[Stack->EBP + Operator.Extra.Address.BaseAddress];
		}

#if HAZE_VM_GET_ADDRESS_LOG
		HSS << Ret << std::endl;
		HAZE_LOG_ERR(HSS.str().c_str());
#endif // HAZE_VM_GET_ADDRESS_LOG

		return Ret;
	}

	static void JmpToOperator(HazeStack* Stack, const InstructionData& Operator)
	{
		if (Operator.Variable.Name == HAZE_JMP_NULL)
		{
		}
		else
		{
			Stack->JmpTo(Operator);
		}
	}

	static void BinaryOperator(HazeStack* Stack)
	{
		const auto& Instruction = Stack->VM->Vector_Instruction[Stack->PC];
		const auto& Operator = Instruction.Operator;
		if (Operator.size() == 2)
		{
			if (IsNumberType(Operator[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(Operator[0].Variable.Type.PrimaryType, Instruction.InsCode, GetAddressByOperator(Stack, Operator[1]), GetAddressByOperator(Stack, Operator[0]));
			}
			else if (IsRegisterScope(Operator[0].Scope) && IsIntegerType(Operator[1].Variable.Type.PrimaryType))
			{
				if (Instruction.InsCode == InstructionOpCode::ADD || Instruction.InsCode == InstructionOpCode::SUB
					|| Instruction.InsCode == InstructionOpCode::ADD_ASSIGN || Instruction.InsCode == InstructionOpCode::SUB_ASSIGN)
				{
					auto Dst = GetAddressByOperator(Stack, Operator[0]);
					auto Src = GetAddressByOperator(Stack, Operator[1]);
					uint64 Address = 0;
					uint64 Size = GetSizeByType(Operator[1].Variable.Type, Stack->VM);
					uint64 Num = 0;
					memcpy(&Address, Dst, sizeof(Address));
					memcpy(&Num, Src, Size);

					char* NewAddress = (char*)Address + Size * Num *
						(Instruction.InsCode == InstructionOpCode::ADD || Instruction.InsCode == InstructionOpCode::ADD_ASSIGN ? 1 : -1);

					Address = (uint64)NewAddress;
					memcpy(Dst, &Address, sizeof(NewAddress));
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("Pointer binary operator error, %s %s operator %s do not support!\n"), Operator[0].Variable.Name.c_str(), Operator[1].Variable.Name.c_str(), GetInstructionString(Stack->VM->Vector_Instruction[Stack->PC].InsCode));
				}
			}
			else
			{
				HAZE_LOG_ERR(HAZE_TEXT("Binary operator error, %s %s operator %s!\n"), Operator[0].Variable.Name.c_str(), Operator[1].Variable.Name.c_str(), GetInstructionString(Stack->VM->Vector_Instruction[Stack->PC].InsCode));
			}
		}
	}
};

std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> HashMap_InstructionProcessor =
{
	{InstructionOpCode::MOV, &InstructionProcessor::Mov},
	{InstructionOpCode::MOVPV, &InstructionProcessor::MovPV},
	{InstructionOpCode::MOVTOPV, &InstructionProcessor::MovToPV},
	{InstructionOpCode::LEA, &InstructionProcessor::Lea},

	{InstructionOpCode::ADD, &InstructionProcessor::Add},
	{InstructionOpCode::SUB, &InstructionProcessor::Sub},
	{InstructionOpCode::MUL, &InstructionProcessor::Mul},
	{InstructionOpCode::DIV, &InstructionProcessor::Div},
	{InstructionOpCode::MOD, &InstructionProcessor::Mod},

	{InstructionOpCode::NEG, &InstructionProcessor::Neg},

	{InstructionOpCode::NOT, &InstructionProcessor::Not},
	
	{InstructionOpCode::INC, &InstructionProcessor::Inc},
	{InstructionOpCode::DEC, &InstructionProcessor::Dec},

	{InstructionOpCode::BIT_AND, &InstructionProcessor::Bit_And},
	{InstructionOpCode::BIT_OR, &InstructionProcessor::Bit_Or},
	{InstructionOpCode::BIT_NEG, &InstructionProcessor::Bit_Neg},
	{InstructionOpCode::BIT_XOR, &InstructionProcessor::Bit_Xor},

	{InstructionOpCode::ADD_ASSIGN, &InstructionProcessor::Add_Assign},
	{InstructionOpCode::SUB_ASSIGN, &InstructionProcessor::Sub_Assign},
	{InstructionOpCode::MUL_ASSIGN, &InstructionProcessor::Mul_Assign},
	{InstructionOpCode::DIV_ASSIGN, &InstructionProcessor::Div_Assign},
	{InstructionOpCode::MOD_ASSIGN, &InstructionProcessor::Mod_Assign},
	{InstructionOpCode::BIT_AND_ASSIGN, &InstructionProcessor::Bit_And_Assign},
	{InstructionOpCode::BIT_OR_ASSIGN, &InstructionProcessor::Bit_Or_Assign},
	{InstructionOpCode::BIT_XOR_ASSIGN, &InstructionProcessor::Bit_Xor_Assign},
	{InstructionOpCode::SHL_ASSIGN, &InstructionProcessor::Shl_Assign },
	{InstructionOpCode::SHR_ASSIGN, &InstructionProcessor::Shr_Assign},

	{InstructionOpCode::SHL, &InstructionProcessor::Shl},
	{InstructionOpCode::SHR, &InstructionProcessor::Shr},

	{InstructionOpCode::PUSH, &InstructionProcessor::Push},
	{InstructionOpCode::POP, &InstructionProcessor::Pop},

	{InstructionOpCode::CALL, &InstructionProcessor::Call},
	{InstructionOpCode::RET, &InstructionProcessor::Ret},
	{InstructionOpCode::NEW, &InstructionProcessor::New},

	{InstructionOpCode::CMP, &InstructionProcessor::Cmp},
	{InstructionOpCode::JMP, &InstructionProcessor::Jmp},
	{InstructionOpCode::JNE, &InstructionProcessor::Jne},
	{InstructionOpCode::JNG, &InstructionProcessor::Jng},
	{InstructionOpCode::JNL, &InstructionProcessor::Jnl},
	{InstructionOpCode::JE, &InstructionProcessor::Je},
	{InstructionOpCode::JG, &InstructionProcessor::Jg},
	{InstructionOpCode::JL, &InstructionProcessor::Jl},


	{InstructionOpCode::LINE, &InstructionProcessor::Line},
};