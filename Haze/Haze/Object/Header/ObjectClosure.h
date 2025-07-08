#pragma once
#include "GCObject.h"

class HazeStack;

struct ClosureRefVariable
{
	HazeVariableType Type;
	void* Object;
};

/*
* 基本类型的值保留
*/

class ObjectClosure : public GCObject
{
	friend class HazeMemory;
public:
	ObjectClosure(x_uint32 gcIndex, const FunctionData* functionData, const FunctionData* refFunction, char* refStackESP);

	~ObjectClosure();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();

	//static void NewObjectArray(HAZE_STD_CALL_PARAM);

private:
	static void CallFunction(HAZE_OBJECT_CALL_PARAM);

private:
	x_uint32 m_DataGCIndex;

	ClosureRefVariable* m_Data;
	const FunctionData* m_FunctionData;
};
