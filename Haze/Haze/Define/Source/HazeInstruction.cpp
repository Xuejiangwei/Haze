#include "HazeHeader.h"
#include "HazeDebugDefine.h"
#include "HazeLogDefine.h"
#include "HazeVM.h"
#include "HazeStack.h"

#include "HazeLibraryManager.h"

#include <Windows.h>

#define HAZE_CALL_LOG				0

#define POINTER_ADD_SUB(T, S, STACK, OPER, INS) T v; memcpy(&v, S, sizeof(T)); \
				auto type = IsPointerPointer(OPER[0].Variable.Type.PrimaryType) ? OPER[0].Variable.Type.PrimaryType : OPER[0].Variable.Type.SecondaryType; \
				auto size = GetSizeByHazeType(type); \
				uint64 address; auto operAddress = GetOperatorAddress(STACK, OPER[0]); memcpy(&address, operAddress, sizeof(operAddress)); \
				if (INS == InstructionOpCode::SUB) address -= size * v; else address += size * v; \
				memcpy(operAddress, &address, sizeof(operAddress));

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

	{HAZE_TEXT("CVT"), InstructionOpCode::CVT },
	{HAZE_TEXT("ARRAY_LENGTH"), InstructionOpCode::ARRAY_LENGTH},

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

	HAZE_LOG_ERR_W("δ���ҵ��ֽ������������<%d>!\n", (int)code);
	return HAZE_TEXT("None");
}

InstructionOpCode GetInstructionByString(const HAZE_STRING& str)
{
	auto iter = s_HashMap_String2Code.find(str);
	if (iter != s_HashMap_String2Code.end())
	{
		return iter->second;
	}

	HAZE_LOG_ERR_W("δ���ҵ����ƶ�Ӧ�ֽ��������<%s>!\n", str.c_str());
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
	friend void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args);
	friend void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData);
#define HAZE_DEBUG_ENABLE 0
#if HAZE_DEBUG_ENABLE
	struct DataDebugScope
	{
		DataDebugScope(HazeStack* stack, const std::vector<InstructionData>& data, InstructionOpCode opCode)
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
				HAZE_LOG_ERR_W("��ʼ ������һ�洢��ַ<%p> ����������ַ<%p>", (char*)Address, GetOperatorAddress(stack, Data[1]));
				ShowData2();
				HAZE_LOG_ERR_W("\n");

				HAZE_LOG_INFO(HAZE_TEXT("ִ��ָ��<%s> <%s> <%s>\n"), GetInstructionString(opCode),
					Data[0].Variable.Name.c_str(), Data[1].Variable.Name.c_str());
			}
			else
			{
				HAZE_LOG_ERR_W("��ʼ ������һ�洢��ַ<%p>\n", (char*)Address);

				HAZE_LOG_INFO(HAZE_TEXT("ִ��ָ��<%s> <%s> \n"), GetInstructionString(opCode),
					Data[0].Variable.Name.c_str());
			}
		}

		~DataDebugScope()
		{
			memcpy(&Address, GetOperatorAddress(Stack, Data[0]), 8);
			if (Data.size() == 2)
			{
				HAZE_LOG_ERR_W("���� ������һ�洢��ַ<%p> ����������ַ<%p>", (char*)Address, GetOperatorAddress(Stack, Data[1]));
				ShowData2();
				HAZE_LOG_ERR_W("\n\n");
			}
			else
			{
				HAZE_LOG_ERR_W("���� ������һ�洢��ַ<%p>\n\n", (char*)Address);
			}
		}

		void ShowData2()
		{
			switch (Data[1].Variable.Type.PrimaryType)
			{
			case HazeValueType::Int:
			{
				int v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(int));
				HAZE_LOG_ERR_W(" ֵ<%d>", v);
			}
			break;
			case HazeValueType::Long:
			{
				int64 v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(int64));
				HAZE_LOG_ERR_W(" ֵ<%d>", v);
			}
			break;
			case HazeValueType::UnsignedInt:
			{
				uint32 v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(uint32));
				HAZE_LOG_ERR_W(" ֵ<%d>", v);
			}
			break;
			case HazeValueType::UnsignedLong:
			{
				uint64 v;
				memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(uint64));
				HAZE_LOG_ERR_W(" ֵ<%d>", v);
			}
			break;
			default:
				break;
			}
		}

	private:
		const std::vector<InstructionData>& Data;
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
			else if (IsPointerType(oper[0].Variable.Type.PrimaryType) && !IsPointerFunction(oper[0].Variable.Type.PrimaryType))
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
				else if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerPointer)
				{
					size = sizeof(char**);
				}
				else
				{
					size = 0;
					INS_ERR_W("�����ָ������");
				}

				auto address = (char*)GetOperatorAddress(stack, oper[0]);
				uint64 pointerAddressValue;
				memcpy(&pointerAddressValue, address, sizeof(address));
				pointerAddressValue += size;
				memcpy(address, &pointerAddressValue, sizeof(address));
			}
			else
			{
				INS_ERR_W("��������");
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
			else if (IsPointerType(oper[0].Variable.Type.PrimaryType) && !IsPointerFunction(oper[0].Variable.Type.PrimaryType))
			{
				uint32 size = 0;
				if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerBase)
				{
					size = GetSizeByHazeType(oper[0].Variable.Type.SecondaryType);
				}
				else if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerClass)
				{
					size = stack->m_VM->GetClassSize(oper[0].Variable.Type.CustomName);
				}
				else if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerPointer)
				{
					size = sizeof(char**);
				}
				else
				{
					size = 0;
					INS_ERR_W("�����ָ������");
				}

				auto address = (char*)GetOperatorAddress(stack, oper[0]);
				uint64 pointerAddressValue;
				memcpy(&pointerAddressValue, address, sizeof(address));
				pointerAddressValue -= size;
				memcpy(address, &pointerAddressValue, sizeof(address));
			}
			else
			{
				INS_ERR_W("��������");
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
			HAZE_LOG_ERR(HAZE_TEXT("bir neg operator error!\n"));
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
			HAZE_LOG_INFO(HAZE_TEXT("���ú���<%s>\n"), oper[0].Variable.Name.c_str());
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

						if (function.FunctionDescData.Type == InstructionFunctionType::StdLibFunction)
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
					HAZE_LOG_ERR_W("���ú���<%s>����δ���ҵ�!\n", oper[0].Variable.Name.c_str());
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

			uint64 size = GetSizeByType(newRegister->Type, stack->m_VM);
			uint64 newSize = size;
			bool isRegister = true;
			auto countAddress = GetOperatorAddress(stack, oper[1]);
			
			if (oper[1].AddressType == InstructionAddressType::Constant)
			{
				if (*((uint64*)countAddress) > 0)
				{
					CalculateValueByType(HazeValueType::UnsignedLong, InstructionOpCode::MUL,
						countAddress, &newSize);
				}
				else
				{
					isRegister = false;
				}
			}
			else
			{
				CalculateValueByType(HazeValueType::UnsignedLong, InstructionOpCode::MUL,
					countAddress, &newSize);
			}
			

			uint64 address = (uint64)stack->Alloca(newSize);
			if (isRegister)
			{
				stack->RegisterArray(address, newSize / size);
			}

			newRegister->Data.resize(sizeof(address));
			memcpy(newRegister->Data.begin()._Unwrapped(), &address, sizeof(address));
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
				INS_ERR_W("<%s>ת��Ϊ<%s>������ʱ����!", oper[1].Variable.Name.c_str(), oper[0].Variable.Name.c_str());
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void ArrayLength(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsUnsignedLongType(oper[0].Variable.Type.PrimaryType) && IsArrayType(oper[1].Variable.Type.PrimaryType))
			{
				auto arrayAddress = GetOperatorAddress(stack, oper[1]);
				uint64 address = 0;
				if (arrayAddress)
				{
					memcpy(&address, arrayAddress, sizeof(arrayAddress));
				}

				uint64 length = stack->GetRegisterArrayLength(address);

				memcpy(GetOperatorAddress(stack, oper[0]), &length, sizeof(length));
			}
			else
			{
				INS_ERR_W("<%s>��ȡ����ʱ����", oper[1].Variable.Name.c_str());
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
			/*tempAddress = (uint64)((void*)&stack->m_VM->GetFunctionByName(insData.Variable.Name));
			ret = &tempAddress;*/
			return (void*)&stack->m_VM->GetFunctionByName(insData.Variable.Name);
		}
		case InstructionAddressType::Constant:
		{
			auto& type = const_cast<HazeDefineType&>(constantValue.GetType());
			auto& value = const_cast<HazeValue&>(constantValue.GetValue());

			type.PrimaryType = insData.Variable.Type.PrimaryType;
			StringToHazeValueNumber(insData.Variable.Name, type.PrimaryType, value);
			ret = GetBinaryPointer(type.PrimaryType, value);
			return ret;
		}
		case InstructionAddressType::NullPtr:
		{
			auto& type = const_cast<HazeDefineType&>(constantValue.GetType());
			auto& value = const_cast<HazeValue&>(constantValue.GetValue());

			type.PrimaryType = insData.Variable.Type.PrimaryType;
			StringToHazeValueNumber(HAZE_TEXT("0"), type.PrimaryType, value);
			ret = GetBinaryPointer(type.PrimaryType, value);
			return ret;
		}
		case InstructionAddressType::ConstantString:
		{
			tempAddress = (uint64)stack->m_VM->GetConstantStringByIndex(insData.Extra.Index);
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
			if (IsNumberType(oper[0].Variable.Type.PrimaryType) && oper[0].Variable.Type == oper[1].Variable.Type)
			{
				CalculateValueByType(oper[0].Variable.Type.PrimaryType, instruction.InsCode, 
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[0]));
			}
			else if (IsPointerType(oper[0].Variable.Type.PrimaryType) && 
				(oper[0].Variable.Type == oper[1].Variable.Type && instruction.InsCode == InstructionOpCode::SUB))
			{

			}
			else if (IsPointerType(oper[0].Variable.Type.PrimaryType) && (IsIntegerType(oper[1].Variable.Type.PrimaryType) 
				&& (instruction.InsCode == InstructionOpCode::SUB || instruction.InsCode == InstructionOpCode::ADD)))
			{
				switch (oper[1].Variable.Type.PrimaryType)
				{
					case HazeValueType::Int:
					{
						POINTER_ADD_SUB(int, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					case HazeValueType::UnsignedInt:
					{
						POINTER_ADD_SUB(uint32, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					case HazeValueType::Long:
					{
						POINTER_ADD_SUB(int64, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					case HazeValueType::UnsignedLong:
					{
						POINTER_ADD_SUB(uint64, GetOperatorAddress(stack, oper[1]), stack, oper, instruction.InsCode);
					}
					break;
					default:
						break;
				}
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
					INS_ERR_W("��Ԫ����ָ�����ʹ���");
				}
			}
			else
			{
				INS_ERR_W("��Ԫ�������, <%s> <%s>", oper[0].Variable.Name.c_str(),oper[1].Variable.Name.c_str());
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

	//ֻ��C++�������������
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

		if (IsHazeDefaultType(type.PrimaryType))
		{
			switch (type.PrimaryType)
			{
			case HazeValueType::Bool:
				src = &va_arg(args, bool);
				break;
			//�ڿɱ䳤�����У��ᱻ��չ��int
			case HazeValueType::Byte:
				{
					value.Value.Byte = va_arg(args, int);
					src = &value.Value.Byte;
				}
				break;
			case HazeValueType::UnsignedByte:
				{
					value.Value.UnsignedByte = va_arg(args, int);
					src = &value.Value.UnsignedByte;
				}
				break;
			case HazeValueType::Char:
				{
					value.Value.Char = va_arg(args, int);
					src = &value.Value.Char;
				}
				break;
			case HazeValueType::Short:
				{
					value.Value.Short = va_arg(args, int);
					src = &value.Value.Short;
				}
				break;
			case HazeValueType::UnsignedShort:
				{
					value.Value.UnsignedShort = va_arg(args, int);
					src = &value.Value.UnsignedShort;
				}
				break;
			case HazeValueType::Int:
				src = &va_arg(args, int);
				break;
			case HazeValueType::Long:
				src = &va_arg(args, int64);
				break;
			//�ڿɱ䳤�����У�float�ᱻ��չ��double
			case HazeValueType::Float:
				{
					value.Value.Float = va_arg(args, double);
					src = &value.Value.Float;
				}
				break; 
			case HazeValueType::Double:
				src = &va_arg(args, double);
				break;
			case HazeValueType::UnsignedInt:
				src = &va_arg(args, uint32);
				break;
			case HazeValueType::UnsignedLong:
				src = &va_arg(args, uint64);
				break;
			default:
				HAZE_LOG_ERR_W("���������Haze����Push����<%s>���ʹ���", GetHazeValueTypeString(type.PrimaryType));
				break;
			}

		}
		else if (IsPointerType(type.PrimaryType))
		{
			src = &va_arg(args, void*);
		}
		else
		{
			HAZE_LOG_ERR_W("���������Haze������ʱֻ֧��Ĭ������!\n");
		}

		PushParam(stack, src, size);
		return size;
	}

	static void ClearRegisterType(HazeStack* stack, const InstructionData& oper)
	{
		//New��Ret�Ĵ�����type��գ���ֹû���������յ�
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

//���Կ��ǽ�HashMap��Ϊʹ������
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

	{InstructionOpCode::CVT, &InstructionProcessor::CVT},
	{InstructionOpCode::ARRAY_LENGTH, &InstructionProcessor::ArrayLength},

	{InstructionOpCode::LINE, &InstructionProcessor::Line},
};