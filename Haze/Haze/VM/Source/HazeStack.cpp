#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeInstruction.h"
#include "HazeMemory.h"

#include "HazeDebugger.h"

extern std::unordered_map<InstructionOpCode, void (*)(HazeStack* Stack)> g_InstructionProcessor;
extern std::unique_ptr<HazeDebugger> g_Debugger;

HazeStack::HazeStack(HazeVM* vm) 
	: m_VM(vm), m_PC(0), m_EBP(0), m_ESP(0)
{
	m_StackMain.resize(HAZE_VM_STACK_SIZE);
	InitStackRegister();
}

HazeStack::~HazeStack()
{
}

void HazeStack::RunGlobalDataInit(uint32 startPC, uint32 endPC)
{
	int pc = m_PC;

	bool isConstructor = endPC - startPC > 1;
	for (m_PC = startPC; true; m_PC++)
	{
		auto iter = g_InstructionProcessor.find(m_VM->Instructions[m_PC].InsCode);
		if (iter != g_InstructionProcessor.end())
		{
			iter->second(this);
		}
		else
		{
			HAZE_LOG_ERR_W("<��ʼ��ȫ�ֱ�������>��"  HAZE_TEXT("!\n"));
			return;
		}

		if (m_PC == (int)endPC - 1)
		{
			if (isConstructor && !m_VM->IsDebug())
			{
				isConstructor = false;
			}
			else
			{
				break;
			}

		}

	}

	m_PC = pc;
}

void HazeStack::JmpTo(const InstructionData& data)
{
	auto& function = m_StackFrame.back().FunctionInfo;
	int address = function->FunctionDescData.InstructionStartAddress + data.Extra.Jmp.StartAddress;
	memcpy(&m_PC, &address, sizeof(m_PC));

	m_PC--;
}

void HazeStack::Run(bool isHazeCall)
{
	while (m_PC < m_VM->Instructions.size())
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

		auto iter = g_InstructionProcessor.find(m_VM->Instructions[m_PC].InsCode);
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

HazeRegister* HazeStack::GetVirtualRegister(const HAZE_CHAR* name)
{
	auto iter = m_VirtualRegister.find(name);
	if (iter != m_VirtualRegister.end())
	{
		return &iter->second;
	}
	return nullptr;
}

void HazeStack::InitStackRegister()
{
	m_VirtualRegister =
	{
		//{ADD_REGISTER, nullptr},
		//{SUB_REGISTER, nullptr},
		//{MUL_REGISTER, nullptr},
		//{DIV_REGISTER, nullptr},
		{RET_REGISTER, HazeRegister()},
		{NEW_REGISTER, HazeRegister()},
		{CMP_REGISTER, HazeRegister()},

		{TEMP_REGISTER_0, HazeRegister()},
		{TEMP_REGISTER_1, HazeRegister()},
		{TEMP_REGISTER_2, HazeRegister()},
		{TEMP_REGISTER_3, HazeRegister()},
		{TEMP_REGISTER_4, HazeRegister()},
		{TEMP_REGISTER_5, HazeRegister()},
		{TEMP_REGISTER_6, HazeRegister()},
		{TEMP_REGISTER_7, HazeRegister()},
		{TEMP_REGISTER_8, HazeRegister()},
		{TEMP_REGISTER_9, HazeRegister()},
	};
}

void HazeStack::OnCall(const FunctionData* info, int paramSize)
{
	RegisterData registerDara({ GetVirtualRegister(CMP_REGISTER)->Data });
	m_StackFrame.push_back(HazeStackFrame(info, paramSize, m_EBP, m_ESP - (HAZE_ADDRESS_SIZE + paramSize), registerDara));

	m_EBP = m_ESP;
	if (info->Variables.size() > info->Params.size())
	{
		m_ESP += info->Variables.back().Offset + GetSizeByType(info->Variables.back().Variable.Type, m_VM);
	}

	m_PC = info->FunctionDescData.InstructionStartAddress;
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

	GarbageCollection();
}

void HazeStack::ResetCallHaze()
{
	m_CallHazeStack.clear();
	m_CallHazeStack.push_back(1);

	Run(true);
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

void* HazeStack::Alloca(uint64 size)
{
	void* ret = HazeMemory::Alloca(size); //Pool->Alloca(Size);
	if (ret == nullptr)
	{
		HAZE_LOG_ERR_W("�ڴ����ʧ��!\n");
	}

	return ret;
}

void HazeStack::RegisterArray(uint64 address, uint64 length)
{
	m_VM->Vector_ArrayCache[address] = length;
}

uint64 HazeStack::GetRegisterArrayLength(uint64 address)
{
	return m_VM->GetRegisterArrayLength(address);
}

void HazeStack::GarbageCollection(bool force, bool collectionAll)
{
	//��ֹ����Ĵ����е����ñ�GC��ֻ�ں�������ʱ����GC�����Ҵ�ʱֻ��Ret����Ĵ������������á�

	if (force && collectionAll)
	{
		HazeMemory::GetMemory()->ForceGC();
	}
}