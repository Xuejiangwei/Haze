#pragma once

//https://juejin.cn/post/6966954993869914119
//https://juejin.cn/post/6968400262629163038

//https://zhuanlan.zhihu.com/p/41023320
//https://zhuanlan.zhihu.com/p/41398507

#include "GCObjectList.h"

#define MAX_HAZE_ALLOC_SIZE 2048
#define GRANULE 16
#define PAGE_UNIT 4096
#define PAGE_NUM 60

class HazeVM;
class MemoryFreeList;
class MemoryBlock;
class GCObjectList;

/*   �����ڴ�ʱ��block�е�freelist�����룬��û�ж����ڴ棬�ٴ�page������һ��block, ��page��û�У���Ӳ���ϵͳ����page
     ���һ��Page�������page��С��һ����ͬ
*/
class HazeMemory
{
public:
	HazeMemory(x_uint64 maxMarkTime = 500);

	~HazeMemory();

	static HazeMemory* GetMemory();

	static Pair<void*, x_uint32> AllocaGCData(x_uint64 size, GC_ObjectType type);

	void SetVM(HazeVM* vm) { m_VM = vm; }

	void AddToRoot(void*);

	void Remove(void* data, x_uint64 memorySize, x_uint32 gcIndex);

	void Mark();

	void Sweep();

	void TryGC(bool forceGC = false);

	enum class MarkStage : x_uint8
	{
		Ready,
		Mark,
		MarkEnd,
		Sweep,
	};

private:
	static void* Alloca(x_uint64 size);

	void ForceGC();

	void MarkVariable(const HazeVariableType& type, const void* address);

	void MarkClassMember(ClassData* classData, const char* baseAddress);

private:
	HazeVM* m_VM;
	bool m_IsForceGC;
	MarkStage m_MarkStage;
	x_uint64 m_MarkStartTimestamp;
	x_uint64 m_MaxMarkTime;			//����

	Unique<MemoryFreeList> m_FreeList[MAX_HAZE_ALLOC_SIZE / GRANULE];
	MemoryBlock* m_MemoryBlocks[PAGE_NUM];
	HashMap<void*, Pair<GC_State, void*>> m_BigMemorys;

	x_uint64 m_LastGCTime;
	x_uint64 m_CurrMarkBaseIndex;
	x_uint64 m_CurrMarkClassIndex;
	x_uint64 m_CurrMarkArrayIndex;

	/*V_Array<void*> m_KeepMemorys;
	V_Array<Pair<x_uint64, GC_State>> m_MarkAddressBases;*/

	Unique<GCObjectList> m_ObjectList;
};
