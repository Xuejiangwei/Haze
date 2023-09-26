#include "HazeMemory.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"
#include "HazeVM.h"
#include "HazeStack.h"

#define PAGE_BASE_SIZE  4 * 4

HazeMemory::HazeMemory()
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

void HazeMemory::MarkClassMember(std::vector<std::pair<uint64, HazeValueType>>& markAddressBases,
	std::vector<std::pair<uint64, ClassData*>>& markAddressClasses, const HazeDefineType& varType, char* baseAddress)
{
	uint64 address = 0;
	auto classDatas = m_VM->FindClass(varType.CustomName);
	for (size_t i = 0; i < classDatas->Members.size(); i++)
	{
		auto& member = classDatas->Members[i];
		if (member.Variable.Type.PrimaryType == HazeValueType::PointerBase)
		{
			memcpy(&address, baseAddress + member.Offset, sizeof(address));
			markAddressBases.push_back({ address, member.Variable.Type.SecondaryType });
		}
		else if (member.Variable.Type.PrimaryType == HazeValueType::PointerClass)
		{
			memcpy(&address, baseAddress + member.Offset, sizeof(address));
			markAddressClasses.push_back({ address, m_VM->FindClass(member.Variable.Type.CustomName) });
		}
		else if (member.Variable.Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(markAddressBases, markAddressClasses, member.Variable.Type,
				baseAddress + member.Offset);
		}
	}
}

void HazeMemory::Mark()
{
	auto memoryIns = HazeMemory::GetMemory();

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
	std::vector<std::pair<uint64, HazeValueType>> markAddressBases;
	std::vector<std::pair<uint64, ClassData*>> markAddressClasses;
	uint64 Address = 0;

	for (auto& it : m_VM->Vector_GlobalData)
	{
		if (it.m_Type.PrimaryType == HazeValueType::PointerBase)
		{
			markAddressBases.push_back({ (uint64)it.Value.Value.Pointer, it.m_Type.SecondaryType });
		}
		else if (it.m_Type.PrimaryType == HazeValueType::PointerClass)
		{
			markAddressClasses.push_back({ (uint64)it.Value.Value.Pointer, m_VM->FindClass(it.m_Type.CustomName) });
		}
		else if (it.m_Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(markAddressBases, markAddressClasses, it.m_Type, (char*)&it.Value);
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
				markAddressBases.push_back({ Address, it.second.Type.SecondaryType });
			}
			else if (it.second.Type.PrimaryType == HazeValueType::PointerClass)
			{
				memcpy(&Address, it.second.Data.begin()._Unwrapped(), sizeof(Address));
				markAddressClasses.push_back({ Address, m_VM->FindClass(it.second.Type.CustomName) });
			}
			else if (it.second.Type.PrimaryType == HazeValueType::Class)
			{
				MarkClassMember(markAddressBases, markAddressClasses, it.second.Type, it.second.Data.begin()._Unwrapped());
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
				markAddressBases.push_back({ Address, var.Variable.Type.SecondaryType });
			}
			else if (var.Variable.Type.PrimaryType == HazeValueType::PointerClass)
			{
				memcpy(&Address, &m_VM->VMStack->m_StackMain[m_VM->VMStack->m_StackFrame[i].EBP + var.Offset], sizeof(Address));
				markAddressClasses.push_back({ Address, m_VM->FindClass(var.Variable.Type.CustomName) });
			}
			else if (var.Variable.Type.PrimaryType == HazeValueType::Class)
			{
				MarkClassMember(markAddressBases, markAddressClasses, var.Variable.Type,
					&m_VM->VMStack->m_StackMain[m_VM->VMStack->m_StackFrame[i].EBP + var.Offset]);
			}
		}
	}

	//遍历完根节点后，再遍历 Vector_MarkAddress
	uint64 baseIndex = 0;
	uint64 classIndex = 0;
	while (baseIndex < markAddressBases.size() || classIndex < markAddressClasses.size())
	{
		if (baseIndex < markAddressBases.size())
		{
			MarkArrayBaseIndex(markAddressBases, markAddressClasses, baseIndex++);
		}

		if (classIndex < markAddressClasses.size())
		{
			MarkArrayClassIndex(markAddressBases, markAddressClasses, classIndex++);
		}
	}

	m_KeepMemorys.clear();
	for (size_t i = 0; i < markAddressBases.size(); i++)
	{
		m_KeepMemorys.push_back((void*)markAddressBases[i].first);
	}

	for (size_t i = 0; i < markAddressClasses.size(); i++)
	{
		m_KeepMemorys.push_back((void*)markAddressClasses[i].first);
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
}

void HazeMemory::ForceGC()
{
	Mark();
	Sweep();
}

void HazeMemory::MarkArrayBaseIndex(std::vector<std::pair<uint64, HazeValueType>>& arrayBase, 
	std::vector<std::pair<uint64, ClassData*>>& arrayClass, uint64 index)
{
}

void HazeMemory::MarkArrayClassIndex(std::vector<std::pair<uint64, HazeValueType>>& arrayBase,
	std::vector<std::pair<uint64, ClassData*>>& arrayClass, uint64 index)
{
}