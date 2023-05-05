#pragma once

#include "Haze.h"

class HazeVM;

struct GC_Array
{

};

class GarbageCollection
{
public:
	GarbageCollection(HazeVM* VM);

	~GarbageCollection();

	void AddToRoot(void*);

	void Mark();

	void Sweep();

	void ForceGC();

private:
	void MarkClassMember(std::vector<std::pair<uint64, HazeValueType>>& Vector_MarkAddressBase,
		std::vector<std::pair<uint64, ClassData*>>& Vector_MarkAddressClass, const HazeDefineType& VarType, int Offset);
	
	void MarkArrayBaseIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index);
	
	void MarkArrayClassIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index);

private:
	HazeVM* VM;

	std::vector<void*> Vector_KeepMemory;
};