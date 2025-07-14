#pragma once
#include "GCObject.h"

class HazeStack;
class HazeVM;

struct ObjectHashNode
{
	HazeValue Key;
	HazeValue Value;
	x_uint64 Next;

	bool IsNone() const
	{
		return Key.Value.UInt64 == 0 && Value.Value.UInt64 == 0 && Next == 0;
	}
};

class ObjectHash : public GCObject
{
	friend class HazeMemory;
public:
	ObjectHash(x_uint32 gcIndex, HazeVM* vm, x_uint32 typeId);

	~ObjectHash();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	//static void NewObjectArray(HAZE_STD_CALL_PARAM);

	HazeVariableType GetKeyBaseType();
	HazeVariableType GetValueBaseType();

private:
	static void GetLength(HAZE_OBJECT_CALL_PARAM);

	static void Add(HAZE_OBJECT_CALL_PARAM);

	static void Remove(HAZE_OBJECT_CALL_PARAM);

	static void Get(HAZE_OBJECT_CALL_PARAM);

	static void Set(HAZE_OBJECT_CALL_PARAM);

private:
	static x_uint64 GetHash(ObjectHash* obj, void* value, HazeStack* stack);

	void Add(HazeValue key, HazeValue value, HazeStack* stack);

private:
	ObjectHashNode* GetFreeNode();

	void Rehash();

private:
	x_uint32 m_DataGCIndex;

	ObjectHashNode* m_Data;
	ObjectHashNode* m_LastFreeNode;
	x_uint64 m_Length;
	x_uint64 m_Capacity;
	HazeVariableType m_KeyType;
	HazeVariableType m_ValueType;
	/*HazeValueType m_KeyType;
	HazeValueType m_ValueType;
	ClassData* m_KeyClassInfo;
	ClassData* m_ValueClassInfo;*/
};
