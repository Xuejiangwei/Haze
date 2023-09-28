#include "HazeMemory.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"
#include "HazeVM.h"
#include "HazeStack.h"
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

void HazeMemory::MarkClassMember(ClassData* classData, char* baseAddress)
{
	uint64 address = 0;
	for (size_t i = 0; i < classData->Members.size(); i++)
	{
		auto& member = classData->Members[i];
		if (member.Variable.Type.PrimaryType == HazeValueType::PointerBase)
		{
			memcpy(&address, baseAddress + member.Offset, sizeof(address));
			m_MarkAddressBases.push_back({ { address, member.Variable.Type.SecondaryType }, GC_State::Gray });
		}
		else if (member.Variable.Type.PrimaryType == HazeValueType::PointerClass)
		{
			memcpy(&address, baseAddress + member.Offset, sizeof(address));
			m_MarkAddressClasses.push_back({{ address, m_VM->FindClass(member.Variable.Type.CustomName) }, GC_State::Gray });
		}
		else if (member.Variable.Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(m_VM->FindClass(member.Variable.Type.CustomName), baseAddress + member.Offset);
		}
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

		m_MarkStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();

		//将所有颜色设置为白色
		for (auto& iter : memoryIns->m_MemoryBlocks)
		{
			auto block = iter;
			while (block && block->IsUsed())
			{
				block->SetAllWhite();
				block = block->GetNext();
			}
		}

		//根节点内存有 静态变量、栈、寄存器等
		uint64 Address = 0;

		for (auto& it : m_VM->Vector_GlobalData)
		{
			if (it.m_Type.PrimaryType == HazeValueType::PointerBase)
			{
				m_MarkAddressBases.push_back({ { (uint64)it.Value.Value.Pointer, it.m_Type.SecondaryType }, GC_State::Gray });
			}
			else if (it.m_Type.PrimaryType == HazeValueType::PointerClass)
			{
				m_MarkAddressClasses.push_back({ { (uint64)it.Value.Value.Pointer, m_VM->FindClass(it.m_Type.CustomName) } 
				, GC_State::Gray });
			}
			else if (it.m_Type.PrimaryType == HazeValueType::Class)
			{
				MarkClassMember(m_VM->FindClass(it.m_Type.CustomName), (char*)&it.Value);
			}
		}

		auto newRegister = m_VM->VMStack->GetVirtualRegister(NEW_REGISTER);
		auto retRegister = m_VM->VMStack->GetVirtualRegister(RET_REGISTER);
		for (auto& it : m_VM->VMStack->m_VirtualRegister)
		{
			if (&it.second == newRegister || &it.second == retRegister)
			{
				//New和Ret寄存器中存留的内存在赋值后不需要保留，考虑在赋值字节码执行中清除
				if (it.second.Type.PrimaryType == HazeValueType::PointerBase)
				{
					memcpy(&Address, it.second.Data.begin()._Unwrapped(), sizeof(Address));
					m_MarkAddressBases.push_back({ { Address, it.second.Type.SecondaryType }, GC_State::Gray });
				}
				else if (it.second.Type.PrimaryType == HazeValueType::PointerClass)
				{
					memcpy(&Address, it.second.Data.begin()._Unwrapped(), sizeof(Address));
					m_MarkAddressClasses.push_back({ { Address, m_VM->FindClass(it.second.Type.CustomName) }, GC_State::Gray });
				}
				else if (it.second.Type.PrimaryType == HazeValueType::Class)
				{
					MarkClassMember(m_VM->FindClass(it.second.Type.CustomName), it.second.Data.begin()._Unwrapped());
				}
			}
		}

		for (size_t i = 0; i < m_VM->VMStack->m_StackFrame.size(); i++)
		{
			for (auto& var : m_VM->VMStack->m_StackFrame[i].FunctionInfo->Variables)
			{
				if (var.Variable.Type.PrimaryType == HazeValueType::PointerBase)
				{
					memcpy(&Address, &m_VM->VMStack->m_StackMain[m_VM->VMStack->m_StackFrame[i].EBP + var.Offset], sizeof(Address));
					m_MarkAddressBases.push_back({ { Address, var.Variable.Type.SecondaryType }, GC_State::Gray });
				}
				else if (var.Variable.Type.PrimaryType == HazeValueType::PointerClass)
				{
					memcpy(&Address, &m_VM->VMStack->m_StackMain[m_VM->VMStack->m_StackFrame[i].EBP + var.Offset], sizeof(Address));
					m_MarkAddressClasses.push_back({ { Address, m_VM->FindClass(var.Variable.Type.CustomName) }, GC_State::Gray });
				}
				else if (var.Variable.Type.PrimaryType == HazeValueType::Class)
				{
					MarkClassMember(m_VM->FindClass(var.Variable.Type.CustomName),
						&m_VM->VMStack->m_StackMain[m_VM->VMStack->m_StackFrame[i].EBP + var.Offset]);
				}
			}
		}
	}
	else
	{
		m_MarkStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}

	m_MarkStage = MarkStage::Running_MarkList;

	//遍历完根节点后，再遍历 Vector_MarkAddress
	while (m_CurrMarkBaseIndex < m_MarkAddressBases.size() || m_CurrMarkClassIndex < m_MarkAddressClasses.size())
	{
		if (m_CurrMarkBaseIndex < m_MarkAddressBases.size())
		{
			MarkArrayBaseIndex();
		}

		if (m_CurrMarkClassIndex < m_MarkAddressClasses.size())
		{
			MarkArrayClassIndex();
		}
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
}

void HazeMemory::Sweep()
{
	auto memoryIns = HazeMemory::GetMemory();

	//需要清空所有freelist,回收时再加入
	for (int i = 0; i < _countof(memoryIns->m_FreeList); i++)
	{
		if (memoryIns->m_FreeList[i])
		{
			memoryIns->m_FreeList[i]->Clear();
		}
	}

	//标记为黑色
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

	//清除并且回收到freelist
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
