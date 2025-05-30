#pragma once

enum class GC_State : x_uint8
{
	White,			//Garbage
	Gray,			//UnCertain, no scan completed
	Black,			//Reserve
};

enum class GC_ObjectType : x_uint8
{
	None,
	Array,
	String,
	Class,
	DynamicClass,

	ArrayData,
	StringData,
	ClassData,
};

class GCObjectList
{
	friend class HazeMemory;
public:
	struct ObjectCacheState
	{
		GC_State State : 2;
		GC_ObjectType Type : 5;
		bool IsUse : 1;

		ObjectCacheState()
		{
			State = GC_State::White;
			Type = GC_ObjectType::None;
			IsUse = false;
		}

		ObjectCacheState(GC_State state, GC_ObjectType type, bool isUse): State(state), Type(type), IsUse(isUse)
		{
		}
	};

	GCObjectList(x_uint64 initLength);

	~GCObjectList();

	x_uint32 Add(void* object, GC_ObjectType type);
	void Remove(x_uint32 index);

	void MarkAllWhite();
	void MarkObjectBlack(x_uint32 index);

private:
	V_Array<ObjectCacheState> m_StateList;
	V_Array<void*> m_ObjectList;
	V_Array<x_uint32> m_ReuseList;
};