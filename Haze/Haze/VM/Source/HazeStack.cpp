#include "HazePch.h"
#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeMemory.h"

#include "HazeDebugger.h"

extern Unique<HazeDebugger> g_Debugger;
extern const A_Array<void(*)(HazeStack* stack), ((x_uint32)InstructionOpCode::LINE + 1)> g_InstructionProcessor;

HazeStack::HazeStack(HazeVM* vm) 
	: m_VM(vm), m_PC(0), m_EBP(0), m_ESP(0)
{
	m_StackMain.resize(HAZE_VM_STACK_SIZE);
	m_CallHazeStack.clear();
	InitStackRegister();
}

HazeStack::~HazeStack()
{

}

void HazeStack::RunGlobalDataInit(x_int64 startPC, x_int64 endPC)
{
	auto pc = m_PC;

	//bool isConstructor = endPC - startPC > 1;
	for (m_PC = startPC; true; m_PC++)
	{
		g_InstructionProcessor[(x_uint32)m_VM->m_Instructions[m_PC].InsCode](this);

		if (m_PC == endPC - 1)
		{
			break;
		}
	}

	m_PC = pc;
}

void HazeStack::OnError()
{
	LogStack();
	m_CallHazeStack.clear();
}

void HazeStack::LogStack()
{
	for (x_int64 i = (x_int64)m_StackFrame.size() - 1; i >= 0; i--)
	{
		HAZE_LOG_ERR_W("Error<%s>\n", m_VM->GetFunctionNameByData(m_StackFrame[i].FunctionInfo)->c_str());
	}
}

void HazeStack::JmpTo(const InstructionData& data)
{
	auto& function = m_StackFrame.back().FunctionInfo;
	auto address = (decltype(m_PC))function->FunctionDescData.InstructionStartAddress + data.Extra.Jmp.StartAddress;
	memcpy(&m_PC, &address, sizeof(m_PC));

	m_PC--;
}

const HazeVariableType& HazeStack::GetTempRegisterType(x_uint64 index) const
{
	auto& backFrame = m_StackFrame.back();
	return backFrame.FunctionInfo->TempRegisters[index].Type;
}

void HazeStack::ResetTempRegisterTypeByDynamicClassUnknow(x_uint64 index, const HazeVariableType& type)
{
	auto& backFrame = m_StackFrame.back();
	const_cast<HazeVariableType&>(backFrame.FunctionInfo->TempRegisters[index].Type) = type;
}

void HazeStack::Run(bool isHazeCall)
{
	while (m_PC < (x_int64)m_VM->m_Instructions.size())
	{
		while (m_VM->IsDebug())
		{
			if (!g_Debugger)
			{
				continue;
			}
			else
			{
				if (g_Debugger->IsPause())
				{
					continue;
				}
				break;
			}
		}

		g_InstructionProcessor[(x_uint32)m_VM->m_Instructions[m_PC].InsCode](this);

		if (isHazeCall && m_CallHazeStack.size() == 0)
		{
			return;
		}

		PCStepInc();
	}
}

void HazeStack::PCStepInc()
{
	static x_uint64 pcRunTimes = 0;

	++m_PC;

	// 假设每条字节码运行平均花费1.5微秒, 暂时10秒检测一次, 则大约运行30万次字节码GC一次
	if (++pcRunTimes > (10 * 1000 * 1000  * 2 / 3))
	{
		HazeMemory::GetMemory()->TryGC();
		pcRunTimes = 0;
	}
}

void HazeStack::InitStackRegister()
{
	//m_VirtualRegister =
	//{
	//	{RET_REGISTER, HazeRegister()},
	//	//{NEW_REGISTER, HazeRegister()},
	//	{CMP_REGISTER, HazeRegister()},

	//	/*{TEMP_REGISTER_A, HazeRegister()},
	//	{TEMP_REGISTER_B, HazeRegister()},
	//	{TEMP_REGISTER_2, HazeRegister()},
	//	{TEMP_REGISTER_3, HazeRegister()},
	//	{TEMP_REGISTER_4, HazeRegister()},
	//	{TEMP_REGISTER_5, HazeRegister()},
	//	{TEMP_REGISTER_6, HazeRegister()},
	//	{TEMP_REGISTER_7, HazeRegister()},
	//	{TEMP_REGISTER_8, HazeRegister()},
	//	{TEMP_REGISTER_9, HazeRegister()},*/
	//};
}

void HazeStack::OnCall(const FunctionData* info, int paramSize)
{
	RegisterData registerDara({ GetVirtualRegister(HazeVirtualRegister::CMP)->Data });
	m_StackFrame.push_back(HazeStackFrame(info, paramSize, m_EBP, m_ESP - paramSize - HAZE_ADDRESS_SIZE, m_ESP, registerDara));

	m_EBP = m_ESP;
	if (info->TempRegisters.size() > 0)
	{
		m_ESP += info->TempRegisters.back().Offset + 8;
	}
	else if (info->Variables.size() > info->Params.size())
	{
		m_ESP += info->Variables.back().Offset + info->Variables.back().Variable.Type.GetTypeSize();
	}

	//在调用函数时，将调用的函数栈全部清零，就不用担心GC时脏数据转换成有效的指针
	memset(&m_StackMain[m_EBP], 0, m_ESP - m_EBP);

	m_PC = (decltype(m_PC))info->FunctionDescData.InstructionStartAddress;
	--m_PC;

	AddCallHazeTimes();

	if (g_Debugger)
	{
		m_VM->OnExecLine(m_StackFrame.back().FunctionInfo->FunctionDescData.StartLine);
	}
}

void HazeStack::OnRet()
{
	if (g_Debugger)
	{
		m_VM->OnExecLine(m_StackFrame.back().FunctionInfo->FunctionDescData.EndLine);
		while (g_Debugger->IsPause()) 
		{
		}
	}

	memcpy(&m_PC, &(m_StackMain[m_EBP - HAZE_ADDRESS_SIZE]), HAZE_ADDRESS_SIZE);
	m_EBP = m_StackFrame.back().EBP;
	m_ESP = m_StackFrame.back().ESP;

	memcpy(GetVirtualRegister(HazeVirtualRegister::CMP)->Data.begin()._Unwrapped(), m_StackFrame.back().Register.Cmp_RegisterData.begin()._Unwrapped(),
		m_StackFrame.back().Register.Cmp_RegisterData.size());

	m_StackFrame.pop_back();

	SubCallHazeTimes();
}

void HazeStack::ResetCallHaze()
{
	auto tempStack = m_CallHazeStack;
	m_CallHazeStack.clear();
	m_CallHazeStack.push_back(1);

	Run(true);

	if (tempStack.size() > 0)
	{
		tempStack.pop_back();
	}

	m_CallHazeStack = Move(tempStack);
}

void HazeStack::AddCallHazeTimes()
{
	if (m_CallHazeStack.size() > 0)
	{
		m_CallHazeStack.push_back(1);
	}
}

void HazeStack::SubCallHazeTimes()
{
	if (m_CallHazeStack.size() > 0)
	{
		m_CallHazeStack.pop_back();
	}
}

void HazeStack::PushGCTempRegister(void* address, const HazeVariableType* type)
{
	m_GCTempRegisters.push_back({ address, type });
}

bool HazeStack::PopGCTempRegister(void* address)
{
	if (m_GCTempRegisters.back().first == address)
	{
		m_GCTempRegisters.pop_back();
		return true;
	}
	return false;
}