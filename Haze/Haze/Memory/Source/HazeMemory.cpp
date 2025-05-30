#include "HazePch.h"
#include "HazeMemory.h"
#include "GCObjectList.h"
#include "MemoryBlock.h"
#include "MemoryHelper.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeLogDefine.h"
#include "ObjectArray.h"
#include "ObjectClass.h"
#include "ObjectString.h"
#include "ObjectDynamicClass.h"
#include <chrono>

#define PAGE_BASE_SIZE	4 * 4
#define GC_TIME			50
#define ENABLE_GC_LOG	0

#if ENABLE_GC_LOG
#define ENABLE_MEMORY_LOG	1
	static x_uint64 s_TotalAllocMemory = 0;

	struct GCTimer
	{
		void Start()
		{
			auto now = std::chrono::system_clock::now();

			StartMemory = s_TotalAllocMemory;
			StartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

			auto milli = StartTimestamp % 1000;
			char buffer[32];
			std::time_t t = std::chrono::system_clock::to_time_t(now);
			std::tm now_tm;
			localtime_s(&now_tm,&t);
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);

			HAZE_LOG_INFO_W("开始GC<%s.%d>, 当前申请内存<%d>\n", String2WString(buffer).c_str(), milli, StartMemory);
		}

		void End()
		{
			auto now = std::chrono::system_clock::now();
			auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			auto milli = endTime % 1000;
			char buffer[32];
			std::time_t t = std::chrono::system_clock::to_time_t(now);
			std::tm now_tm;
			localtime_s(&now_tm, &t);
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);

			HAZE_LOG_INFO_W("结束GC<%s.%d>, 当前申请内存<%d>, 用时<%d>毫秒, 回收前<%d>个字节, 回收<%d>个字节\n", String2WString(buffer).c_str(), milli, s_TotalAllocMemory,
				endTime - StartTimestamp, StartMemory, StartMemory - s_TotalAllocMemory);
		}

		static void OnAlloc(x_uint64 size, void* address)
		{
			s_TotalAllocMemory += size;
#if ENABLE_MEMORY_LOG
			HAZE_LOG_INFO_W("申请内存<%d>, 当前申请内存<%d> 地址<%p>\n", size, s_TotalAllocMemory, address);
#endif // ENABLE_MEMORY_LOG
		}

		static void OnFree(x_uint64 size, void* address)
		{
			s_TotalAllocMemory -= size;
#if ENABLE_MEMORY_LOG
			HAZE_LOG_INFO_W("回收内存<%d>, 当前申请内存<%d> 地址<%p>\n", size, s_TotalAllocMemory, address);
#endif // ENABLE_MEMORY_LOG
		}

	private:
		x_uint64 StartTimestamp;
		x_uint64 StartMemory;
	};

	static GCTimer s_GCTimer;

#endif // ENABLE_GC_LOG


HazeMemory::HazeMemory(x_uint64 maxMarkTime)
	: m_IsForceGC(false), m_MaxMarkTime(maxMarkTime), m_MarkStage(MarkStage::Ready), m_LastGCTime(0)
{
	m_ObjectList = MakeUnique<GCObjectList>(100);
	//m_LastGCTime = std::chrono::seconds(std::time(NULL)).count();
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

Pair<void*, x_uint32> HazeMemory::AllocaGCData(x_uint64 size, GC_ObjectType type)
{
	auto memoryIns = GetMemory();
	auto address = memoryIns->Alloca(size);
	auto index = memoryIns->m_ObjectList->Add(address, type);
	return  { address, index };
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

#if ENABLE_GC_LOG
	s_GCTimer.OnAlloc(size, ret);
#endif // ENABLE_GC_LOG

	return ret;
}

void HazeMemory::AddToRoot(void*)
{
}

void HazeMemory::Remove(void* data, x_uint64 memorySize, x_uint32 gcIndex)
{
	if (data)
	{
		m_ObjectList->Remove(gcIndex);
		memorySize = RoundUp(memorySize);
		if (memorySize > 0)
		{
			if (memorySize <= MAX_HAZE_ALLOC_SIZE)
			{
				x_uint32 offset = memorySize % GRANULE == 0 ? -1 : 1;
				x_uint32 index = (x_uint32)memorySize / GRANULE + offset;
				m_FreeList[index]->Push(data);
			}
			else
			{
				auto it = m_BigMemorys.find(data);
				if (it != m_BigMemorys.end())
				{
					free(data);
					m_BigMemorys.erase(it);
				}
				else
				{
					GC_ERR_W("回收内存错误, 大内存未能找到");
					return;
				}
			}
		}

#if ENABLE_GC_LOG
		s_GCTimer.OnFree(memorySize, data);
#endif // ENABLE_GC_LOG
	}
}

//需要考虑到循环引用的引起的死循环情况, 要么在Object中添加标记位（例如ObjectClass中添加uint8类型成员去做标记位），要么hashSet存储已标记的object
void HazeMemory::MarkVariable(const HazeDefineType& type, const void* address)
{
	switch (type.PrimaryType)
	{
		case HazeValueType::Class:
		{
			if ((ObjectClass*)address)
			{
				m_ObjectList->MarkObjectBlack(((ObjectClass*)address)->m_GCIndex);
				MarkClassMember(m_VM->FindClass(*type.CustomName), (char*)((ObjectClass*)address)->m_Data);
			}
		}
			break;
		case HazeValueType::DynamicClass:
		{
			if ((ObjectDynamicClass*)address)
			{
				m_ObjectList->MarkObjectBlack(((ObjectDynamicClass*)address)->m_GCIndex);
			}
		}
			break;
		case HazeValueType::Array:
		{
			if ((ObjectArray*)address)
			{
				m_ObjectList->MarkObjectBlack(((ObjectArray*)address)->m_GCIndex);

				auto objectArray = ((ObjectArray*)address);
				x_uint64 length = objectArray->m_Length;
				if (IsArrayType(objectArray->m_ValueType))
				{
					for (x_uint64 i = 0; i < length; i++)
					{
						auto v = *((ObjectArray**)objectArray->m_Data + i);
						MarkVariable(v->m_ValueType, v);

						if (!IsArrayType(v->m_ValueType))
						{
							m_ObjectList->MarkObjectBlack(v->m_GCIndex);
						}
					}
				}
				else if (IsClassType(objectArray->m_ValueType))
				{
					for (x_uint64 i = 0; i < length; i++)
					{
						auto v = (ObjectClass*)objectArray->m_Data + i;
						MarkVariable({ HazeValueType::Class, &v->m_ClassInfo->Name }, v);
					}
				}
				else if (IsDynamicClassType(objectArray->m_ValueType))
				{
					for (x_uint64 i = 0; i < length; i++)
					{
						auto v = (ObjectDynamicClass*)objectArray->m_Data + i;
						MarkVariable(HazeValueType::DynamicClass, v);
					}
				}
				else if (IsStringType(objectArray->m_ValueType))
				{
					for (x_uint64 i = 0; i < length; i++)
					{
						auto v = (ObjectString*)objectArray->m_Data + i;
						MarkVariable(objectArray->m_ValueType, v);
					}
				}

				x_uint64 size = ((ObjectArray*)address)->m_Length  * GetSizeByHazeType(type.SecondaryType);

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
			if ((ObjectString*)address)
			{
				m_ObjectList->MarkObjectBlack(((ObjectString*)address)->m_GCIndex);
			}
		}
		break;
	default:
		break;
	}
}

void HazeMemory::MarkClassMember(ClassData* classData, const char* baseAddress)
{
	void* address = 0;
	for (size_t i = 0; i < classData->Members.size(); i++)
	{
		auto& member = classData->Members[i];
		memcpy(&address, baseAddress + member.Offset, sizeof(address));
		MarkVariable(member.Variable.Type, address);
	}
}

void HazeMemory::Mark()
{
	if (m_MarkStage == MarkStage::Ready)
	{
		m_MarkStage = MarkStage::Mark;
		m_CurrMarkBaseIndex = 0;
		m_CurrMarkClassIndex = 0;
		m_CurrMarkArrayIndex = 0;

		m_MarkStartTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

#if ENABLE_GC_LOG
		s_GCTimer.Start();
#endif // ENABLE_GC_LOG

		//将所有颜色设置为白色
		m_ObjectList->MarkAllWhite();

		//根节点内存有 静态变量、栈、函数调用栈中缓存的寄存器、当前的寄存器
		void* address = 0;

		for (auto& it : m_VM->m_GlobalData)
		{
			MarkVariable(it.m_Type, it.Value.Value.Pointer);
		}

		for (auto& it : m_VM->m_StringTable)
		{
			m_ObjectList->MarkObjectBlack(((ObjectString*)it)->m_GCIndex);
		}

		for (auto& it : m_VM->m_ExtreGlobalData)
		{
			if (it)
			{
				m_ObjectList->MarkObjectBlack(it->m_GCIndex);
				MarkClassMember(it->m_ClassInfo, (char*)it);
			}
		}

		//因为只在函数结束调用Ret指令时会GC，所以只存在Ret虚拟寄存器有引用的情况
		auto retRegister = m_VM->m_Stack->GetVirtualRegister(RET_REGISTER);
		if (retRegister && retRegister->Data.size() > 0)
		{
			memcpy(&address, retRegister->Data.begin()._Unwrapped(), sizeof(address));
			MarkVariable(retRegister->Type, address);
		}

		for (size_t i = 0; i < m_VM->m_Stack->m_StackFrame.size(); i++)
		{
			for (auto& var : m_VM->m_Stack->m_StackFrame[i].FunctionInfo->Variables)
			{
				memcpy(&address, &m_VM->m_Stack->m_StackMain[m_VM->m_Stack->m_StackFrame[i].CurrParamESP + var.Offset], sizeof(address));
				MarkVariable(var.Variable.Type, address);
			}

			for (auto& var : m_VM->m_Stack->m_StackFrame[i].FunctionInfo->TempRegisters)
			{
				memcpy(&address, &m_VM->m_Stack->m_StackMain[m_VM->m_Stack->m_StackFrame[i].CurrParamESP + var.Offset], sizeof(address));
				MarkVariable(var.Type, address);
			}
		}

		m_MarkStage = MarkStage::MarkEnd;
	}
}

void HazeMemory::Sweep()
{
	if (m_MarkStage != MarkStage::MarkEnd)
	{
		return;
	}
	m_MarkStage = MarkStage::Sweep;

	//需要清空所有freelist,回收时再加入
	/*for (int i = 0; i < _countof(memoryIns->m_FreeList); i++)
	{
		if (memoryIns->m_FreeList[i])
		{
			memoryIns->m_FreeList[i]->Clear();
		}
	}*/

	//标记为黑色
	/*for (auto& iter : memoryIns->m_MemoryBlocks)
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
	}*/

	//清除并且回收到freelist
	for (x_uint64 i = 0; i < m_ObjectList->m_StateList.size(); i++)
	{
		if (m_ObjectList->m_StateList[i].IsUse && m_ObjectList->m_StateList[i].State == GC_State::White)
		{
			GCObject* gcObject = (GCObject*)m_ObjectList->m_ObjectList[i];
			x_uint64 memorySize = 0;
			switch (m_ObjectList->m_StateList[i].Type)
			{
				case GC_ObjectType::Class:
					memorySize = sizeof(ObjectClass);
					((ObjectClass*)gcObject)->~ObjectClass();
					break;
				case GC_ObjectType::String:
					memorySize = sizeof(ObjectString);
					((ObjectString*)m_ObjectList->m_ObjectList[i])->~ObjectString();
					break;
				case GC_ObjectType::Array:
					memorySize = sizeof(ObjectArray);
					((ObjectArray*)m_ObjectList->m_ObjectList[i])->~ObjectArray();
					break;
				case GC_ObjectType::DynamicClass:
					memorySize = sizeof(ObjectDynamicClass);
					((ObjectDynamicClass*)m_ObjectList->m_ObjectList[i])->~ObjectDynamicClass();
					break;
				default:
					break;
			}

			if (memorySize > 0)
			{
				Remove(gcObject, memorySize, gcObject->m_GCIndex);
			}
		}
	}
	
	/*for (auto& iter : memoryIns->m_MemoryBlocks)
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
	}*/


#if ENABLE_GC_LOG
	s_GCTimer.End();
#endif // ENABLE_GC_LOG

	m_MarkStage = MarkStage::Ready;
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
	m_IsForceGC = true;
	Mark();
	Sweep();

	m_IsForceGC = false;
	m_LastGCTime = std::chrono::seconds(std::time(NULL)).count();
}

