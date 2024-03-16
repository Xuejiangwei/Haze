#include "HazeHeader.h"
#include "HazeLog.h"
#include "HazeVM.h"
#include "HazeStack.h"

#include "HazeLibraryManager.h"

#include <Windows.h>

#define HAZE_CALL_LOG				0

extern std::unique_ptr<HazeLibraryManager> g_HazeLibManager;

static std::unordered_map<HAZE_STRING, InstructionOpCode> s_HashMap_String2Code =
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

bool IsRegisterDesc(HazeDataDesc desc)
{
	return HazeDataDesc::RegisterBegin < desc && desc < HazeDataDesc::RegisterEnd;
}

const HAZE_CHAR* GetInstructionString(InstructionOpCode code)
{
	static std::unordered_map<InstructionOpCode, const HAZE_CHAR*> s_HashMap_Code2String;

	if (s_HashMap_Code2String.size() <= 0)
	{
		for (auto& iter : s_HashMap_String2Code)
		{
			s_HashMap_Code2String[iter.second] = iter.first.c_str();
		}
	}

	auto iter = s_HashMap_Code2String.find(code);
	if (iter != s_HashMap_Code2String.end())
	{
		return iter->second;
	}

	return HAZE_TEXT("None");
}

InstructionOpCode GetInstructionByString(const HAZE_STRING& str)
{
	auto iter = s_HashMap_String2Code.find(str);
	if (iter != s_HashMap_String2Code.end())
	{
		return iter->second;
	}

	return InstructionOpCode::NONE;
}

bool IsJmpOpCode(InstructionOpCode code)
{
	return code >= InstructionOpCode::JMP && code <= InstructionOpCode::JL;
}

bool IsClassMember(HazeDataDesc desc)
{
	return desc >= HazeDataDesc::ClassMember_Local_Public && desc <= HazeDataDesc::ClassMember_Local_Protected;
}

void CallHazeFunction(HazeStack* stack, FunctionData* funcData, va_list& args);

class InstructionProcessor
{
	friend void CallHazeFunction(HazeStack* stack, FunctionData* funcData, va_list& args);
public:
	static void Mov(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			const void* src = GetOperatorAddress(stack, oper[1]);
			memcpy(dst, src, GetSizeByType(oper[0].Variable.Type, stack->m_VM));

			ClearRegisterType(stack, oper[1]);
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void MovPV(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			const void* src = GetOperatorAddress(stack, oper[1]);

			uint64 address = 0;
			memcpy(&address, src, sizeof(address));
			memcpy(dst, (void*)address, GetSizeByType(oper[0].Variable.Type, stack->m_VM));

			ClearRegisterType(stack, oper[1]);
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void MovToPV(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			const void* src = GetOperatorAddress(stack, oper[1]);

			uint64 address = 0;
			memcpy(&address, dst, sizeof(address));
			memcpy((void*)address, src, GetSizeByType(oper[1].Variable.Type, stack->m_VM));

			ClearRegisterType(stack, oper[1]);
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Lea(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			uint64 address = (uint64)GetOperatorAddress(stack, oper[1]);
			memcpy(dst, &address, GetSizeByType(oper[0].Variable.Type, stack->m_VM));
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Push(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			int size = GetSizeByType(oper[0].Variable.Type, stack->m_VM);

			if (oper[0].Desc == HazeDataDesc::Constant)
			{
				int number = StringToStandardType<int>(oper[0].Variable.Name);
				memcpy(&stack->m_StackMain[stack->m_ESP], &number, size);
			}
			else if (oper[0].Desc == HazeDataDesc::Address)
			{
				memcpy(&stack->m_StackMain[stack->m_ESP], &stack->m_PC, size);
			}
			else if (oper[0].Desc == HazeDataDesc::ClassThis)
			{
				uint64 address = (uint64)GetOperatorAddress(stack, oper[0]);
				memcpy(&stack->m_StackMain[stack->m_ESP], &address, sizeof(uint64));
			}
			else/* if (Operator[0].Scope == InstructionScopeType::Local || oper[0].Scope == InstructionScopeType::Global)*/
			{
				if (oper[0].Extra.Address.BaseAddress + (int)stack->m_EBP >= 0)
				{
					//Size = oper[0].Scope == HazeDataDesc::ConstantString ? Size = sizeof(Operator[0].Extra.Index) : Size;
					memcpy(&stack->m_StackMain[stack->m_ESP], GetOperatorAddress(stack, oper[0]), size);
				}
				else
				{
					memset(&stack->m_StackMain[stack->m_ESP], 0, size);
				}
			}

			stack->m_ESP += size;
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Pop(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			stack->m_ESP -= GetSizeByType(oper[0].Variable.Type, stack->m_VM);
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Add(HazeStack* stack)
	{
		BinaryOperator(stack);

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Sub(HazeStack* stack)
	{
		BinaryOperator(stack);

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Mul(HazeStack* stack)
	{
		BinaryOperator(stack);

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Div(HazeStack* stack)
	{
		BinaryOperator(stack);

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Mod(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::MOD, 
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Neg(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::NEG,
					GetOperatorAddress(stack, oper[0]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Not(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::NOT, GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Inc(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				OperatorValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::INC, GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Dec(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				OperatorValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::DEC,
					GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Bit_And(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_AND,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Bit_Or(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_OR,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Bit_Xor(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_XOR,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Add_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::ADD_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Sub_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SUB_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Mul_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::MUL_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Div_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::DIV_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Mod_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::MOD_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Bit_And_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_AND_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Bit_Or_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_OR_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Bit_Neg(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			if (IsIntegerType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_NEG,
					GetOperatorAddress(stack, oper[0]), GetOperatorAddress(stack, oper[0]));
			}
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("bir neg operator error!\n"));
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Bit_Xor_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_XOR_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Shl_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHL_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Shr_Assign(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHR_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Shl(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType) && IsIntegerType(oper[1].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHL,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Shr(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHR,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Call(HazeStack* stack)
	{
		//EBP = PC;
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() >= 1)
		{
#if HAZE_CALL_LOG
			HAZE_LOG_INFO(HAZE_TEXT("调用函数<%s>\n"), oper[0].Variable.Name.c_str());
#endif

#if HAZE_DEBUG_ENABLE

			stack->m_VM->InstructionExecPost();

#endif

			memcpy(&stack->m_StackMain[stack->m_ESP - HAZE_ADDRESS_SIZE], &stack->m_PC, HAZE_ADDRESS_SIZE);

			if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerFunction)
			{
				void* value = GetOperatorAddress(stack, oper[0]);
				uint64 functionAddress;
				memcpy(&functionAddress, value, sizeof(functionAddress));
				stack->OnCall((FunctionData*)functionAddress, oper[0].Extra.Call.ParamByteSize);
			}
			else
			{
				int functionIndex = stack->m_VM->GetFucntionIndexByName(oper[0].Variable.Name);
				auto& function = stack->m_VM->Vector_FunctionTable[functionIndex];

				if (function.FunctionDescData.Type == InstructionFunctionType::HazeFunction)
				{
					stack->OnCall(&function, oper[0].Extra.Call.ParamByteSize);
				}
				else
				{
					uint32 tempEBP = stack->m_EBP;
					stack->m_EBP = stack->m_ESP;

					if (function.FunctionDescData.Type == InstructionFunctionType::StdLibFunction)
					{
						function.FunctionDescData.StdLibFunction(stack, &function, oper[0].Extra.Call.ParamNum);
					}
					else if (function.FunctionDescData.Type == InstructionFunctionType::DLLLibFunction)
					{
						HazeRegister* retRegister = stack->GetVirtualRegister(RET_REGISTER);
						retRegister->Type.PrimaryType = function.Type;
						int size = GetSizeByType(retRegister->Type, stack->m_VM);
						retRegister->Data.resize(size);

						uint64 address = (uint64)(stack);
						memcpy(&stack->m_StackMain[stack->m_ESP], &address, sizeof(address));

						address = (uint64)(&ExeHazePointerFunction);
						memcpy(&stack->m_StackMain[stack->m_ESP + sizeof(address)], &address, sizeof(address));

						g_HazeLibManager->ExecuteDLLFunction(oper[1].Variable.Name, oper[0].Variable.Name,
							&stack->m_StackMain[stack->m_ESP - HAZE_ADDRESS_SIZE], retRegister->Data.begin()._Unwrapped(),
							stack, &ExeHazePointerFunction);
					}

					stack->m_ESP -= (oper[0].Extra.Call.ParamByteSize + HAZE_ADDRESS_SIZE);
					stack->m_EBP = tempEBP;
				}
			}
		}
	}

	static void Ret(HazeStack* stack)
	{
#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			HazeRegister* retRegister = stack->GetVirtualRegister(RET_REGISTER);
			retRegister->Type = oper[0].Variable.Type;

			int size = GetSizeByType(retRegister->Type, stack->m_VM);

			retRegister->Data.resize(size);
			memcpy(retRegister->Data.begin()._Unwrapped(), GetOperatorAddress(stack, oper[0]), size);
		}

		stack->OnRet();
	}

	static void New(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			HazeRegister* newRegister = stack->GetVirtualRegister(NEW_REGISTER);
			newRegister->Type = oper[0].Variable.Type;

			int size = GetSizeByType(newRegister->Type, stack->m_VM);

			uint64 address = (uint64)stack->Alloca(size);

			newRegister->Data.resize(size);
			memcpy(newRegister->Data.begin()._Unwrapped(), &address, size);
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Cmp(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);
			CompareValueByType(GetStrongerType(oper[0].Variable.Type.PrimaryType, oper[1].Variable.Type.PrimaryType),
				cmpRegister, GetOperatorAddress(stack, oper[0]), GetOperatorAddress(stack, oper[1]));
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Jmp(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			JmpToOperator(stack, oper[0]);
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Jne(HazeStack* stack)
	{
#define REGISTER_EQUAL(R) R->Data[0] == 1
#define REGISTER_GREATER(R) R->Data[1] == 1
#define REGISTER_LESS(R) R->Data[2] == 1

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);

			if (!REGISTER_EQUAL(cmpRegister))
			{
				JmpToOperator(stack, oper[0]);
			}
			else
			{
				JmpToOperator(stack, oper[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Jng(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);

			if (!REGISTER_GREATER(cmpRegister))
			{
				JmpToOperator(stack, oper[0]);
			}
			else
			{
				JmpToOperator(stack, oper[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Jnl(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);

			if (!REGISTER_LESS(cmpRegister))
			{
				JmpToOperator(stack, oper[0]);
			}
			else
			{
				JmpToOperator(stack, oper[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Je(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);

			if (REGISTER_EQUAL(cmpRegister))
			{
				JmpToOperator(stack, oper[0]);
			}
			else
			{
				JmpToOperator(stack, oper[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Jg(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);

			if (REGISTER_GREATER(cmpRegister))
			{
				JmpToOperator(stack, oper[0]);
			}
			else
			{
				JmpToOperator(stack, oper[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Jl(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);

			if (REGISTER_LESS(cmpRegister))
			{
				JmpToOperator(stack, oper[0]);
			}
			else
			{
				JmpToOperator(stack, oper[1]);
			}
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

	static void Line(HazeStack* stack)
	{
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			stack->m_VM->OnExecLine(oper[0].Extra.Line);
		}

#if HAZE_DEBUG_ENABLE

		stack->m_VM->InstructionExecPost();

#endif
	}

private:
	//	static void* const GetAddressByOperator(HazeStack* stack, const InstructionData& oper)
	//	{
	//#define HAZE_VM_GET_ADDRESS_LOG 0
	//
	//#if HAZE_VM_GET_ADDRESS_LOG
	//		HAZE_STRING_STREAM HSS;
	//		HSS << "Address " << oper.Variable.Name << " ";
	//#endif
	//
	//		void* Ret = nullptr;
	//		thread_local static HazeVariable ConstantValue;
	//		thread_local static uint64 TempAddress;
	//
	//		if (Operator.Desc == HazeDataDesc::Constant || oper.Desc == HazeDataDesc::NullPtr)
	//		{
	//			auto& Type = const_cast<HazeDefineType&>(ConstantValue.GetType());
	//			auto& Value = const_cast<HazeValue&>(ConstantValue.GetValue());
	//
	//			Type.PrimaryType = oper.Variable.Type.PrimaryType;
	//			StringToHazeValueNumber(Operator.Variable.Name, Type.PrimaryType, Value);
	//			Ret = GetBinaryPointer(Type.PrimaryType, Value);
	//		}
	//		else if (Operator.Scope == HazeVariableScope::Global)
	//		{
	//			Ret = stack->VM->GetGlobalValue(Operator.Variable.Name);
	//		}
	//		else if (Operator.Desc == HazeDataDesc::ConstantString)
	//		{
	//			TempAddress = (uint64)&stack->GetVM()->GetHazeStringByIndex(Operator.Extra.Index);
	//			Ret = &TempAddress;
	//		}
	//		else if (IsRegisterScope(Operator.Desc))
	//		{
	//			HazeRegister* Register = stack->GetVirtualRegister(Operator.Variable.Name.c_str());
	//
	//			if (Register->Type != oper.Variable.Type)
	//			{
	//				Register->Type = oper.Variable.Type;
	//				Register->Data.resize(GetSizeByType(Operator.Variable.Type, stack->VM));
	//			}
	//
	//			Ret = Register->Data.begin()._Unwrapped();
	//		}
	//		else if (IsClassMember(Operator.Desc) && oper.AddressType == InstructionAddressType::Pointer_Offset)
	//		{
	//			memcpy(&TempAddress, &stack->Stack_Main[stack->EBP + oper.Extra.Address.BaseAddress], sizeof(uint64));
	//
	//			Ret = (char*)TempAddress + oper.Extra.Address.Offset;
	//
	//#if HAZE_VM_GET_ADDRESS_LOG
	//			HSS << stack->EBP << " " << stack->ESP << " " << oper.Extra.Address.BaseAddress << " " << Address << " ";
	//#endif
	//		}
	//		else if (Operator.Desc == HazeDataDesc::ArrayElement)
	//		{
	//			Ret = &stack->Stack_Main[stack->EBP + oper.Extra.Address.BaseAddress + oper.Extra.Address.Offset];
	//		}
	//		else if (Operator.Desc == HazeDataDesc::FunctionAddress)
	//		{
	//			TempAddress = (uint64)((void*)&stack->VM->GetFunctionByName(Operator.Variable.Name));
	//			Ret = &TempAddress;
	//		}
	//		else /*if (Operator.Scope == InstructionScopeType::Local)*/
	//		{
	//			Ret = &stack->Stack_Main[stack->EBP + oper.Extra.Address.BaseAddress];
	//		}
	//
	//#if HAZE_VM_GET_ADDRESS_LOG
	//		HSS << Ret << std::endl;
	//		HAZE_LOG_ERR(HSS.str().c_str());
	//#endif // HAZE_VM_GET_ADDRESS_LOG
	//
	//		return Ret;
	//	}

	static void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData)
	{
		thread_local static HazeVariable constantValue;
		thread_local static uint64 tempAddress;

		void* ret = nullptr;

		switch (insData.AddressType)
		{
		case InstructionAddressType::Global:
		{
			return stack->m_VM->GetGlobalValueByIndex(insData.Extra.Index);
		}
		case InstructionAddressType::Global_Base_Offset:
		{
			ret = stack->m_VM->GetGlobalValueByIndex(insData.Extra.Index);
			return (char*)ret + insData.Extra.Address.Offset;
		}
		case InstructionAddressType::Global_BasePointer_Offset:
		{
			ret = stack->m_VM->GetGlobalValueByIndex(insData.Extra.Index);
			memcpy(&tempAddress, ret, sizeof(tempAddress));
			return (char*)tempAddress + insData.Extra.Address.Offset;
		}
		case InstructionAddressType::Local:
		{
			return &stack->m_StackMain[stack->m_EBP + insData.Extra.Address.BaseAddress];
		}
		case InstructionAddressType::Local_Base_Offset:
		{
			return &stack->m_StackMain[stack->m_EBP + insData.Extra.Address.BaseAddress + insData.Extra.Address.Offset];
		}
		case InstructionAddressType::Local_BasePointer_Offset:
		{
			ret = &stack->m_StackMain[stack->m_EBP + insData.Extra.Address.BaseAddress];
			memcpy(&tempAddress, ret, sizeof(tempAddress));
			return (char*)tempAddress + insData.Extra.Address.Offset;
		}
		case InstructionAddressType::FunctionAddress:
		{
			tempAddress = (uint64)((void*)&stack->m_VM->GetFunctionByName(insData.Variable.Name));
			ret = &tempAddress;
			return ret;
		}
		case InstructionAddressType::Constant:
		case InstructionAddressType::NullPtr:
		{
			auto& type = const_cast<HazeDefineType&>(constantValue.GetType());
			auto& value = const_cast<HazeValue&>(constantValue.GetValue());

			type.PrimaryType = insData.Variable.Type.PrimaryType;
			StringToHazeValueNumber(insData.Variable.Name, type.PrimaryType, value);
			ret = GetBinaryPointer(type.PrimaryType, value);
			return ret;
		}
		case InstructionAddressType::ConstantString:
		{
			tempAddress = (uint64)&stack->m_VM->GetHazeStringByIndex(insData.Extra.Index);
			return &tempAddress;
		}
		case InstructionAddressType::Register:
		{
			HazeRegister* hazeRegister = stack->GetVirtualRegister(insData.Variable.Name.c_str());

			if (hazeRegister->Type != insData.Variable.Type)
			{
				hazeRegister->Type = insData.Variable.Type;
				hazeRegister->Data.resize(GetSizeByType(insData.Variable.Type, stack->m_VM));
			}

			return hazeRegister->Data.begin()._Unwrapped();
		}
		default:
			return nullptr;
			break;
		}
	}

	static void JmpToOperator(HazeStack* stack, const InstructionData& insData)
	{
		if (insData.Variable.Name == HAZE_JMP_NULL)
		{
		}
		else
		{
			stack->JmpTo(insData);
		}
	}

	static void BinaryOperator(HazeStack* stack)
	{
		const auto& instruction = stack->m_VM->Instructions[stack->m_PC];
		const auto& oper = instruction.Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, instruction.InsCode, 
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
			else if (IsRegisterDesc(oper[0].Desc) && IsIntegerType(oper[1].Variable.Type.PrimaryType))
			{
				if (instruction.InsCode == InstructionOpCode::ADD || instruction.InsCode == InstructionOpCode::SUB
					|| instruction.InsCode == InstructionOpCode::ADD_ASSIGN || instruction.InsCode == InstructionOpCode::SUB_ASSIGN)
				{
					auto dst = GetOperatorAddress(stack, oper[0]);
					auto src = GetOperatorAddress(stack, oper[1]);
					uint64 address = 0;
					uint64 size = GetSizeByType(oper[1].Variable.Type, stack->m_VM);
					uint64 num = 0;
					memcpy(&address, dst, sizeof(address));
					memcpy(&num, src, size);

					char* newAddress = (char*)address + size * num *
						(instruction.InsCode == InstructionOpCode::ADD || instruction.InsCode == InstructionOpCode::ADD_ASSIGN ? 1 : -1);

					address = (uint64)newAddress;
					memcpy(dst, &address, sizeof(newAddress));
				}
				else
				{
					HAZE_LOG_ERR(HAZE_TEXT("Pointer binary operator error, %s %s operator %s do not support!\n"),
						oper[0].Variable.Name.c_str(), oper[1].Variable.Name.c_str(), 
						GetInstructionString(stack->m_VM->Instructions[stack->m_PC].InsCode));
				}
			}
			else
			{
				HAZE_LOG_ERR(HAZE_TEXT("Binary operator error, %s %s operator %s!\n"), oper[0].Variable.Name.c_str(),
					oper[1].Variable.Name.c_str(), GetInstructionString(stack->m_VM->Instructions[stack->m_PC].InsCode));
			}
		}
	}

	static void ExeHazePointerFunction(void* stackPointer, void* value, int paramNum, ...)
	{
		va_list args;
		va_start(args, paramNum);
		CallHazeFunction((HazeStack*)stackPointer, (FunctionData*)value, paramNum, args);
		va_end(args);
	}

	static void CallHazeFunction(HazeStack* stack, FunctionData* funcData, int paramNum, va_list& args)
	{
		int size = 0;
		for (size_t i = 0; i < funcData->Params.size(); i++)
		{
			size += GetSizeByType(funcData->Params[i].Type, stack->m_VM);
		}
		stack->m_ESP += size;

		for (size_t i = 0; i < funcData->Params.size(); i++)
		{
			PushParam(stack, args, funcData->Params[i].Type);
		}
		stack->m_ESP += size;

		memcpy(&stack->m_StackMain[stack->m_ESP], &stack->m_PC, HAZE_ADDRESS_SIZE);
		stack->m_ESP += HAZE_ADDRESS_SIZE;

		stack->OnCall(funcData, size);
		stack->m_PC++;
		stack->ResetCallHaze();
	}

	static int PushParam(HazeStack* stack, va_list& Args, const HazeDefineType& m_Type)
	{
		int size = GetSizeByType(m_Type, stack->m_VM);
		if (IsHazeDefaultType(m_Type.PrimaryType))
		{
			stack->m_ESP -= size;
			switch (m_Type.PrimaryType)
			{
			case HazeValueType::Bool:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, bool), size);
				break;
			case HazeValueType::Byte:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, hbyte), size);
				break;
			case HazeValueType::UnsignedByte:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, uhbyte), size);
				break;
			case HazeValueType::Char:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, hchar), size);
				break;
			case HazeValueType::Short:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, short), size);
				break;
			case HazeValueType::UnsignedShort:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, ushort), size);
				break;
			case HazeValueType::Int:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, int), size);
				break;
			case HazeValueType::Long:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, int64), size);
				break;
			case HazeValueType::Double:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, double), size);
				break;
			case HazeValueType::UnsignedInt:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, uint32), size);
				break;
			case HazeValueType::UnsignedLong:
				memcpy(&stack->m_StackMain[stack->m_ESP], &va_arg(Args, uint64), size);
				break;
			default:
				HAZE_LOG_ERR_W("三方库调用Haze函数Push参数<%s>类型错误", GetHazeValueTypeString(m_Type.PrimaryType));
				break;
			}
		}
		else
		{
			HAZE_LOG_ERR_W("三方库调用Haze函数暂时只支持默认类型!\n");
		}

		return size;
	}

	static void ClearRegisterType(HazeStack* stack, const InstructionData& oper)
	{
		//New和Ret寄存器的type清空，防止没有垃圾回收掉
		if (oper.AddressType == InstructionAddressType::Register)
		{
			auto Register = stack->GetVirtualRegister(oper.Variable.Name.c_str());
			if (Register == stack->GetVirtualRegister(NEW_REGISTER) || Register == stack->GetVirtualRegister(RET_REGISTER))
			{
				Register->Type.Reset();
			}
		}
	}
};

void CallHazeFunction(HazeStack* stack, FunctionData* funcData, va_list& args)
{
	InstructionProcessor::CallHazeFunction(stack, funcData, (int)funcData->Params.size(), args);
}

//可以考虑将HashMap改为使用数组
std::unordered_map<InstructionOpCode, void(*)(HazeStack* stack)> g_InstructionProcessor =
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