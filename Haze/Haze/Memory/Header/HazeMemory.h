#pragma once

#include "Haze.h"

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

struct GC_Array
{
};

//�����ڴ�ʱ��block�е�freelist�����룬��û�ж����ڴ棬�ٴ�page������һ��block,��page��û�У���Ӳ���ϵͳ����page
//���һ��Page�������page��С��һ����ͬ

class HazeMemory
{
public:
	HazeMemory();

	~HazeMemory();

	static HazeMemory* GetMemory();

	static void* Alloca(uint64 size);

	void SetVM(HazeVM* vm) { VM = vm; }

	void AddToRoot(void*);

	void Mark();

	void Sweep();

	void ForceGC();

private:
	void MarkClassMember(std::vector<std::pair<uint64, HazeValueType>>& Vector_MarkAddressBase,
		std::vector<std::pair<uint64, ClassData*>>& Vector_MarkAddressClass, const HazeDefineType& VarType, char* BaseAddress);

	void MarkArrayBaseIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index);

	void MarkArrayClassIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index);

private:
	HazeVM* VM;
	std::vector<void*> Vector_KeepMemory;
	
	std::unique_ptr<MemoryFreeList> m_FreeList[MAX_HAZE_ALLOC_SIZE / GRANULE];
	MemoryBlock* m_MemoryBlock[PAGE_NUM];
	std::unordered_map<void*, std::pair<GC_State, void*>> HashMap_BigMemory;
};
