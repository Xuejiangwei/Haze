#include "HazeMemory.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeLogDefine.h"
#include <chrono>

#define PAGE_BASE_SIZE  4 * 4

HazeMemory::HazeMemory(uint64 maxMarkTime)
	: m_IsForceGC(false), m_MaxMarkTime(maxMarkTime), m_MarkStage(MarkStage::Ready)
{
}

HazeMemory::~HazeMemory()
{
	for (int i = 0; i < _countof(m_MemoryBlocks); i++)
	{
		auto block = m_MemoryBlocks[i];
		while (block)
		{
			auto nextBlock = block->GetNext();
			delete block;
			block = nextBlock;
		}
	}
}

HazeMemory* HazeMemory::GetMemory()
{
	static std::unique_ptr<HazeMemory> s_Memory = std::make_unique<HazeMemory>();
	return s_Memory.get();
}

void* HazeMemory::Alloca(uint64 size)
{
	void* ret = nullptr;
	size = RoundUp(size);
	
	if (size <= MAX_HAZE_ALLOC_SIZE)
	{
		uint32 offset = size % GRANULE == 0 ? -1 : 1;
		uint32 index = (uint32)size / GRANULE + offset;
		
		auto memoryIns = GetMemory();
		if (!memoryIns->m_FreeList[index])
		{
			memoryIns->m_FreeList[index] = std::make_unique<MemoryFreeList>();
		}
		
		auto freeList = memoryIns->m_FreeList[index].get();

		if (freeList->HasMemory())
		{
			ret = freeList->Pop();
		}
		else
		{
			index = (uint32)size / PAGE_UNIT;
			
			MemoryBlock* block = nullptr;
			if (memoryIns->m_MemoryBlocks[index])
			{
				block = memoryIns->m_MemoryBlocks[index];
				MemoryBlock* prevBlock = nullptr;
				while (block && block->IsUsed())
				{
					prevBlock = block;
					block = block->GetNext();
				}
				
				if (block)
				{
					uint32 length = sizeof(block->m_Memory) / block->m_BlockInfo.UnitSize;
					char* address = block->m_Memory;
					while (length-- > 0)
					{
						freeList->Push(address);
						address += block->m_BlockInfo.UnitSize;
					}
				}
				else
				{
					block = new MemoryBlock((uint32)size);
					prevBlock->SetNext(block);
				}
			}
			else
			{
				block = new MemoryBlock((uint32)size);
				memoryIns->m_MemoryBlocks[index] = block;
			}

			uint32 length = sizeof(block->m_Memory) / block->m_BlockInfo.UnitSize;
			char* address = block->m_Memory;
			while (length-- > 0)
			{
				freeList->Push(address);
				address += block->m_BlockInfo.UnitSize;
			}

			ret = freeList->Pop();
		}
	}
	else
	{
		ret = malloc(size);
		GetMemory()->m_BigMemorys[ret] = { GC_State::Black, ret };
	}

	return ret;
}

void HazeMemory::AddToRoot(void*)
{
}

void HazeMemory::MarkVariable(const HazeDefineType& type, uint64 startAddress, char* classAddress)
{
	switch (type.PrimaryType)
	{
	case HazeValueType::PointerBase:
		m_MarkAddressBases.push_back({ { startAddress, type.SecondaryType }, GC_State::Gray });
		break;
	case HazeValueType::PointerClass:
		m_MarkAddressClasses.push_back({ { startAddress, m_VM->FindClass(type.CustomName) }, GC_State::Gray });
		break;
	case HazeValueType::Class:
		MarkClassMember(m_VM->FindClass(type.CustomName), classAddress);
		break;
	case HazeValueType::ArrayBase:
	{
		uint64 size = m_VM->GetRegisterArrayLength(startAddress) * GetSizeByHazeType(type.SecondaryType);

#ifdef _DEBUG
		if (size == 0)
		{
			GC_ERR_W("�����������鳤��Ϊ0");
			return;
		}
#endif // _DEBUG

		m_MarkAddressArrays.push_back({ { startAddress, size }, GC_State::Gray });
	}
		break;
	case HazeValueType::ArrayClass:
	{
		auto classData = m_VM->FindClass(type.CustomName);
		for (uint64 i = 0; i < m_VM->GetRegisterArrayLength(startAddress); i++)
		{
			MarkClassMember(classData, (char*)startAddress + classData->Size * i);
		}

		uint64 size = m_VM->GetRegisterArrayLength(startAddress) * classData->Size;

#ifdef _DEBUG
		if (size == 0)
		{
			GC_ERR_W("�����������鳤��Ϊ0");
			return;
		}
#endif // _DEBUG

		m_MarkAddressArrays.push_back({ { startAddress, size }, GC_State::Gray });
	}
	break;
	case HazeValueType::ArrayPointer:
	{
		uint64 address;
		if (IsHazeDefaultTypeAndVoid(type.SecondaryType))
		{
			for (uint64 i = 0; i < m_VM->GetRegisterArrayLength(startAddress); i++)
			{
				memcpy(&address, (char*)startAddress + sizeof(char*) * i, sizeof(char*));
				m_MarkAddressBases.push_back({ { address, type.SecondaryType }, GC_State::Gray });
			}
		}
		else if (IsClassType(type.SecondaryType))
		{
			auto classData = m_VM->FindClass(type.CustomName);
			for (uint64 i = 0; i < m_VM->GetRegisterArrayLength(startAddress); i++)
			{
				memcpy(&address, (char*)startAddress + sizeof(char*) * i, sizeof(char*));
				m_MarkAddressClasses.push_back({ {address, m_VM->FindClass(type.CustomName) }, GC_State::Gray });
			}
		}
		else
		{
			GC_ERR_W("ָ������ָ����������");
			return;
		}

		uint64 size = m_VM->GetRegisterArrayLength(startAddress) * sizeof(char*);

#ifdef _DEBUG
		if (size == 0)
		{
			GC_ERR_W("�����������鳤��Ϊ0");
			return;
		}
#endif // _DEBUG

		m_MarkAddressArrays.push_back({ { startAddress, size }, GC_State::Gray });
	}
	break;
	default:
		break;
	}
}

void HazeMemory::MarkClassMember(ClassData* classData, char* baseAddress)
{
	uint64 address = 0;
	for (size_t i = 0; i < classData->Members.size(); i++)
	{
		auto& member = classData->Members[i];
		memcpy(&address, baseAddress + member.Offset, sizeof(address));
		MarkVariable(member.Variable.Type, address, baseAddress + member.Offset);
	}
}

void HazeMemory::Mark()
{
	auto memoryIns = HazeMemory::GetMemory();

	if (m_MarkStage == MarkStage::Ready)
	{
		m_MarkStage = MarkStage::Running_MarkRoot;
		m_CurrMarkBaseIndex = 0;
		m_CurrMarkClassIndex = 0;
		m_CurrMarkArrayIndex = 0;

		m_MarkStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();

		//��������ɫ����Ϊ��ɫ
		for (auto& iter : memoryIns->m_MemoryBlocks)
		{
			auto block = iter;
			while (block && block->IsUsed())
			{
				block->SetAllWhite();
				block = block->GetNext();
			}
		}

		//���ڵ��ڴ��� ��̬������ջ���Ĵ�����
		uint64 Address = 0;

		for (auto& it : m_VM->Vector_GlobalData)
		{
			MarkVariable(it.m_Type, (uint64)it.Value.Value.Pointer, (char*)it.Value.Value.Pointer);
		}

		//��Ϊֻ�ں�����������Retָ��ʱ��GC������ֻ����Ret����Ĵ��������õ����
		auto retRegister = m_VM->VMStack->GetVirtualRegister(RET_REGISTER);
		if (retRegister && retRegister->Data.size() > 0)
		{
			memcpy(&Address, retRegister->Data.begin()._Unwrapped(), sizeof(Address));
			MarkVariable(retRegister->Type, Address, retRegister->Data.begin()._Unwrapped());
		}

		for (size_t i = 0; i < m_VM->VMStack->m_StackFrame.size(); i++)
		{
			for (auto& var : m_VM->VMStack->m_StackFrame[i].FunctionInfo->Variables)
			{
				memcpy(&Address, &m_VM->VMStack->m_StackMain[m_VM->VMStack->m_StackFrame[i].EBP + var.Offset], sizeof(Address));
				MarkVariable(var.Variable.Type, Address, 
					&m_VM->VMStack->m_StackMain[m_VM->VMStack->m_StackFrame[i].EBP + var.Offset]);
			}
		}
	}
	else
	{
		m_MarkStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}

	m_MarkStage = MarkStage::Running_MarkList;

	//��������ڵ���ٱ��� Vector_MarkAddress
	while (m_CurrMarkBaseIndex < m_MarkAddressBases.size())
	{
		MarkArrayBaseIndex();
		m_CurrMarkBaseIndex++;
	}

	while (m_CurrMarkClassIndex < m_MarkAddressClasses.size())
	{
		//m_MarkAddressClassesû�н�����ȱ�����������Ҫ������ȱ���
		MarkArrayClassIndex();
		m_CurrMarkClassIndex++;
	}

	while (m_CurrMarkArrayIndex < m_MarkAddressArrays.size())
	{
		MarkArrayArrayIndex();
		m_CurrMarkArrayIndex++;
	}

	m_KeepMemorys.clear();
	for (size_t i = 0; i < m_MarkAddressBases.size(); i++)
	{
		m_KeepMemorys.push_back((void*)m_MarkAddressBases[i].first.first);
	}

	for (size_t i = 0; i < m_MarkAddressClasses.size(); i++)
	{
		m_KeepMemorys.push_back((void*)m_MarkAddressClasses[i].first.first);
	}

	for (size_t i = 0; i < m_MarkAddressArrays.size(); i++)
	{
		m_KeepMemorys.push_back((void*)m_MarkAddressArrays[i].first.first);
	}
}

void HazeMemory::Sweep()
{
	auto memoryIns = HazeMemory::GetMemory();

	//��Ҫ�������freelist,����ʱ�ټ���
	for (int i = 0; i < _countof(memoryIns->m_FreeList); i++)
	{
		if (memoryIns->m_FreeList[i])
		{
			memoryIns->m_FreeList[i]->Clear();
		}
	}

	//���Ϊ��ɫ
	for (auto& iter : memoryIns->m_MemoryBlocks)
	{
		auto block = iter;
		while (block && block->IsUsed())
		{
			for (size_t i = 0; i < m_KeepMemorys.size(); i++)
			{
				if (m_KeepMemorys[i] && block->IsInBlock(m_KeepMemorys[i]))
				{
					block->MarkBlack(m_KeepMemorys[i]);
					m_KeepMemorys[i] = nullptr;
				}
			}

			block = block->GetNext();
		}
	}

	//������һ��յ�freelist
	for (auto& iter : memoryIns->m_MemoryBlocks)
	{
		auto block = iter;
		while (block && block->IsUsed())
		{
			uint32 unitSize = block->m_BlockInfo.UnitSize;
			uint32 offset = unitSize % GRANULE == 0 ? -1 : 1;
			uint32 index = unitSize / GRANULE + offset;

			uint32 unitCount = block->m_BlockInfo.MarkCount;
			for (size_t i = 0; i < unitCount; i++)
			{
				if (block->m_BlockInfo.Mark[i] == (uint8)GC_State::White)
				{
					if (memoryIns->m_FreeList[index])
					{
						memoryIns->m_FreeList[index]->Push(&block->m_Memory[unitSize * i]);
					}
					else
					{
						memoryIns->m_FreeList[index] = std::make_unique<MemoryFreeList>();
						memoryIns->m_FreeList[index]->Push(&block->m_Memory[unitSize * i]);
					}
				}
			}

			block = block->GetNext();
		}
	}

	//���±�����飬��ʱ����Ҫ����Ϊ�ǿɴ��Եı����������ɴ������ڴ棬�������ݼ�¼�л���������
	for (auto& i : m_MarkAddressArrays)
	{
		i.first.second = m_VM->Vector_ArrayCache[i.first.first];
	}

	m_VM->Vector_ArrayCache.clear();
	for (auto& i : m_MarkAddressArrays)
	{
		m_VM->Vector_ArrayCache[i.first.first] = i.first.second;
	}
}

void HazeMemory::ForceGC()
{
	m_IsForceGC = true;
	Mark();
	Sweep();

	m_IsForceGC = false;
}

inline bool HazeMemory::MarkArrayBaseIndex()
{
	if (m_MarkStage != MarkStage::Running_MarkList)
	{
		return true;
	}

	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	if (time.count() - m_MarkStartTimestamp > m_MaxMarkTime)
	{
		m_MarkStartTimestamp = time.count();
	}

	return m_MarkStartTimestamp = time.count();
}

inline bool HazeMemory::MarkArrayClassIndex()
{
	if (m_MarkStage != MarkStage::Running_MarkList)
	{
		return true;
	}

	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	if (time.count() - m_MarkStartTimestamp > m_MaxMarkTime)
	{
		m_MarkStartTimestamp = time.count();
	}

	auto markClass = m_MarkAddressClasses[m_CurrMarkClassIndex++];
	MarkClassMember(markClass.first.second, (char*)markClass.first.first);
	markClass.second = GC_State::Black;

	return m_MarkStartTimestamp = time.count();
}

inline bool HazeMemory::MarkArrayArrayIndex()
{
	if (m_MarkStage != MarkStage::Running_MarkList)
	{
		return true;
	}

	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	if (time.count() - m_MarkStartTimestamp > m_MaxMarkTime)
	{
		m_MarkStartTimestamp = time.count();
	}

	return m_MarkStartTimestamp = time.count();
}
