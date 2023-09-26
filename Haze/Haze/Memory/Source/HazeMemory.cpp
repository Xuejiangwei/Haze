#include "HazeMemory.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"
#include "HazeVM.h"
#include "HazeStack.h"

#define PAGE_BASE_SIZE  4 * 4


static GC_Array GC_Arrays;

HazeMemory::HazeMemory()
{
}

HazeMemory::~HazeMemory()
{
	for (int i = 0; i < _countof(m_MemoryBlock); i++)
	{
		auto block = m_MemoryBlock[i];
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
	static std::unique_ptr<HazeMemory> Memory = std::make_unique<HazeMemory>();
	return Memory.get();
}

void* HazeMemory::Alloca(uint64 size)
{
	void* Ret = nullptr;
	size = RoundUp(size);
	
	if (size <= MAX_HAZE_ALLOC_SIZE)
	{
		uint32 offset = size % GRANULE == 0 ? -1 : 1;
		uint32 index = size / GRANULE + offset;
		
		auto memoryIns = GetMemory();
		if (!memoryIns->m_FreeList[index])
		{
			memoryIns->m_FreeList[index] = std::make_unique<MemoryFreeList>();
		}
		
		auto freeList = memoryIns->m_FreeList[index].get();

		if (freeList->HasMemory())
		{
			Ret = freeList->Pop();
		}
		else
		{
			index = size / PAGE_UNIT;
			
			void* mallocStart = nullptr;
			void* mallocEnd = nullptr;
			MemoryBlock* block = nullptr;
			if (memoryIns->m_MemoryBlock[index])
			{
				block = memoryIns->m_MemoryBlock[index];
				MemoryBlock* prevBlock = nullptr;
				while (block && block->IsUsed())
				{
					prevBlock = block;
					block = block->GetNext();
				}
				
				if (block)
				{
					uint32 Length = sizeof(block->m_Memory) / block->BlockInfo.UnitSize;
					char* Address = block->m_Memory;
					while (Length-- > 0)
					{
						freeList->Push(Address);
						Address += block->BlockInfo.UnitSize;
					}
				}
				else
				{
					block = new MemoryBlock(size);
					prevBlock->SetNext(block);
				}
			}
			else
			{
				block = new MemoryBlock(size);
				memoryIns->m_MemoryBlock[index] = block;
			}

			uint32 Length = sizeof(block->m_Memory) / block->BlockInfo.UnitSize;
			char* Address = block->m_Memory;
			while (Length-- > 0)
			{
				freeList->Push(Address);
				Address += block->BlockInfo.UnitSize;
			}

			Ret = freeList->Pop();
		}
	}
	else
	{
		Ret = malloc(size);
		GetMemory()->HashMap_BigMemory[Ret] = { GC_State::Black, Ret };
	}

	return Ret;
}

void HazeMemory::AddToRoot(void*)
{
}

void HazeMemory::MarkClassMember(std::vector<std::pair<uint64, HazeValueType>>& Vector_MarkAddressBase,
	std::vector<std::pair<uint64, ClassData*>>& Vector_MarkAddressClass, const HazeDefineType& VarType, char* BaseAddress)
{
	uint64 Address = 0;
	auto m_ClassDatas = m_VM->FindClass(VarType.CustomName);
	for (size_t i = 0; i < m_ClassDatas->Members.size(); i++)
	{
		auto& Member = m_ClassDatas->Members[i];
		if (Member.Variable.m_Type.PrimaryType == HazeValueType::PointerBase)
		{
			memcpy(&Address, BaseAddress + Member.Offset, sizeof(Address));
			Vector_MarkAddressBase.push_back({ Address, Member.Variable.m_Type.SecondaryType });
		}
		else if (Member.Variable.m_Type.PrimaryType == HazeValueType::PointerClass)
		{
			memcpy(&Address, BaseAddress + Member.Offset, sizeof(Address));
			Vector_MarkAddressClass.push_back({ Address, m_VM->FindClass(Member.Variable.m_Type.CustomName) });
		}
		else if (Member.Variable.m_Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, Member.Variable.m_Type, BaseAddress + Member.Offset);
		}
	}
}

void HazeMemory::Mark()
{
	auto memoryIns = HazeMemory::GetMemory();

	//将所有颜色设置为白色
	for (auto& Iter : memoryIns->m_MemoryBlock)
	{
		auto block = Iter;
		while (block && block->IsUsed())
		{
			block->SetAllWhite();
			block = block->GetNext();
		}
	}

	//根节点内存有 静态变量、栈、寄存器等
	std::vector<std::pair<uint64, HazeValueType>> Vector_MarkAddressBase;
	std::vector<std::pair<uint64, ClassData*>> Vector_MarkAddressClass;
	uint64 Address = 0;

	for (auto& It : m_VM->Vector_GlobalData)
	{
		if (It.m_Type.PrimaryType == HazeValueType::PointerBase)
		{
			Vector_MarkAddressBase.push_back({ (uint64)It.Value.Value.Pointer, It.m_Type.SecondaryType });
		}
		else if (It.m_Type.PrimaryType == HazeValueType::PointerClass)
		{
			Vector_MarkAddressClass.push_back({ (uint64)It.Value.Value.Pointer, m_VM->FindClass(It.m_Type.CustomName) });
		}
		else if (It.m_Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, It.m_Type, (char*)&It.Value);
		}
	}

	auto NewRegister = m_VM->VMStack->GetVirtualRegister(NEW_REGISTER);
	auto RetRegister = m_VM->VMStack->GetVirtualRegister(RET_REGISTER);
	for (auto& It : m_VM->VMStack->HashMap_VirtualRegister)
	{
		if (&It.second == NewRegister || &It.second == RetRegister)
		{
			//New和Ret寄存器中存留的内存在赋值后不需要保留，考虑在赋值字节码执行中清除
			if (It.second.m_Type.PrimaryType == HazeValueType::PointerBase)
			{
				memcpy(&Address, It.second.m_Data.begin()._Unwrapped(), sizeof(Address));
				Vector_MarkAddressBase.push_back({ Address, It.second.m_Type.SecondaryType });
			}
			else if (It.second.m_Type.PrimaryType == HazeValueType::PointerClass)
			{
				memcpy(&Address, It.second.m_Data.begin()._Unwrapped(), sizeof(Address));
				Vector_MarkAddressClass.push_back({ Address, m_VM->FindClass(It.second.m_Type.CustomName) });
			}
			else if (It.second.m_Type.PrimaryType == HazeValueType::Class)
			{
				MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, It.second.m_Type, It.second.m_Data.begin()._Unwrapped());
			}
		}
	}

	for (size_t i = 0; i < m_VM->VMStack->Stack_Frame.size(); i++)
	{
		for (auto& Var : m_VM->VMStack->Stack_Frame[i].FunctionInfo->Variables)
		{
			if (Var.Variable.m_Type.PrimaryType == HazeValueType::PointerBase)
			{
				memcpy(&Address, &m_VM->VMStack->Stack_Main[m_VM->VMStack->Stack_Frame[i].EBP + Var.Offset], sizeof(Address));
				Vector_MarkAddressBase.push_back({ Address, Var.Variable.m_Type.SecondaryType });
			}
			else if (Var.Variable.m_Type.PrimaryType == HazeValueType::PointerClass)
			{
				memcpy(&Address, &m_VM->VMStack->Stack_Main[m_VM->VMStack->Stack_Frame[i].EBP + Var.Offset], sizeof(Address));
				Vector_MarkAddressClass.push_back({ Address, m_VM->FindClass(Var.Variable.m_Type.CustomName) });
			}
			else if (Var.Variable.m_Type.PrimaryType == HazeValueType::Class)
			{
				MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, Var.Variable.m_Type, &m_VM->VMStack->Stack_Main[m_VM->VMStack->Stack_Frame[i].EBP + Var.Offset]);
			}
		}
	}

	//遍历完根节点后，再遍历 Vector_MarkAddress
	uint64 BaseIndex = 0;
	uint64 ClassIndex = 0;
	while (BaseIndex < Vector_MarkAddressBase.size() || ClassIndex < Vector_MarkAddressClass.size())
	{
		if (BaseIndex < Vector_MarkAddressBase.size())
		{
			MarkArrayBaseIndex(Vector_MarkAddressBase, Vector_MarkAddressClass, BaseIndex++);
		}

		if (ClassIndex < Vector_MarkAddressClass.size())
		{
			MarkArrayClassIndex(Vector_MarkAddressBase, Vector_MarkAddressClass, ClassIndex++);
		}
	}

	Vector_KeepMemory.clear();
	for (size_t i = 0; i < Vector_MarkAddressBase.size(); i++)
	{
		Vector_KeepMemory.push_back((void*)Vector_MarkAddressBase[i].first);
	}

	for (size_t i = 0; i < Vector_MarkAddressClass.size(); i++)
	{
		Vector_KeepMemory.push_back((void*)Vector_MarkAddressClass[i].first);
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

	std::vector<void*> KeepMemory;

	for (auto& Iter : memoryIns->m_MemoryBlock)
	{
		auto block = Iter;
		while (block && block->IsUsed())
		{
			for (size_t i = 0; i < Vector_KeepMemory.size(); i++)
			{
				if (Vector_KeepMemory[i] && block->IsInBlock(Vector_KeepMemory[i]))
				{
					block->MarkBlack(Vector_KeepMemory[i]);
					Vector_KeepMemory[i] = nullptr;
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

void HazeMemory::MarkArrayBaseIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index)
{
}

void HazeMemory::MarkArrayClassIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index)
{
}