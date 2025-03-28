#include "HazePch.h"
#include "HazeMemory.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeLogDefine.h"
#include "ObjectArray.h"
#include "ObjectClass.h"
#include "ObjectString.h"
#include <chrono>

#define PAGE_BASE_SIZE	4 * 4
#define GC_TIME			60

HazeMemory::HazeMemory(x_uint64 maxMarkTime)
	: m_IsForceGC(false), m_MaxMarkTime(maxMarkTime), m_MarkStage(MarkStage::Ready), m_LastGCTime(0)
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
	static Unique<HazeMemory> s_Memory = MakeUnique<HazeMemory>();
	return s_Memory.get();
}

void* HazeMemory::Alloca(x_uint64 size)
{
	void* ret = nullptr;
	if (size == 0)
	{
		return ret;
	}
	
	size = RoundUp(size);
	
	if (size <= MAX_HAZE_ALLOC_SIZE)
	{
		x_uint32 offset = size % GRANULE == 0 ? -1 : 1;
		x_uint32 index = (x_uint32)size / GRANULE + offset;
		
		auto memoryIns = GetMemory();
		if (!memoryIns->m_FreeList[index])
		{
			memoryIns->m_FreeList[index] = MakeUnique<MemoryFreeList>();
		}
		
		auto freeList = memoryIns->m_FreeList[index].get();

		if (freeList->HasMemory())
		{
			ret = freeList->Pop();
		}
		else
		{
			index = (x_uint32)size / PAGE_UNIT;
			
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
					x_uint32 length = sizeof(block->m_Memory) / block->m_BlockInfo.UnitSize;
					char* address = block->m_Memory;
					while (length-- > 0)
					{
						freeList->Push(address);
						address += block->m_BlockInfo.UnitSize;
					}
				}
				else
				{
					block = new MemoryBlock((x_uint32)size);
					prevBlock->SetNext(block);
				}
			}
			else
			{
				block = new MemoryBlock((x_uint32)size);
				memoryIns->m_MemoryBlocks[index] = block;
			}

			x_uint32 length = sizeof(block->m_Memory) / block->m_BlockInfo.UnitSize;
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
		HAZE_LOG_ERR_W("申请内存太大，大小为<%d>\n", size);
		ret = malloc(size);
		GetMemory()->m_BigMemorys[ret] = { GC_State::Black, ret };
	}

	memset(ret, 0, size);
	return ret;
}

void HazeMemory::AddToRoot(void*)
{
}

//需要考虑到循环引用的引起的死循环情况, 要么在Object中添加标记位（例如ObjectClass中添加uint8类型成员去做标记位），要么hashSet存储已标记的object
void HazeMemory::MarkVariable(const HazeDefineType& type, x_uint64 startAddress, char* classAddress)
{
	switch (type.PrimaryType)
	{
	case HazeValueType::Class:
	{
		if ((ObjectClass*)startAddress)
		{
			m_MarkAddressBases.push_back({ startAddress, GC_State::Gray });
			MarkClassMember(m_VM->FindClass(*type.CustomName), classAddress);
		}
	}
		break;
	case HazeValueType::Array:
	{
		if ((ObjectArray*)startAddress)
		{
			m_MarkAddressBases.push_back({ startAddress, GC_State::Gray });

			auto objectArray = ((ObjectArray*)startAddress);
			x_uint64 length = objectArray->m_Length;
			if (IsArrayType(objectArray->m_ValueType))
			{
				for (x_uint64 i = 0; i < length; i++)
				{
					auto v = (ObjectArray*)objectArray->m_Data + i;
					MarkVariable(v->m_ValueType, (x_uint64)v, nullptr);
				}
			}
			else if (IsClassType(objectArray->m_ValueType))
			{
				for (x_uint64 i = 0; i < length; i++)
				{
					auto v = (ObjectClass*)objectArray->m_Data + i;
					MarkVariable({ HazeValueType::Class, &v->m_ClassInfo->Name }, (x_uint64)v, nullptr);
				}
			}
			else if (IsStringType(objectArray->m_ValueType))
			{
				for (x_uint64 i = 0; i < length; i++)
				{
					auto v = (ObjectString*)objectArray->m_Data + i;
					MarkVariable(objectArray->m_ValueType, (x_uint64)v, nullptr);
				}
			}

			x_uint64 size = ((ObjectArray*)startAddress)->m_Length  * GetSizeByHazeType(type.SecondaryType);

	#ifdef _DEBUG
			if (size == 0)
			{
				GC_ERR_W("基本类型数组长度为0");
				return;
			}
	#endif // _DEBUG
		}
	}
		break;
	case HazeValueType::String:
	{
		//之后的考虑不能这样判断指针的有效性
		if ((ObjectString*)startAddress)
		{
			m_MarkAddressBases.push_back({ startAddress, GC_State::Gray });
			m_MarkAddressBases.push_back({ (x_uint64)((ObjectString*)startAddress)->GetData(), GC_State::Gray });
		}
	}
		break;
	default:
		break;
	}
}

void HazeMemory::MarkClassMember(ClassData* classData, char* baseAddress)
{
	x_uint64 address = 0;
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

		//根节点内存有 静态变量、栈、函数调用栈中缓存的寄存器、当前的寄存器
		x_uint64 Address = 0;

		for (auto& it : m_VM->m_GlobalData)
		{
			MarkVariable(it.m_Type, (x_uint64)it.Value.Value.Pointer, (char*)it.Value.Value.Pointer);
		}

		for (auto& it : m_VM->m_ExtreGlobalData)
		{
			if (it)
			{
				m_MarkAddressBases.push_back({ (x_uint64)it, GC_State::Gray });
				MarkClassMember(it->m_ClassInfo, (char*)it);
			}
		}

		//因为只在函数结束调用Ret指令时会GC，所以只存在Ret虚拟寄存器有引用的情况
		auto retRegister = m_VM->m_Stack->GetVirtualRegister(RET_REGISTER);
		if (retRegister && retRegister->Data.size() > 0)
		{
			memcpy(&Address, retRegister->Data.begin()._Unwrapped(), sizeof(Address));
			MarkVariable(retRegister->Type, Address, retRegister->Data.begin()._Unwrapped());
		}

		for (size_t i = 0; i < m_VM->m_Stack->m_StackFrame.size(); i++)
		{
			for (auto& var : m_VM->m_Stack->m_StackFrame[i].FunctionInfo->Variables)
			{
				memcpy(&Address, &m_VM->m_Stack->m_StackMain[m_VM->m_Stack->m_StackFrame[i].EBP + var.Offset], sizeof(Address));
				MarkVariable(var.Variable.Type, Address, 
					&m_VM->m_Stack->m_StackMain[m_VM->m_Stack->m_StackFrame[i].EBP + var.Offset]);
			}

			for (auto& var : m_VM->m_Stack->m_StackFrame[i].FunctionInfo->TempRegisters)
			{
				memcpy(&Address, &m_VM->m_Stack->m_StackMain[m_VM->m_Stack->m_StackFrame[i].EBP + var.Offset], sizeof(Address));
				MarkVariable(var.Type, Address,
					&m_VM->m_Stack->m_StackMain[m_VM->m_Stack->m_StackFrame[i].EBP + var.Offset]);
			}
		}
	}
	else
	{
		m_MarkStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}

	m_MarkStage = MarkStage::Running_MarkList;

	m_KeepMemorys.clear();
	for (size_t i = 0; i < m_MarkAddressBases.size(); i++)
	{
		m_KeepMemorys.push_back((void*)m_MarkAddressBases[i].first);
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
			x_uint32 unitSize = block->m_BlockInfo.UnitSize;
			x_uint32 offset = unitSize % GRANULE == 0 ? -1 : 1;
			x_uint32 index = unitSize / GRANULE + offset;

			x_uint32 unitCount = block->m_BlockInfo.MarkCount;
			for (size_t i = 0; i < unitCount; i++)
			{
				if (block->m_BlockInfo.Mark[i] == (x_uint8)GC_State::White)
				{
					if (memoryIns->m_FreeList[index])
					{
						memoryIns->m_FreeList[index]->Push(&block->m_Memory[unitSize * i]);
					}
					else
					{
						memoryIns->m_FreeList[index] = MakeUnique<MemoryFreeList>();
						memoryIns->m_FreeList[index]->Push(&block->m_Memory[unitSize * i]);
					}
				}
			}

			block = block->GetNext();
		}
	}
}

void HazeMemory::TryGC(bool forceGC)
{
	if (forceGC)
	{
		ForceGC();
	}
	else
	{
		x_uint64 currTime = std::chrono::seconds(std::time(NULL)).count();
		if (currTime - m_LastGCTime > GC_TIME)
		{
			ForceGC();
		}
	}
}

void HazeMemory::ForceGC()
{
	m_LastGCTime = std::chrono::seconds(std::time(NULL)).count();
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


