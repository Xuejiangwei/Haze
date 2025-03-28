#pragma once

//https://juejin.cn/post/6966954993869914119
//https://juejin.cn/post/6968400262629163038

//https://zhuanlan.zhihu.com/p/41023320
//https://zhuanlan.zhihu.com/p/41398507

#define MAX_HAZE_ALLOC_SIZE 2048
#define GRANULE 16
#define PAGE_UNIT 4096
#define PAGE_NUM 60

class HazeVM;
class MemoryFreeList;
class MemoryBlock;

enum class GC_State : x_uint8
{
	White,			//Garbage
	Gray,			//UnCertain, no scan completed
	Black,			//Reserve
};

//申请内存时从block中的freelist中申请，若没有多余内存，再从page中申请一个block,若page中没有，则从操作系统申请page
//最后一个Page表，里面的page大小不一定相同

class HazeMemory
{
public:
	HazeMemory(x_uint64 maxMarkTime = 500);

	~HazeMemory();

	static HazeMemory* GetMemory();

	static void* Alloca(x_uint64 size);

	void SetVM(HazeVM* vm) { m_VM = vm; }

	void AddToRoot(void*);

	void Mark();

	void Sweep();

	void TryGC(bool forceGC = false);

	enum class MarkStage : x_uint8
	{
		Ready,
		Running_MarkRoot,
		Running_MarkList
	};

private:
	void ForceGC();

	void MarkVariable(const HazeDefineType& type, x_uint64 startAddress, char* classAddress);

	void MarkClassMember(ClassData* classData, char* baseAddress);

	inline bool MarkArrayBaseIndex();

private:
	HazeVM* m_VM;
	bool m_IsForceGC;
	MarkStage m_MarkStage;
	x_uint64 m_MarkStartTimestamp;
	x_uint64 m_MaxMarkTime;			//毫秒

	Unique<MemoryFreeList> m_FreeList[MAX_HAZE_ALLOC_SIZE / GRANULE];
	MemoryBlock* m_MemoryBlocks[PAGE_NUM];
	HashMap<void*, Pair<GC_State, void*>> m_BigMemorys;

	x_uint64 m_LastGCTime;
	x_uint64 m_CurrMarkBaseIndex;
	x_uint64 m_CurrMarkClassIndex;
	x_uint64 m_CurrMarkArrayIndex;

	V_Array<void*> m_KeepMemorys;
	V_Array<Pair<x_uint64, GC_State>> m_MarkAddressBases;
};
