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

enum class GC_State : uint8
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
	HazeMemory(uint64 maxMarkTime = 500);

	~HazeMemory();

	static HazeMemory* GetMemory();

	static void* Alloca(uint64 size);

	void SetVM(HazeVM* vm) { m_VM = vm; }

	void AddToRoot(void*);

	void Mark();

	void Sweep();

	void ForceGC();

	enum class MarkStage
	{
		Ready,
		Running_MarkRoot,
		Running_MarkList
	};

private:
	void MarkVariable(const HazeDefineType& type, uint64 startAddress, char* classAddress);

	void MarkClassMember(ClassData* classData, char* baseAddress);

	inline bool MarkArrayBaseIndex();

	inline bool MarkArrayClassIndex();

	inline bool MarkArrayArrayIndex();

private:
	HazeVM* m_VM;
	bool m_IsForceGC;
	MarkStage m_MarkStage;
	uint64 m_MarkStartTimestamp;
	uint64 m_MaxMarkTime;		//毫秒

	Unique<MemoryFreeList> m_FreeList[MAX_HAZE_ALLOC_SIZE / GRANULE];
	MemoryBlock* m_MemoryBlocks[PAGE_NUM];
	HashMap<void*, Pair<GC_State, void*>> m_BigMemorys;

	uint64 m_CurrMarkBaseIndex;
	uint64 m_CurrMarkClassIndex;
	uint64 m_CurrMarkArrayIndex;

	V_Array<void*> m_KeepMemorys;
	V_Array<Pair<Pair<uint64, HazeValueType>, GC_State>> m_MarkAddressBases;
	V_Array<Pair<Pair<uint64, ClassData*>, GC_State>> m_MarkAddressClasses;
	V_Array<Pair<void*, GC_State>> m_MarkAddressArrays;
};
