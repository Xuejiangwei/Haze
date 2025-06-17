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
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 1].Operator[0]; \
		OBJECT_ERR_W(TYPE_STR "����<%s>Ϊ��", var.Variable.Name.c_str()); \
		return; \
	}