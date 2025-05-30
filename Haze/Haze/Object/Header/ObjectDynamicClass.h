#pragma once
#include "GCObject.h"
#include "HazeInstruction.h"

class ObjectDynamicClass : public GCObject
{
	friend class HazeVM;
	friend class HazeMemory;
	friend class InstructionProcessor;
public:
	struct CustomMethods
	{
		//需要每个都设置
		void(*Constructor)(void*);
		void(*Deconstructor)(void*);
		void(*GetMember)(HazeStack* stack, const HString& name, void* dataPtr);
		void(*SetMember)(HazeStack* stack, const HString& name, void* dataPtr, x_uint8* currESP);
		void(*CallFunction)(HazeStack* stack, const HString& name, void* dataPtr, x_uint8* currESP);

		bool IsValid() const
		{
			return Constructor && Deconstructor && GetMember && SetMember && CallFunction;
		}
	};

public:
	ObjectDynamicClass(x_uint32 gcIndex, CustomMethods* methods, void* dataPtr);

	~ObjectDynamicClass();

	static struct AdvanceClassInfo* GetAdvanceClassInfo();
private:
	static void GetMember(HAZE_OBJECT_CALL_PARAM);

	static void SetMember(HAZE_OBJECT_CALL_PARAM);

	static void CallFunction(HAZE_OBJECT_CALL_PARAM);

private:
	void* m_Data;
	CustomMethods* m_Methods;
};
