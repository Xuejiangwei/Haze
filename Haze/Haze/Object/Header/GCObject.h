#pragma once

class GCObject
{
public:
	GCObject(x_uint32 gcIndex) : m_GCIndex(gcIndex) {}

	x_uint32 m_GCIndex;
};

#define CHECK_GET_STACK_OBJECT(OBJ, TYPE_STR) GET_PARAM(OBJ); \
	if (!OBJ) \
	{ \
		OBJECT_ERR_W(TYPE_STR "对象为空"); \
		return; \
	}

#define OBJ_TYPE_DEF(TYPE) HazeVariableType(HazeValueType::TYPE, HAZE_TYPE_ID(HazeValueType::TYPE))