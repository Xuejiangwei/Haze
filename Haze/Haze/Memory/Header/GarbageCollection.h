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

private:
	void MarkClassMember(std::vector<std::pair<uint64, HazeValueType>>& Vector_MarkAddressBase,
		std::vector<std::pair<uint64, ClassData*>>& Vector_MarkAddressClass, const HazeDefineVariable& Var, int Offset);
	
	void MarkArrayIndex(std::vector<uint64>& Array, uint64 Index);


private:
	HazeVM* VM;
};