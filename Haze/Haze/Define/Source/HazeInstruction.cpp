#include "HazePch.h"
#include "HazeHeader.h"
#include "HazeDebugDefine.h"
#include "HazeLogDefine.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "ObjectArray.h"
#include "HazeLibraryManager.h"

#include <Windows.h>

#define HAZE_CALL_LOG				0

#define POINTER_ADD_SUB(T, S, STACK, OPER, INS) T v; memcpy(&v, S, sizeof(T)); \
				auto type = OPER[0].Variable.Type.SecondaryType; \
				auto size = GetSizeByHazeType(type); \
				uint64 address; auto operAddress = GetOperatorAddress(STACK, OPER[0]); memcpy(&address, operAddress, sizeof(operAddress)); \
				if (INS == InstructionOpCode::SUB) address -= size * v; else address += size * v; \
				memcpy(operAddress, &address, sizeof(operAddress));

extern Unique<HazeLibraryManager> g_HazeLibManager;

static HashMap<HString, InstructionOpCode> s_HashMap_String2Code =
{
	{H_TEXT("MOV"), InstructionOpCode::MOV },
	{H_TEXT("MOVPV"), InstructionOpCode::MOVPV },
	{H_TEXT("MOVTOPV"), InstructionOpCode::MOVTOPV },
	{H_TEXT("LEA"), InstructionOpCode::LEA },
	{H_TEXT("ADD"), InstructionOpCode::ADD },
	{H_TEXT("SUB"), InstructionOpCode::SUB },
	{H_TEXT("MUL"), InstructionOpCode::MUL },
	{H_TEXT("DIV"), InstructionOpCode::DIV },
	{H_TEXT("MOD"), InstructionOpCode::MOD },

	{H_TEXT("NEG"), InstructionOpCode::NEG },
	{H_TEXT("NOT"), InstructionOpCode::NOT },
	{H_TEXT("INC"), InstructionOpCode::INC },
	{H_TEXT("DEC"), InstructionOpCode::DEC},

	{H_TEXT("BIT_AND"), InstructionOpCode::BIT_AND },
	{H_TEXT("BIT_OR"), InstructionOpCode::BIT_OR },
	{H_TEXT("BIT_NEG"), InstructionOpCode::BIT_NEG },
	{H_TEXT("BIT_XOR"), InstructionOpCode::BIT_XOR },
	{H_TEXT("SHL"), InstructionOpCode::SHL },
	{H_TEXT("SHR"), InstructionOpCode::SHR },

	{H_TEXT("ADD_ASSIGN"), InstructionOpCode::ADD_ASSIGN },
	{H_TEXT("SUB_ASSIGN"), InstructionOpCode::SUB_ASSIGN },
	{H_TEXT("MUL_ASSIGN"), InstructionOpCode::MUL_ASSIGN },
	{H_TEXT("DIV_ASSIGN"), InstructionOpCode::DIV_ASSIGN },
	{H_TEXT("MOD_ASSIGN"), InstructionOpCode::MOD_ASSIGN },
	{H_TEXT("BIT_AND_ASSIGN"), InstructionOpCode::BIT_AND_ASSIGN },
	{H_TEXT("BIT_OR_ASSIGN"), InstructionOpCode::BIT_OR_ASSIGN },
	{H_TEXT("BIT_XOR_ASSIGN"), InstructionOpCode::BIT_XOR_ASSIGN },
	{H_TEXT("SHL_ASSIGN"), InstructionOpCode::SHL_ASSIGN },
	{H_TEXT("SHR_ASSIGN"), InstructionOpCode::SHR_ASSIGN },

	{H_TEXT("PUSH"), InstructionOpCode::PUSH },
	{H_TEXT("POP"), InstructionOpCode::POP },

	{H_TEXT("CALL"), InstructionOpCode::CALL },
	{H_TEXT("RET"), InstructionOpCode::RET },

	{H_TEXT("NEW"), InstructionOpCode::NEW },

	{H_TEXT("CMP"), InstructionOpCode::CMP },
	{H_TEXT("JMP"), InstructionOpCode::JMP },
	{H_TEXT("JNE"), InstructionOpCode::JNE },
	{H_TEXT("JNG"), InstructionOpCode::JNG },
	{H_TEXT("JNL"), InstructionOpCode::JNL },
	{H_TEXT("JE"), InstructionOpCode::JE },
	{H_TEXT("JG"), InstructionOpCode::JG },
	{H_TEXT("JL"), InstructionOpCode::JL },

	{H_TEXT("CVT"), InstructionOpCode::CVT },

	{H_TEXT("LINE"), InstructionOpCode::LINE },

	{H_TEXT("SIGN"), InstructionOpCode::SIGN },
};

bool IsRegisterDesc(HazeDataDesc desc)
{
	return HazeDataDesc::RegisterBegin < desc && desc < HazeDataDesc::RegisterEnd;
}

const HChar* GetInstructionString(InstructionOpCode code)
{
	static HashMap<InstructionOpCode, const HChar*> s_HashMap_Code2String;

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

	HAZE_LOG_ERR_W("未能找到字节码操作符名称<%d>!\n", (int)code);
	return H_TEXT("None");
}

InstructionOpCode GetInstructionByString(const HString& str)
{
	auto iter = s_HashMap_String2Code.find(str);
	if (iter != s_HashMap_String2Code.end())
	{
		return iter->second;
	}

	HAZE_LOG_ERR_W("未能找到名称对应字节码操作符<%s>!\n", str.c_str());
	return InstructionOpCode::NONE;
}

bool IsJmpOpCode(InstructionOpCode code)
{
	return code >= InstructionOpCode::JMP && code <= InstructionOpCode::JL;
}

bool IsClassMember(HazeDataDesc desc)
{
	return desc >= HazeDataDesc::ClassMember_Local_Public && desc <= HazeDataDesc::ClassMember_Local_Private;
}

void CallHazeFunction(HazeStack* stack, FunctionData* funcData, va_list& args);

class InstructionProcessor
{
	friend void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args);
	friend void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData);
#define HAZE_DEBUG_ENABLE 1
#if HAZE_DEBUG_ENABLE
	struct DataDebugScope
	{
		DataDebugScope(HazeStack* stack, const V_Array<InstructionData>& data, InstructionOpCode opCode)
			: Stack(stack), Data(data)
		{
			auto address = GetOperatorAddress(Stack, Data[0]);
			if (address)
			{
				memcpy(&Address, address, 8);
			}
			else
			{
				Address = 0;
			}

			if (Data.size() == 2)
			{
				HAZE_LOG_ERR_W("开始 操作数一存储地址<%p> 操作数二地址<%p> EBP<%d> ESP<%d>", (char*)Address, GetOperatorAddress(stack, Data[1]),
					Stack->m_EBP, Stack->m_ESP);
				ShowData2();
				HAZE_LOG_ERR_W("\n");

				HAZE_LOG_INFO(H_TEXT("执行指令<%s> <%s> <%s>\n"), GetInstructionString(opCode),
					Data[0].Variable.Name.c_str(), Data[1].Variable.Name.c_str());
			}
			else
			{
				HAZE_LOG_ERR_W("开始 操作数一存储地址<%p> EBP<%d> ESP<%d>\n", (char*)Address, Stack->m_EBP, Stack->m_ESP);

				HAZE_LOG_INFO(H_TEXT("执行指令<%s> <%s> \n"), GetInstructionString(opCode),
					Data[0].Variable.Name.c_str());
			}
		}

		~DataDebugScope()
		{
			memcpy(&Address, GetOperatorAddress(Stack, Data[0]), 8);
			if (Data.size() == 2)
			{
				HAZE_LOG_ERR_W("结束 操作数一存储地址<%p> 操作数二地址<%p> EBP<%d> ESP<%d>", (char*)Address, 
					GetOperatorAddress(Stack, Data[1]), Stack->m_EBP, Stack->m_ESP);
				ShowData2();
				HAZE_LOG_ERR_W("\n\n");
			}
			else
			{
				HAZE_LOG_ERR_W("结束 操作数一存储地址<%p> EBP<%d> ESP<%d>\n\n", (char*)Address, Stack->m_EBP, Stack->m_ESP);
			}
		}

		void ShowData2()
		{
			switch (Data[1].Variable.Type.PrimaryType)
			{
			case HazeValueType::Int32:
			{
				int v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(int));
				HAZE_LOG_ERR_W(" 值<%d>", v);
			}
			break;
			case HazeValueType::Int64:
			{
				int64 v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(int64));
				HAZE_LOG_ERR_W(" 值<%d>", v);
			}
			break;
			case HazeValueType::UInt32:
			{
				uint32 v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(uint32));
				HAZE_LOG_ERR_W(" 值<%d>", v);
			}
			break;
			case HazeValueType::UInt64:
			{
				uint64 v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(uint64));
				HAZE_LOG_ERR_W(" 值<%d>", v);
			}
			break;
			default:
				break;
			}
		}

	private:
		const V_Array<InstructionData>& Data;
		uint64 Address;
		HazeStack* Stack;
	};
	#define INSTRUCTION_DATA_DEBUG DataDebugScope debugScope(stack, stack->m_VM->Instructions[stack->m_PC].Operator, stack->m_VM->Instructions[stack->m_PC].InsCode)
#else
	#define INSTRUCTION_DATA_DEBUG
#endif

public:
	static void Mov(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			const void* src = GetOperatorAddress(stack, oper[1]);
			memcpy(dst, src, GetSizeByType(oper[0].Variable.Type, stack->m_VM));

			ClearRegisterType(stack, oper[1]);
		}

		stack->m_VM->InstructionExecPost();
	}

	static void MovPV(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

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

		stack->m_VM->InstructionExecPost();
	}

	static void MovToPV(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

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

		stack->m_VM->InstructionExecPost();
	}

	static void Lea(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			uint64 address = (uint64)GetOperatorAddress(stack, oper[1]);
			memcpy(dst, &address, GetSizeByType(oper[0].Variable.Type, stack->m_VM));
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Push(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			int size = GetSizeByType(oper[0].Variable.Type, stack->m_VM);

			if (oper[0].Desc == HazeDataDesc::Address)
			{
				memcpy(&stack->m_StackMain[stack->m_ESP], &stack->m_PC, size);
			}
			else if (oper[0].Desc == HazeDataDesc::ClassThis)
			{
				uint64 address = (uint64)GetOperatorAddress(stack, oper[0]);
				memcpy(&stack->m_StackMain[stack->m_ESP], GetOperatorAddress(stack, oper[0]), sizeof(uint64));
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

		stack->m_VM->InstructionExecPost();
	}

	static void Pop(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			auto size = GetSizeByType(oper[0].Variable.Type, stack->m_VM);
			auto address = GetOperatorAddress(stack, oper[0]);
			memcpy(address, &stack->m_StackMain[stack->m_ESP - size], size);
			
			stack->m_ESP -= size;
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Add(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void Sub(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
	
		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void Mul(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void Div(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void Mod(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::MOD, 
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Neg(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::NEG,
					GetOperatorAddress(stack, oper[0]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Not(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::NOT, GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Inc(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				OperatorValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::INC, GetOperatorAddress(stack, oper[0]));
			}
			/*else if (IsPointerType(oper[0].Variable.Type.PrimaryType) && !IsPointerFunction(oper[0].Variable.Type.PrimaryType))
			{
				uint32 size = 0;
				if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerBase)
				{
					size = GetSizeByHazeType(oper[0].Variable.Type.SecondaryType);
				}
				else if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerClass)
				{
					size =	stack->m_VM->GetClassSize(oper[0].Variable.Type.CustomName);
				}
				else
				{
					size = 0;
					INS_ERR_W("错误的指针类型");
				}

				auto address = (char*)GetOperatorAddress(stack, oper[0]);
				uint64 pointerAddressValue;
				memcpy(&pointerAddressValue, address, sizeof(address));
				pointerAddressValue += size;
				memcpy(address, &pointerAddressValue, sizeof(address));
			}*/
			else
			{
				INS_ERR_W("操作错误");
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Dec(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				OperatorValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::DEC,
					GetOperatorAddress(stack, oper[0]));
			}
			else
			{
				INS_ERR_W("操作错误");
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Bit_And(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_AND,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Bit_Or(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_OR,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Bit_Xor(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
	
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_XOR,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Add_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::ADD_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Sub_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SUB_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Mul_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::MUL_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Div_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::DIV_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}



		stack->m_VM->InstructionExecPost();
	}

	static void Mod_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::MOD_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Bit_And_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_AND_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Bit_Or_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_OR_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Bit_Neg(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG; 
		
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
			HAZE_LOG_ERR_W("字节取反操作错误!\n");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Bit_Xor_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG; 
		
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::BIT_XOR_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Shl_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHL_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Shr_Assign(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHR_ASSIGN,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Shl(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType) && IsIntegerType(oper[1].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHL,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Shr(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType))
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, InstructionOpCode::SHR,
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Call(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() >= 1)
		{
#if HAZE_CALL_LOG
			HAZE_LOG_INFO(H_TEXT("调用函数<%s>\n"), oper[0].Variable.Name.c_str());
#endif

			memcpy(&stack->m_StackMain[stack->m_ESP - HAZE_ADDRESS_SIZE], &stack->m_PC, HAZE_ADDRESS_SIZE);

			if (oper[1].Desc == HazeDataDesc::CallFunctionPointer)
			{
				((void(*)(HazeStack*))(oper[1].Extra.Pointer))(stack);
			}
			else if (oper[0].Variable.Type.PrimaryType == HazeValueType::Function)
			{
				void* value = GetOperatorAddress(stack, oper[0]);
				uint64 functionAddress;
				memcpy(&functionAddress, (char*)value, sizeof(functionAddress));
				stack->OnCall((FunctionData*)functionAddress, oper[0].Extra.Call.ParamByteSize);
			}
			else
			{
				int functionIndex = stack->m_VM->GetFucntionIndexByName(oper[0].Variable.Name);
				if (functionIndex >= 0)
				{
					auto& function = stack->m_VM->Vector_FunctionTable[functionIndex];
					if (function.FunctionDescData.Type == InstructionFunctionType::HazeFunction)
					{
						stack->OnCall(&function, oper[0].Extra.Call.ParamByteSize);
					}
					else
					{
						uint32 tempEBP = stack->m_EBP;
						stack->m_EBP = stack->m_ESP;

						if (function.FunctionDescData.Type == InstructionFunctionType::StaticLibFunction)
						{
							function.FunctionDescData.StdLibFunction(stack, &function, oper[0].Extra.Call.ParamNum);
						}
						else if (function.FunctionDescData.Type == InstructionFunctionType::DLLLibFunction)
						{
							HazeRegister* retRegister = stack->GetVirtualRegister(RET_REGISTER);
							retRegister->Type = function.Type;
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
				else
				{
					HAZE_LOG_ERR_W("调用函数<%s>错误，未能找到!\n", oper[0].Variable.Name.c_str());
				}
				
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Ret(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

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

		stack->m_VM->InstructionExecPost();
	}

	static void New(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		HazeRegister* newRegister = stack->GetVirtualRegister(oper[0].Variable.Name.c_str());
		if (oper.size() == 2)
		{
			newRegister->Type = oper[0].Variable.Type;

			bool isArray = IsArrayType(newRegister->Type.PrimaryType);
		
			uint64 size = 0;
			if (isArray)
			{
				if (IsClassType(newRegister->Type.SecondaryType))
				{
					size = stack->m_VM->GetClassSize(*newRegister->Type.CustomName);
				}
				else
				{
					size = GetSizeByHazeType(newRegister->Type.SecondaryType);
				}
			}
			else
			{
				size = GetSizeByType(newRegister->Type, stack->m_VM);
			}
			uint64 newSize = size;
			auto countAddress = GetOperatorAddress(stack, oper[1]);
			
			auto count = *((uint64*)countAddress);
			void* address = nullptr;
			if (count > 0)
			{
				isArray = true;
				for (uint64 i = 0; i < count; i++)
				{
					newSize *= stack->m_VM->Instructions[stack->m_PC + i + 1].Operator[0].Extra.SignData;
				}

				address = stack->Alloca(newSize + sizeof(ObjectArray));
				new((char*)address + newSize) ObjectArray(count, address, newSize / size, stack->m_PC);
				address = (char*)address + newSize;
			}
			else
			{
				address = stack->Alloca(newSize);
			}
			if (isArray)
			{
				stack->RegisterArray(address, newSize / size);
			}

			newRegister->Data.resize(sizeof(address));
			memcpy(newRegister->Data.begin()._Unwrapped(), &address, sizeof(address));

			stack->m_PC += count;
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Cmp(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(CMP_REGISTER);
			CompareValueByType(GetStrongerType(oper[0].Variable.Type.PrimaryType, oper[1].Variable.Type.PrimaryType),
				cmpRegister, GetOperatorAddress(stack, oper[0]), GetOperatorAddress(stack, oper[1]));
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Jmp(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			JmpToOperator(stack, oper[0]);
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Jne(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

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

		stack->m_VM->InstructionExecPost();
	}

	static void Jng(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
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

		stack->m_VM->InstructionExecPost();
	}

	static void Jnl(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

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

		stack->m_VM->InstructionExecPost();
	}

	static void Je(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

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

		stack->m_VM->InstructionExecPost();
	}

	static void Jg(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
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

		stack->m_VM->InstructionExecPost();
	}

	static void Jl(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
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

		stack->m_VM->InstructionExecPost();
	}

	static void CVT(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Variable.Type.PrimaryType) && IsNumberType(oper[1].Variable.Type.PrimaryType))
			{
				HazeValue v1, v2;
				auto address = GetOperatorAddress(stack, oper[0]);

				memcpy(&v1, address, GetSizeByHazeType(oper[0].Variable.Type.PrimaryType));
				memcpy(&v2, GetOperatorAddress(stack, oper[1]), GetSizeByHazeType(oper[1].Variable.Type.PrimaryType));
				ConvertBaseTypeValue(oper[0].Variable.Type.PrimaryType, v1, oper[1].Variable.Type.PrimaryType, v2);
				memcpy(address, &v1, GetSizeByHazeType(oper[0].Variable.Type.PrimaryType));
			}
			else
			{
				INS_ERR_W("<%s>转换为<%s>的类型时错误!", oper[1].Variable.Name.c_str(), oper[0].Variable.Name.c_str());
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void Line(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			stack->m_VM->OnExecLine(oper[0].Extra.Line);
		}

		stack->m_VM->InstructionExecPost();
	}

private:
	static void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData)
	{
		thread_local static HazeVariable constantValue;
		thread_local static uint64 tempAddress;

		void* ret = nullptr;

		switch (insData.AddressType)
		{
		case InstructionAddressType::Global:
		{
			ret = stack->m_VM->GetGlobalValueByIndex(insData.Extra.Index);
		}
			break;
		case InstructionAddressType::Global_Base_Offset:
		{
			ret = stack->m_VM->GetGlobalValueByIndex(insData.Extra.Index);
			ret = (char*)ret + insData.Extra.Address.Offset;
		}
			break;
		case InstructionAddressType::Global_BasePointer_Offset:
		{
			ret = stack->m_VM->GetGlobalValueByIndex(insData.Extra.Index);
			memcpy(&tempAddress, ret, sizeof(tempAddress));
			ret = (char*)tempAddress + insData.Extra.Address.Offset;
		}
			break;
		case InstructionAddressType::Local:
		{
			ret = &stack->m_StackMain[stack->m_EBP + insData.Extra.Address.BaseAddress];
		}
			break;
		case InstructionAddressType::Local_Base_Offset:
		{
			ret = &stack->m_StackMain[stack->m_EBP + insData.Extra.Address.BaseAddress + insData.Extra.Address.Offset];
		}
			break;
		case InstructionAddressType::Local_BasePointer_Offset:
		{
			ret = &stack->m_StackMain[stack->m_EBP + insData.Extra.Address.BaseAddress];
			memcpy(&tempAddress, ret, sizeof(tempAddress));
			ret = (char*)tempAddress + insData.Extra.Address.Offset;
		}
			break;
		case InstructionAddressType::FunctionAddress:
		{
			/*tempAddress = (uint64)((void*)&stack->m_VM->GetFunctionByName(insData.Variable.Name));
			ret = &tempAddress;*/
			ret = (void*)&stack->m_VM->GetFunctionByName(insData.Variable.Name);
		}
			break;
		case InstructionAddressType::Constant:
		{
			auto& type = const_cast<HazeDefineType&>(constantValue.GetType());
			auto& value = const_cast<HazeValue&>(constantValue.GetValue());

			type.PrimaryType = insData.Variable.Type.PrimaryType;
			StringToHazeValueNumber(insData.Variable.Name, type.PrimaryType, value);
			ret = GetBinaryPointer(type.PrimaryType, value);
		}
			break;
		case InstructionAddressType::NullPtr:
		{
			auto& type = const_cast<HazeDefineType&>(constantValue.GetType());
			auto& value = const_cast<HazeValue&>(constantValue.GetValue());

			type.PrimaryType = insData.Variable.Type.PrimaryType;
			StringToHazeValueNumber(H_TEXT("0"), type.PrimaryType, value);
			ret = GetBinaryPointer(type.PrimaryType, value);
		}
			break;
		case InstructionAddressType::ConstantString:
		{
			tempAddress = (uint64)stack->m_VM->GetConstantStringByIndex(insData.Extra.Index);
			ret = &tempAddress;
		}
			break;
		case InstructionAddressType::Register:
		{
			HazeRegister* hazeRegister = stack->GetVirtualRegister(insData.Variable.Name.c_str());

			if (hazeRegister->Type != insData.Variable.Type)
			{
				hazeRegister->Type = insData.Variable.Type;
				hazeRegister->Data.resize(GetSizeByType(insData.Variable.Type, stack->m_VM));
			}

			ret = hazeRegister->Data.begin()._Unwrapped();
		}
			break;
		case InstructionAddressType::PointerAddress:
			ret = insData.Extra.Pointer;
		default:
			break;
		}

		if (IsRefrenceType(insData.Variable.Type.PrimaryType) && ret)
		{
			memcpy(&tempAddress, ret, sizeof(ret));
			ret = (void*)tempAddress;
		}

		return ret;
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
			if (IsNumberType(oper[0].Variable.Type.PrimaryType) && oper[0].Variable.Type == oper[1].Variable.Type)
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, instruction.InsCode, 
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
			/*else if (IsPointerType(oper[0].Variable.Type.PrimaryType) && (IsIntegerType(oper[1].Variable.Type.PrimaryType) 
				&& (instruction.InsCode == InstructionOpCode::SUB || instruction.InsCode == InstructionOpCode::ADD)))
			{
				switch (oper[1].Variable.Type.PrimaryType)
				{
					case HazeValueType::Int32:
					{
						POINTER_ADD_SUB(int, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					case HazeValueType::UInt32:
					{
						POINTER_ADD_SUB(uint32, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					case HazeValueType::Int64:
					{
						POINTER_ADD_SUB(int64, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					case HazeValueType::UInt64:
					{
						POINTER_ADD_SUB(uint64, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					default:
						break;
				}
			}*/
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
					INS_ERR_W("二元计算指针类型错误");
				}
			}
			else
			{
				INS_ERR_W("二元计算错误, <%s> <%s>", oper[0].Variable.Name.c_str(),oper[1].Variable.Name.c_str());
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

	//只能C++多参数函数调用
	static void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, int paramNum, va_list& args)
	{
		int size = 0;
		for (size_t i = 0; i < funcData->Params.size(); i++)
		{
			size += GetSizeByType(funcData->Params[i].Type, stack->m_VM);
		}
		stack->m_ESP += size;

		for (size_t i = 0; i < funcData->Params.size(); i++)
		{
			PushParamByArgs(stack, args, funcData->Params[i].Type);
		}
		stack->m_ESP += size;

		memcpy(&stack->m_StackMain[stack->m_ESP], &stack->m_PC, HAZE_ADDRESS_SIZE);
		stack->m_ESP += HAZE_ADDRESS_SIZE;

		stack->OnCall(funcData, size);
		stack->m_PC++;
		stack->ResetCallHaze();
	}

	static void PushParam(HazeStack* stack, void* src, int size)
	{
		stack->m_ESP -= size;
		memcpy(&stack->m_StackMain[stack->m_ESP], src, size);
	}

	static int PushParamByArgs(HazeStack* stack, va_list& args, const HazeDefineType& type)
	{
		HazeValue value;
		int size = GetSizeByType(type, stack->m_VM);
		void* src = nullptr;

		if (IsHazeBaseType(type.PrimaryType))
		{
			switch (type.PrimaryType)
			{
			case HazeValueType::Bool:
				src = &va_arg(args, bool);
				break;
			//在可变长参数中，会被扩展成int
			case HazeValueType::Int8:
				{
					value.Value.Int8 = va_arg(args, int);
					src = &value.Value.Int8;
				}
				break;
			case HazeValueType::UInt8:
				{
					value.Value.UInt8 = va_arg(args, int);
					src = &value.Value.UInt8;
				}
				break;
			case HazeValueType::Int16:
				{
					value.Value.Int16 = va_arg(args, int);
					src = &value.Value.Int16;
				}
				break;
			case HazeValueType::UInt16:
				{
					value.Value.UInt16 = va_arg(args, uint32);
					src = &value.Value.UInt16;
				}
				break;
			case HazeValueType::Int32:
				src = &va_arg(args, int32);
				break;
			case HazeValueType::UInt32:
				src = &va_arg(args, uint32);
				break;
			case HazeValueType::Int64:
				src = &va_arg(args, int64);
				break;
			case HazeValueType::UInt64:
				src = &va_arg(args, uint64);
				break;

			//在可变长参数中，float会被扩展成double
			case HazeValueType::Float32:
				{
					value.Value.Float32 = va_arg(args, float64);
					src = &value.Value.Float32;
				}
				break; 
			case HazeValueType::Float64:
				src = &va_arg(args, float64);
				break;
			default:
				HAZE_LOG_ERR_W("三方库调用Haze函数Push参数<%s>类型错误", GetHazeValueTypeString(type.PrimaryType));
				break;
			}

		}
		/*else if (IsPointerType(type.PrimaryType))
		{
			src = &va_arg(args, void*);
		}*/
		else
		{
			HAZE_LOG_ERR_W("三方库调用Haze函数暂时只支持默认类型!\n");
		}

		PushParam(stack, src, size);
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

void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args)
{
	InstructionProcessor::CallHazeFunction(stack, funcData, (int)funcData->Params.size(), args);
}

void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData)
{
	return InstructionProcessor::GetOperatorAddress(stack, insData);
}

//可以考虑将HashMap改为使用数组
HashMap<InstructionOpCode, void(*)(HazeStack* stack)> g_InstructionProcessor =
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

	{InstructionOpCode::CVT, &InstructionProcessor::CVT},

	{InstructionOpCode::LINE, &InstructionProcessor::Line},
};