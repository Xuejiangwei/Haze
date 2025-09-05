#include "HazePch.h"
#include "HazeHeader.h"
#include "HazeDebugDefine.h"
#include "HazeLogDefine.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeLibraryManager.h"
#include "ObjectArray.h"
#include "ObjectClass.h"
#include "ObjectString.h"
#include "ObjectBase.h"
#include "ObjectHash.h"
#include "ObjectClosure.h"
#include "HazeMemory.h"
#include "Compiler.h"

#include <Windows.h>

#define HAZE_CALL_LOG				0
#define HAZE_DEBUG_ENABLE			0

#define POINTER_ADD_SUB(T, S, STACK, OPER, INS) T v; memcpy(&v, S, sizeof(T)); \
				auto type = OPER[0].Variable.Type.SecondaryType; \
				auto size = GetSizeByHazeType(type); \
				x_uint64 address; auto operAddress = GetOperatorAddress(STACK, OPER[0]); memcpy(&address, operAddress, sizeof(operAddress)); \
				if (INS == InstructionOpCode::SUB) address -= size * v; else address += size * v; \
				memcpy(operAddress, &address, sizeof(operAddress));

extern Unique<HazeLibraryManager> g_HazeLibManager;

// 声明指令处理函数，否则Lib库有报错
extern const A_Array<void(*)(HazeStack* stack), ((x_uint32)InstructionOpCode::LINE + 1)> g_InstructionProcessor;

static HashMap<STDString, InstructionOpCode> s_HashMap_String2Code =
{
#define HAZE_OP_CODE_DEFINE(OP_CODE) { H_TEXT(#OP_CODE), InstructionOpCode::OP_CODE },
	#include "HazeOpCodeTemplate"
#undef HAZE_OP_CODE_DEFINE
};

bool IsRegisterDesc(HazeDataDesc desc)
{
	return HazeDataDesc::RegisterBegin < desc && desc < HazeDataDesc::RegisterEnd;
}

bool IsConstDesc(HazeDataDesc desc)
{
	return desc == HazeDataDesc::ConstantValue;
}

bool IsConstStringDesc(HazeDataDesc desc)
{
	return desc == HazeDataDesc::ConstantString;
}

bool IsClassMember(HazeDataDesc desc)
{
	return desc >= HazeDataDesc::ClassMember_Local_Public && desc <= HazeDataDesc::ClassMember_Local_Private;
}

bool IsGlobalDesc(HazeDataDesc desc)
{
	return desc == HazeDataDesc::Variable_Global || IsConstDesc(desc) || IsConstStringDesc(desc) || desc == HazeDataDesc::NullPtr;
}

bool IsLocalDesc(HazeDataDesc desc)
{
	return desc == HazeDataDesc::Variable_Local || desc == HazeDataDesc::RegisterTemp || desc == HazeDataDesc::Element;
}

const x_HChar* GetInstructionString(InstructionOpCode code)
{
	static HashMap<InstructionOpCode, const x_HChar*> s_HashMap_Code2String;

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

InstructionOpCode GetInstructionByString(const STDString& str)
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

bool IsArithmeticOpCode(InstructionOpCode opcode)
{
	return opcode >= InstructionOpCode::ADD && opcode == InstructionOpCode::SHR;
}

bool IsComparisonOpCode(InstructionOpCode opcode)
{
	return opcode == InstructionOpCode::CMP;
}

bool IsCallOpCode(InstructionOpCode opcode)
{
	return opcode == InstructionOpCode::CALL;
}

bool IsMovOpCode(InstructionOpCode opcode)
{
	return opcode >= InstructionOpCode::MOV && opcode <= InstructionOpCode::LEA;
}

HazeVirtualRegister GetVirtualRegisterByDesc(HazeDataDesc desc)
{
	switch (desc)
	{
		case HazeDataDesc::RegisterRet:
			return HazeVirtualRegister::RET;
		case HazeDataDesc::RegisterCmp:
			return HazeVirtualRegister::CMP;
		default:
			break;
	}

	return HazeVirtualRegister::_END;
}

void CallHazeFunction(HazeStack* stack, FunctionData* funcData, va_list& args);

static HashSet<InstructionOpCode> s_DebugCode = {
		{ InstructionOpCode::MOV },
		{ InstructionOpCode::MOVPV },
		{ InstructionOpCode::MOVTOPV },
		{ InstructionOpCode::LEA },
		{ InstructionOpCode::ADD },
		{ InstructionOpCode::SUB },
		{ InstructionOpCode::MUL },
		{ InstructionOpCode::DIV },
		{ InstructionOpCode::MOD },
		{ InstructionOpCode::NEG },
		{ InstructionOpCode::NOT },
		{ InstructionOpCode::BIT_AND },
		{ InstructionOpCode::BIT_OR },
		{ InstructionOpCode::BIT_NEG },
		{ InstructionOpCode::BIT_XOR },
		{ InstructionOpCode::SHL },
		{ InstructionOpCode::SHR },
		{ InstructionOpCode::PUSH },
		{ InstructionOpCode::POP },
		{ InstructionOpCode::CALL },
		{ InstructionOpCode::NEW },
		{ InstructionOpCode::CMP },
		{ InstructionOpCode::JMP },
		{ InstructionOpCode::JNE },
		{ InstructionOpCode::JNG },
		{ InstructionOpCode::JNL },
		{ InstructionOpCode::JE },
		{ InstructionOpCode::JG },
		{ InstructionOpCode::JL },
		{ InstructionOpCode::CVT },
		{ InstructionOpCode::LINE },
		{ InstructionOpCode::RET },
};

class InstructionProcessor
{
	friend void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args);
	friend void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData);
#if HAZE_DEBUG_ENABLE
	struct DataDebugScope
	{
		DataDebugScope(HazeStack* stack, const V_Array<InstructionData>& data, InstructionOpCode opCode)
			: Stack(stack), Data(data), OpCode(opCode)
		{
			if (s_DebugCode.find(OpCode) == s_DebugCode.end())
			{
				return;
			}

			auto address = GetOperatorAddress(Stack, Data[0]);
			if (address && !IsNoneType(Data[0].Type) && Data[0].Type != HazeValueType::ObjectFunction)
			{
				memcpy(&Address, address, GetSizeByHazeType(Data[0].Type));
			}
			else
			{
				Address = 0;
			}

			if (Data.size() == 2)
			{
				HAZE_LOG_ERR_W("开始 操作数一地址<%p> 存储地址<%p> 操作数二地址<%p> EBP<%d> ESP<%d>", address, (char*)Address, GetOperatorAddress(stack, Data[1]),
					Stack->m_EBP, Stack->m_ESP);
				ShowData2();
				HAZE_LOG_ERR_W("\n");

				HAZE_LOG_INFO(H_TEXT("执行指令<%s> \n"), GetInstructionString(opCode));
			}
			else
			{
				HAZE_LOG_ERR_W("开始 操作数一地址<%p> 存储地址<%p> EBP<%d> ESP<%d>\n", address, (char*)Address, Stack->m_EBP, Stack->m_ESP);

				HAZE_LOG_INFO(H_TEXT("执行指令<%s> \n"), GetInstructionString(opCode));
			}
		}

		~DataDebugScope()
		{
			if (s_DebugCode.find(OpCode) == s_DebugCode.end())
			{
				return;
			}

			auto address = GetOperatorAddress(Stack, Data[0]);
			if (address && !IsNoneType(Data[0].Type) && Data[0].Type != HazeValueType::ObjectFunction)
			{
				memcpy(&Address, address, GetSizeByHazeType(Data[0].Type));
			}
			
			if (Data.size() == 2)
			{
				HAZE_LOG_ERR_W("结束 操作数一地址<%p> 存储地址<%p> 操作数二地址<%p> EBP<%d> ESP<%d>", address, (char*)Address,
					GetOperatorAddress(Stack, Data[1]), Stack->m_EBP, Stack->m_ESP);
				ShowData2();
				HAZE_LOG_ERR_W("\n\n");
			}
			else
			{
				HAZE_LOG_ERR_W("结束 操作数一地址<%p> 存储地址<%p> EBP<%d> ESP<%d>\n\n", address, (char*)Address, Stack->m_EBP, Stack->m_ESP);
			}
		}

		void ShowData2()
		{
			switch (Data[1].Type)
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
					x_int64 v;
					memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(x_int64));
					HAZE_LOG_ERR_W(" 值<%d>", v);
				}
				break;
				case HazeValueType::UInt32:
				{
					x_uint32 v;
					memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(x_uint32));
					HAZE_LOG_ERR_W(" 值<%d>", v);
				}
				break;
				case HazeValueType::UInt64:
				{
					x_uint64 v;
					memcpy(&v, GetOperatorAddress(Stack, Data[1]), sizeof(x_uint64));
					HAZE_LOG_ERR_W(" 值<%d>", v);
				}
				break;
				case HazeValueType::String:
				case HazeValueType::Array:
				case HazeValueType::Class:
				{
					HAZE_LOG_ERR_W(" 值<%p>", GetOperatorAddress(Stack, Data[1]));
				}
				break;
				default:
					break;
			}
		}

	private:
		const V_Array<InstructionData>& Data;
		x_uint64 Address;
		HazeStack* Stack;
		InstructionOpCode OpCode;
	};
	#define INSTRUCTION_DATA_DEBUG DataDebugScope debugScope(stack, stack->m_VM->m_Instructions[stack->m_PC].Operator, stack->m_VM->m_Instructions[stack->m_PC].InsCode)
#else
	#define INSTRUCTION_DATA_DEBUG
#endif

public:
	static inline x_uint32 GetOperSize(HazeStack* stack, const InstructionData& instData)
	{
		if (!(IsDynamicClassUnknowType(instData.Type) && instData.Desc == HazeDataDesc::RegisterTemp))
		{
			return GetSizeByHazeType(instData.Type);
		}

		// 看还需不需要加临时变量的偏移，可能索引不需要，寻找字节偏移需要
		return stack->GetTempRegisterType(instData.VariableIndexOrId).GetTypeSize();
	}

public:
	static void NONE(HazeStack* stack)
	{
		INS_ERR_W("操作码为空");
	}

	static void MOV(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			const void* src = GetOperatorAddress(stack, oper[1]);
			memcpy(dst, src, GetSizeByHazeType(oper[0].Type));

			ClearRegisterType(stack, oper[1]);
		}

		stack->m_VM->InstructionExecPost();
	}

	static void MOVPV(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			const void* src = GetOperatorAddress(stack, oper[1]);

			x_uint64 address = 0;
			memcpy(&address, src, sizeof(address));
			memcpy(dst, (void*)address, GetSizeByHazeType(oper[0].Type));

			ClearRegisterType(stack, oper[1]);
		}

		stack->m_VM->InstructionExecPost();
	}

	static void MOVTOPV(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			const void* src = GetOperatorAddress(stack, oper[1]);

			x_uint64 address = 0;
			memcpy(&address, dst, sizeof(address));
			memcpy((void*)address, src, GetSizeByHazeType(oper[1].Type));

			ClearRegisterType(stack, oper[1]);
		}

		stack->m_VM->InstructionExecPost();
	}

	static void LEA(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			void* dst = GetOperatorAddress(stack, oper[0]);
			x_uint64 address = (x_uint64)GetOperatorAddress(stack, oper[1]);
			memcpy(dst, &address, GetSizeByHazeType(oper[0].Type));
		}

		stack->m_VM->InstructionExecPost();
	}

	static void PUSH(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			int size = GetSizeByHazeType(oper[0].Type);
			if (oper[0].Desc == HazeDataDesc::Address)
			{
				memcpy(&stack->m_StackMain[stack->m_ESP], &stack->m_PC, HAZE_ADDRESS_SIZE);
			}
			else if (oper[0].Desc == HazeDataDesc::ClassThis)
			{
				memcpy(&stack->m_StackMain[stack->m_ESP], GetOperatorAddress(stack, oper[0]), sizeof(x_uint64));
			}
			else/* if (Operator[0].Scope == InstructionScopeType::Local || oper[0].Scope == InstructionScopeType::Global)*/
			{
				if (oper[0].Extra.Address.BaseAddress + (int)stack->m_EBP >= 0)
				{
					size = GetOperSize(stack, oper[0]);
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

	static void POP(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			auto size = GetOperSize(stack, oper[0]);
			/*auto address = GetOperatorAddress(stack, oper[0]);
			memcpy(address, &stack->m_StackMain[stack->m_ESP - size], size);*/
			
			stack->m_ESP -= size;
		}

		stack->m_VM->InstructionExecPost();
	}

	static void ADD(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void SUB(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
	
		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void MUL(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void DIV(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		BinaryOperator(stack);

		stack->m_VM->InstructionExecPost();
	}

	static void MOD(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 3)
		{
			if (IsNumberType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::MOD, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[1]), stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void NEG(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::NEG, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), nullptr, stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void NOT(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::NOT, GetOperatorAddress(stack, oper[0]), 
					GetOperatorAddress(stack, oper[1]), nullptr, stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void BIT_AND(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 3)
		{
			if (IsNumberType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::BIT_AND, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[2]), stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void BIT_OR(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 3)
		{
			if (IsNumberType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::BIT_OR, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[2]), stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void BIT_XOR(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
	
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 3)
		{
			if (IsNumberType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::BIT_XOR, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[2]), stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void BIT_NEG(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG; 
		
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsIntegerType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::BIT_NEG, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), nullptr, stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}


		stack->m_VM->InstructionExecPost();
	}

	static void SHL(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 3)
		{
			if (IsNumberType(oper[0].Type) && IsIntegerType(oper[1].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::SHL, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[2]), stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void SHR(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 3)
		{
			if (IsNumberType(oper[0].Type))
			{
				CalculateValueByType(oper[0].Type, InstructionOpCode::SHR, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[2]), stack);
			}
		}
		else
		{
			INS_ERR_W("操作数不对");
		}

		stack->m_VM->InstructionExecPost();
	}

	static void CALL(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			memcpy(&stack->m_StackMain[stack->m_ESP - HAZE_ADDRESS_SIZE], &stack->m_PC, HAZE_ADDRESS_SIZE);

			if (oper[0].Type == HazeValueType::ObjectFunction)
			{
				x_uint32 tempEBP = stack->m_EBP;
				stack->m_EBP = stack->m_ESP;
				int paramByteSize = 0;
				
#if HAZE_CALL_LOG
				HString functionName = stack->m_VM->GetAdvanceFunctionName((x_uint16)oper[0].Extra.ObjectCall.Index);;
				HAZE_LOG_INFO(H_TEXT("调用对象<%s>函数<%s> EBP<%d>  ESP<%d>\n"), functionName.c_str(), functionName.c_str(), stack->m_EBP, stack->m_ESP);
#endif
				
				stack->m_VM->GetAdvanceFunction((x_uint16)oper[0].Extra.ObjectCall.Index)->ClassFunc(stack, oper[0].Extra.Call.ParamNum, paramByteSize);

				if (paramByteSize >= 0)
				{
					stack->m_ESP -= (paramByteSize + HAZE_ADDRESS_SIZE);
					stack->m_EBP = tempEBP;
				}
			}
			else if (oper[0].Type == HazeValueType::Function)
			{
//#if HAZE_CALL_LOG
//				HAZE_LOG_INFO(H_TEXT("调用函数<%s> EBP<%d>  ESP<%d>\n"), oper[0].Variable.Name.c_str(), stack->m_EBP, stack->m_ESP);
//#endif
				void* value = GetOperatorAddress(stack, oper[0]);
				x_uint64 functionAddress;
				memcpy(&functionAddress, (char*)value, sizeof(functionAddress));
				stack->OnCall((FunctionData*)functionAddress, oper[0].Extra.Call.ParamByteSize);
			}
			else
			{
				int functionIndex = -1;
				auto symbolName = stack->GetVM()->GetTypeInfoMap()->GetTypeName(oper[0].VariableIndexOrId);
				if (oper[0].AddressType == InstructionAddressType::FunctionDynamicAddress)
				{
					
					auto obj = ((ObjectClass**)(GetOperatorAddress(stack, stack->m_VM->m_Instructions[stack->m_PC - 2].Operator[0])));
					functionIndex = (*obj)->m_ClassInfo->Functions.find(*symbolName)->second;
					/*auto obj = ((ObjectClass**)(GetOperatorAddress(stack, stack->m_VM->m_Instructions[stack->m_PC - 2].Operator[0])));
					functionIndex = (*obj)->m_ClassInfo->Functions.find(oper[0].Variable.Name)->second;*/
				}
				else
				{
					functionIndex = stack->m_VM->GetFucntionIndexByName(*symbolName);
				}

#if HAZE_CALL_LOG
				HAZE_LOG_INFO(H_TEXT("调用函数<%s> EBP<%d>  ESP<%d>\n"), symbolName->c_str(), stack->m_EBP, stack->m_ESP);
#endif
				 
				if (functionIndex >= 0)
				{
					auto& function = stack->m_VM->m_FunctionTable[functionIndex];
					if (function.FunctionDescData.Type == InstructionFunctionType::HazeFunction)
					{
						stack->OnCall(&function, oper[0].Extra.Call.ParamByteSize);
					}
					else
					{
						x_uint32 tempEBP = stack->m_EBP;
						stack->m_EBP = stack->m_ESP;

						if (function.FunctionDescData.Type == InstructionFunctionType::StaticLibFunction)
						{
							function.FunctionDescData.StdLibFunction(stack, oper[0].Extra.Call.ParamNum, oper[0].Extra.Call.ParamByteSize);
						}
						/*else if (function.FunctionDescData.Type == InstructionFunctionType::DLLLibFunction)
						{
							HazeRegister* retRegister = stack->GetVirtualRegister(HazeVirtualRegister::RET);
							retRegister->Type = function.Type;
							int size = retRegister->Type.GetTypeSize();
							retRegister->Data.resize(size);

							x_uint64 address = (x_uint64)(stack);
							memcpy(&stack->m_StackMain[stack->m_ESP], &address, sizeof(address));

							auto symbolName = stack->GetVM()->GetTypeInfoMap()->GetTypeName(oper[0].VariableIndexOrId);
							g_HazeLibManager->ExecuteDLLFunction(oper[1].Variable.Name, oper[0].Variable.Name,
								&stack->m_StackMain[stack->m_ESP - HAZE_ADDRESS_SIZE], retRegister->Data.begin()._Unwrapped(),
								stack);
						}*/

						stack->m_ESP -= (oper[0].Extra.Call.ParamByteSize + HAZE_ADDRESS_SIZE);
						stack->m_EBP = tempEBP;
					}
				}
				else
				{
					HAZE_LOG_ERR_W("调用函数错误，未能找到!\n");
				}
				
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void RET(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			HazeRegister* retRegister = stack->GetVirtualRegister(HazeVirtualRegister::RET);
			//retRegister->Type = oper[0].Variable.Type;

			int size = GetSizeByHazeType(oper[0].Type);

			retRegister->Data.resize(size);
			memcpy(retRegister->Data.begin()._Unwrapped(), GetOperatorAddress(stack, oper[0]), size);
		}
		stack->OnRet();

		stack->m_VM->InstructionExecPost();
	}

	static void NEW(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		//HazeRegister* newRegister = stack->GetVirtualRegister(oper[0].Variable.Name.c_str());
		auto& type = stack->GetTempRegisterType(oper[0].VariableIndexOrId);
		if (oper.size() == 2)
		{
			auto countAddress = GetOperatorAddress(stack, oper[1]);
			
			auto count = *((x_uint64*)countAddress);
			Pair<void*, x_uint32> address = { nullptr, 0 };
			if (count > 0)
			{
				x_uint64* lengths = new x_uint64[count];
				for (x_uint64 i = 0; i < count; i++)
				{
					auto& instructionData = stack->m_VM->m_Instructions[stack->m_PC + i + 1];

					HazeValue v1, v2;
					memcpy(&v2, GetOperatorAddress(stack, instructionData.Operator[0]), 
						GetSizeByHazeType(instructionData.Operator[0].Type));
					ConvertBaseTypeValue(HazeValueType::UInt64, v1, instructionData.Operator[0].Type, v2);

					lengths[i] = v1.Value.UInt64;
				}

				address = HazeMemory::AllocaGCData(sizeof(ObjectArray), GC_ObjectType::Array);
				new((char*)address.first) ObjectArray(address.second, stack->m_VM, type.TypeId, lengths);

				delete[] lengths;
			}
			else if (IsStringType(type.BaseType))
			{
				address = HazeMemory::AllocaGCData(sizeof(ObjectString), GC_ObjectType::String);
				new(address.first) ObjectString(address.second, nullptr);
			}
			else if (IsClassType(type.BaseType))
			{
				address = HazeMemory::AllocaGCData(sizeof(ObjectClass), GC_ObjectType::Class);
				new(address.first) ObjectClass(address.second, stack->m_VM, type.TypeId);
			}
			else if (IsHashType(type.BaseType))
			{
				address = HazeMemory::AllocaGCData(sizeof(ObjectHash), GC_ObjectType::Hash);
				new(address.first) ObjectHash(address.second, stack->m_VM, type.TypeId);
			}
			else if (IsObjectBaseType(type.BaseType))
			{
				address = HazeMemory::AllocaGCData(sizeof(ObjectBase), GC_ObjectType::ObjectBase);
				new(address.first) ObjectBase(address.second, stack->m_VM, type.TypeId);
			}
			else if (IsClosureType(type.BaseType))
			{
				auto& frame = stack->GetCurrFrame();
				address = HazeMemory::AllocaGCData(sizeof(ObjectClosure), GC_ObjectType::Closure);
				new(address.first) ObjectClosure(address.second, ((FunctionData**)GetOperatorAddress(stack, oper[0]))[0], frame.FunctionInfo, &stack->m_StackMain[frame.CurrParamESP]);
			}
			else
			{
				HAZE_LOG_ERR_W("NEW<%s>错误 只能生成<类><数组><字符串><基本类型><闭包>!\n", stack->m_VM->GetTypeInfoMap()->GetTypeName(type.TypeId));
				//address = HazeMemory::Alloca(newSize);
			}

			/*newRegister->Data.resize(sizeof(address));
			memcpy(newRegister->Data.begin()._Unwrapped(), &address, sizeof(address));*/

			auto dst = GetOperatorAddress(stack, oper[0]);
			memcpy(dst, &address.first, sizeof(address.first));
			
			//stack->m_PC += (uint32)count;
		}

		stack->m_VM->InstructionExecPost();
	}

	static void CMP(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(HazeVirtualRegister::CMP);
			auto strongerType = GetStrongerType(oper[0].Type, oper[1].Type);
			CompareValueByType(strongerType != HazeValueType::None ? strongerType : 
				(GetSizeByHazeType(oper[0].Type) == GetSizeByHazeType(oper[1].Type)) ? oper[0].Type : HazeValueType::None,
				cmpRegister, GetOperatorAddress(stack, oper[0]), GetOperatorAddress(stack, oper[1]));
		}

		stack->m_VM->InstructionExecPost();
	}

	static void JMP(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 1)
		{
			JmpToOperator(stack, oper[0]);
		}

		stack->m_VM->InstructionExecPost();
	}

	static void JNE(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

#define REGISTER_EQUAL(R) R->Data[0] == 1
#define REGISTER_GREATER(R) R->Data[1] == 1
#define REGISTER_LESS(R) R->Data[2] == 1

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(HazeVirtualRegister::CMP);

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

	static void JNG(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(HazeVirtualRegister::CMP);

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

	static void JNL(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(HazeVirtualRegister::CMP);

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

	static void JE(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(HazeVirtualRegister::CMP);

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

	static void JG(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(HazeVirtualRegister::CMP);

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

	static void JL(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			HazeRegister* cmpRegister = stack->GetVirtualRegister(HazeVirtualRegister::CMP);

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

		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			if (IsNumberType(oper[0].Type) && (IsNumberType(oper[1].Type) || IsEnumType(oper[1].Type)))
			{
				HazeValue v1, v2;
				auto address = GetOperatorAddress(stack, oper[0]);

				memcpy(&v1, address, GetSizeByHazeType(oper[0].Type));
				memcpy(&v2, GetOperatorAddress(stack, oper[1]), GetSizeByHazeType(oper[1].Type));
				ConvertBaseTypeValue(oper[0].Type, v1, oper[1].Type, v2);
				memcpy(address, &v1, GetSizeByHazeType(oper[0].Type));
			}
			else if (IsClassType(oper[0].Type) && IsDynamicClassType(oper[1].Type))
			{
				void* dst = GetOperatorAddress(stack, oper[0]);
				const void* src = GetOperatorAddress(stack, oper[1]);
				memcpy(dst, src, GetSizeByHazeType(oper[0].Type));
			}
			else
			{
				INS_ERR_W("<%s>转换为<%s>的类型时错误", GetHazeValueTypeString(oper[1].Type), GetHazeValueTypeString(oper[0].Type));
			}
		}

		stack->m_VM->InstructionExecPost();
	}

	static void MOV_DCU(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
		if (oper.size() == 2)
		{
			static HazeVariableType s_Type;
			void* dst = GetOperatorAddress(stack, oper[0]);

			// 若是寄存器的话需要获取类型，并在下面重新设置以保存引用
			if (oper[1].AddressType == InstructionAddressType::Register)
			{
				s_Type = stack->GetVirtualRegister(GetVirtualRegisterByDesc(oper[1].Desc))->Type;
			}
			else if (IsDynamicClassUnknowType(oper[0].Type))
			{
				s_Type = HAZE_VAR_TYPE(oper[1].Type);
			}
			else
			{
				s_Type = HAZE_VAR_TYPE(HazeValueType::None);
			}

			const void* src = GetOperatorAddress(stack, oper[1]);
			memcpy(dst, src, s_Type.GetTypeSize());

			if (!IsNoneType(s_Type.BaseType))
			{
				stack->ResetTempRegisterTypeByDynamicClassUnknow(oper[0].VariableIndexOrId, s_Type);
			}

			ClearRegisterType(stack, oper[1]);
		}

		stack->m_VM->InstructionExecPost();
	}

	static void LINE(HazeStack* stack)
	{
		INSTRUCTION_DATA_DEBUG;
		const auto& oper = stack->m_VM->m_Instructions[stack->m_PC].Operator;
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
		thread_local static x_uint64 tempAddress;

		void* ret = nullptr;

		switch (insData.AddressType)
		{
			case InstructionAddressType::Global:
			{
				ret = stack->m_VM->GetGlobalValueById(insData.VariableIndexOrId);
			}
				break;
			case InstructionAddressType::Local:
			{
				ret = &stack->m_StackMain[stack->m_EBP + insData.Extra.Address.BaseAddress];
			}
				break;
			case InstructionAddressType::FunctionAddress:
			{
				/*tempAddress = (uint64)((void*)&stack->m_VM->GetFunctionByName(insData.Variable.Name));
				ret = &tempAddress;*/
				auto symbol = stack->m_VM->GetTypeInfoMap()->GetTypeName(insData.VariableIndexOrId);
				ret = (void*)&stack->m_VM->GetFunctionByName(*symbol);
			}
				break;
			case InstructionAddressType::Constant:
			{
				auto& value = const_cast<HazeValue&>(insData.Extra.RuntimeDynamicValue);
				//StringToHazeValueNumber(insData.Variable.Name, insData.Variable.Type.BaseType, value);
				ret = GetBinaryPointer(insData.Type, value);

				if (insData.Type == HazeValueType::Enum)
				{
					ret = ret;
				}

			}
				break;
			case InstructionAddressType::NullPtr:
			{
				auto& value = const_cast<HazeValue&>(insData.Extra.RuntimeDynamicValue);
				StringToHazeValueNumber(H_TEXT("0"), insData.Type, value);
				ret = GetBinaryPointer(insData.Type, value);
			}
				break;
			case InstructionAddressType::ConstantString:
			{
				tempAddress = (x_uint64)stack->m_VM->GetConstantStringByIndex(insData.Extra.Index);
				ret = &tempAddress;
			}
				break;
			case InstructionAddressType::Register:
			{
				HazeRegister* hazeRegister = stack->GetVirtualRegister(GetVirtualRegisterByDesc(insData.Desc));

				if (hazeRegister->Type.BaseType != insData.Type)
				{
					//hazeRegister->Type = insData.Type;
					//HAZE_LOG_INFO_W("获取寄存器数据\n");
					hazeRegister->Data.resize(GetSizeByHazeType(insData.Type));
				}

				ret = hazeRegister->Data.begin()._Unwrapped();
			}
				break;
			/*case InstructionAddressType::PureString:
			{
				tempAddress = (x_uint64)(&insData.Variable.Name);
				ret = &tempAddress;
			}
				break;*/
			default:
				break;
		}

		return ret;
	}

	static void JmpToOperator(HazeStack* stack, const InstructionData& insData)
	{
		if (insData.Extra.Index != -1)
		{
			stack->JmpTo(insData);
		}
	}

	static void BinaryOperator(HazeStack* stack)
	{
		const auto& instruction = stack->m_VM->m_Instructions[stack->m_PC];
		const auto& oper = instruction.Operator;
		if (oper.size() == 3)
		{
			if (IsNumberType(oper[1].Type) && oper[1].Type == oper[2].Type)
			{
				CalculateValueByType(oper[1].Type, instruction.InsCode, GetOperatorAddress(stack, oper[0]),
					GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[2]), stack);
			}
			else if (oper[0].AddressType == InstructionAddressType::Register && 
				IsDynamicClassUnknowType(oper[1].Type) || IsDynamicClassUnknowType(oper[2].Type))
			{
				auto& operType1 = oper[1].Desc == HazeDataDesc::RegisterTemp ? stack->GetTempRegisterType(oper[1].VariableIndexOrId).BaseType : oper[1].Type;
				auto& operType2 = oper[2].Desc == HazeDataDesc::RegisterTemp ? stack->GetTempRegisterType(oper[2].VariableIndexOrId).BaseType : oper[2].Type;

				if (IsNumberType(operType1) && IsNumberType(operType2) && operType1 == operType2)
				{
					CalculateValueByType(operType1, instruction.InsCode, GetOperatorAddress(stack, oper[0]),
						GetOperatorAddress(stack, oper[1]), GetOperatorAddress(stack, oper[2]), stack);
					
					stack->ResetTempRegisterTypeByDynamicClassUnknow(oper[0].VariableIndexOrId, HAZE_VAR_TYPE(operType1));
				}
				else
				{
					INS_ERR_W("二元计算错误, 动态类成员不全是数字");
				}
			}
			else
			{
				INS_ERR_W("二元计算错误");
			}
		}
	}

	/*static void ExeHazePointerFunction(void* stackPointer, void* value, int paramNum, ...)
	{
		va_list args;
		va_start(args, paramNum);
		CallHazeFunction((HazeStack*)stackPointer, (FunctionData*)value, paramNum, args);
		va_end(args);
	}*/

	//只能C++多参数函数调用
	static void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, int paramNum, va_list& args)
	{
		assert(paramNum == funcData->Params.size());

		stack->m_EBP = stack->m_ESP;

		int size = 0;
		for (size_t i = 0; i < funcData->Params.size(); i++)
		{
			size += funcData->Params[i].Type.GetTypeSize();
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

	static int PushParamByArgs(HazeStack* stack, va_list& args, const HazeVariableType& type)
	{
		HazeValue value;
		int size = type.GetTypeSize();
		void* src = nullptr;

		switch (type.BaseType)
		{
			case HazeValueType::Bool:
				src = &va_arg(args, bool);
				break;
			//在可变长参数中，会被扩展成int
			case HazeValueType::Int8:
				{
					value.Value.Int8 = (x_int8)va_arg(args, int);
					src = &value.Value.Int8;
				}
				break;
			case HazeValueType::UInt8:
				{
					value.Value.UInt8 = (x_uint8)va_arg(args, int);
					src = &value.Value.UInt8;
				}
				break;
			case HazeValueType::Int16:
				{
					value.Value.Int16 = (x_int16)va_arg(args, int);
					src = &value.Value.Int16;
				}
				break;
			case HazeValueType::UInt16:
				{
					value.Value.UInt16 = (x_uint16)va_arg(args, x_uint32);
					src = &value.Value.UInt16;
				}
				break;
			case HazeValueType::Int32:
				src = &va_arg(args, x_int32);
				break;
			case HazeValueType::UInt32:
				src = &va_arg(args, x_uint32);
				break;
			case HazeValueType::Int64:
				src = &va_arg(args, x_int64);
				break;
			case HazeValueType::UInt64:
				src = &va_arg(args, x_uint64);
				break;

			//在可变长参数中，float会被扩展成double
			case HazeValueType::Float32:
				{
					value.Value.Float32 = (x_float32)va_arg(args, x_float64);
					src = &value.Value.Float32;
				}
				break; 
			case HazeValueType::Float64:
				src = &va_arg(args, x_float64);
				break;

			case HazeValueType::Array:
			case HazeValueType::String:
			case HazeValueType::Class:
			case HazeValueType::DynamicClass:
			case HazeValueType::ObjectBase:
			case HazeValueType::Hash:
				src = &va_arg(args, x_uint64);
				break;
			default:
				INS_ERR_W("三方库调用Haze函数Push参数<%s>类型错误", GetHazeValueTypeString(type.BaseType));
				break;
		}

		
		PushParam(stack, src, size);
		return size;
	}

	static void ClearRegisterType(HazeStack* stack, const InstructionData& oper)
	{
		//New和Ret寄存器的type清空，防止没有垃圾回收掉
		if (oper.AddressType == InstructionAddressType::Register)
		{
			auto Register = stack->GetVirtualRegister(GetVirtualRegisterByDesc(oper.Desc));
			if (/*Register == stack->GetVirtualRegister(NEW_REGISTER) || */Register == stack->GetVirtualRegister(HazeVirtualRegister::RET))
			{
				Register->Type.Reset();
			}
		}
	}
};

void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args)
{
	if (funcData->InstructionNum > 0)
	{
		InstructionProcessor::CallHazeFunction(stack, funcData, (int)funcData->Params.size(), args);
	}
}

void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData)
{
	return InstructionProcessor::GetOperatorAddress(stack, insData);
}

// 指令处理器函数表定义
const A_Array<void(*)(HazeStack* stack), ((x_uint32)InstructionOpCode::LINE + 1)> g_InstructionProcessor =
{
#define HAZE_OP_CODE_DEFINE(OP_CODE) &InstructionProcessor::OP_CODE,
	#include "HazeOpCodeTemplate"
#undef HAZE_OP_CODE_DEFINE
};