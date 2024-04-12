#pragma once

#include "HazeHeader.h"

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

//�����ڴ�ʱ��block�е�freelist�����룬��û�ж����ڴ棬�ٴ�page������һ��block,��page��û�У���Ӳ���ϵͳ����page
//���һ��Page�������page��С��һ����ͬ

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
	uint64 m_MaxMarkTime;		//����

	std::unique_ptr<MemoryFreeList> m_FreeList[MAX_HAZE_ALLOC_SIZE / GRANULE];
	MemoryBlock* m_MemoryBlocks[PAGE_NUM];
	std::unordered_map<void*, std::pair<GC_State, void*>> m_BigMemorys;

	uint64 m_CurrMarkBaseIndex;
	uint64 m_CurrMarkClassIndex;
	uint64 m_CurrMarkArrayIndex;

	std::vector<void*> m_KeepMemorys;
	std::vector<std::pair<std::pair<uint64, HazeValueType>, GC_State>> m_MarkAddressBases;
	std::vector<std::pair<std::pair<uint64, ClassData*>, GC_State>> m_MarkAddressClasses;
	std::vector<std::pair<std::pair<uint64, uint64>, GC_State>> m_MarkAddressArrays;
};
