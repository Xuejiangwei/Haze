#include "HazePch.h"
#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeMemory.h"

#include "HazeDebugger.h"

extern HashMap<InstructionOpCode, void (*)(HazeStack* Stack)> g_InstructionProcessor;
extern Unique<HazeDebugger> g_Debugger;

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

void HazeStack::RunGlobalDataInit(x_uint32 startPC, x_uint32 endPC)
{
	int pc = m_PC;

	//bool isConstructor = endPC - startPC > 1;
	for (m_PC = startPC; true; m_PC++)
	{
		auto iter = g_InstructionProcessor.find(m_VM->m_Instructions[m_PC].InsCode);
		if (iter != g_InstructionProcessor.end())
		{
			iter->second(this);
		}
		else
		{
			HAZE_LOG_ERR_W("<初始化全局变量错误>："  H_TEXT("!\n"));
			return;
		}

		if (m_PC == (int)endPC - 1)
		{
			break;
		}

	}

	m_PC = pc;
}

void HazeStack::JmpTo(const InstructionData& data)
{
	auto& function = m_StackFrame.back().FunctionInfo;
	int address = (decltype(m_PC))function->FunctionDescData.InstructionStartAddress + data.Extra.Jmp.StartAddress;
	memcpy(&m_PC, &address, sizeof(m_PC));

	m_PC--;
}

void HazeStack::Run(bool isHazeCall)
{
	while (m_PC < m_VM->m_Instructions.size())
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

		auto iter = g_InstructionProcessor.find(m_VM->m_Instructions[m_PC].InsCode);
		if (iter != g_InstructionProcessor.end())
		{
			iter->second(this);
		}
		else
		{
			return;
		}

		if (isHazeCall && m_CallHazeStack.size() == 0)
		{
			return;
		}

		PCStepInc();
	}
}

void HazeStack::PCStepInc()
{
	++m_PC;
}

void HazeStack::InitStackRegister()
{
	m_VirtualRegister =
	{
		{RET_REGISTER, HazeRegister()},
		//{NEW_REGISTER, HazeRegister()},
		{CMP_REGISTER, HazeRegister()},

		{TEMP_REGISTER_A, HazeRegister()},
		{TEMP_REGISTER_B, HazeRegister()},
		/*{TEMP_REGISTER_2, HazeRegister()},
		{TEMP_REGISTER_3, HazeRegister()},
		{TEMP_REGISTER_4, HazeRegister()},
		{TEMP_REGISTER_5, HazeRegister()},
		{TEMP_REGISTER_6, HazeRegister()},
		{TEMP_REGISTER_7, HazeRegister()},
		{TEMP_REGISTER_8, HazeRegister()},
		{TEMP_REGISTER_9, HazeRegister()},*/
	};
}

void HazeStack::OnCall(const FunctionData* info, int paramSize)
{
	RegisterData registerDara({ GetVirtualRegister(CMP_REGISTER)->Data });
	m_StackFrame.push_back(HazeStackFrame(info, paramSize, m_EBP, m_ESP - (HAZE_ADDRESS_SIZE + paramSize), registerDara));

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

	memcpy(GetVirtualRegister(CMP_REGISTER)->Data.begin()._Unwrapped(), m_StackFrame.back().Register.Cmp_RegisterData.begin()._Unwrapped(),
		m_StackFrame.back().Register.Cmp_RegisterData.size());

	m_StackFrame.pop_back();

	SubCallHazeTimes();

	//HazeMemory::GetMemory()->TryGC();
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

	m_CallHazeStack = tempStack;
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

void HazeStack::PushGCTempRegister(void* address, const HazeDefineType* type)
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
