#include "HazePch.h"
#include "ObjectHash.h"
#include "Compiler.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "HazeLibraryDefine.h"
#include "ObjectBase.h"
#include "HazeMemory.h"

#define FACTOR			0.5
#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "��ϣ")

template<typename T>
x_uint64 __ObjectHash_Hash__(T* v)
{
	return *v;
}

template<>
x_uint64 __ObjectHash_Hash__(x_float32* v)
{
	return *((x_uint32*)v);
}

template<>
x_uint64 __ObjectHash_Hash__(x_float64* v)
{
	return *((x_uint64*)v);
}

template<>
x_uint64 __ObjectHash_Hash__(void* v)
{
	return (x_uint64)v;
}

template<>
x_uint64 __ObjectHash_Hash__(ObjectString* v)
{
	return 0;
}

template<>
x_uint64 __ObjectHash_Hash__(ObjectClass* v)
{
	return 0;
}

template<>
x_uint64 __ObjectHash_Hash__(ObjectBase* v)
{
	return 0;
}

//template<>
//x_uint64 __ObjectHash_Hash__(ObjectBase* v)
//{
//	return 0;
//}


ObjectHash::ObjectHash(x_uint32 gcIndex, const TemplateDefineTypes& defineTypes)
	: GCObject(gcIndex)
{
	if (defineTypes.Types.size() == 2)
	{
		m_KeyType = new TemplateDefineType(defineTypes.Types[0]);
		m_ValueType = new TemplateDefineType(defineTypes.Types[1]);
	}

	Rehash();
}

ObjectHash::~ObjectHash()
{
	delete m_KeyType;
	delete m_ValueType;

	HazeMemory::GetMemory()->Remove(m_Data, m_Capacity * sizeof(ObjectHashNode), m_DataGCIndex);
}

AdvanceClassInfo* ObjectHash::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Add(H_TEXT("����"), { &ObjectHash::GetLength, HazeValueType::UInt64, {} });
	info.Add(H_TEXT("���"), { &ObjectHash::Add, HazeValueType::Void, { HazeValueType::MultiVariable } });
	info.Add(H_TEXT("�Ƴ�"), { &ObjectHash::Remove, HazeValueType::Void, { HazeValueType::MultiVariable } });

	info.Add(HAZE_ADVANCE_GET_FUNCTION, { &ObjectHash::Get, HazeValueType::Void, { HazeValueType::MultiVariable } });
	info.Add(HAZE_ADVANCE_SET_FUNCTION, { &ObjectHash::Set, HazeValueType::Void, { HazeValueType::MultiVariable } });

	return &info;
}

HazeDefineType ObjectHash::GetKeyBaseType()
{
	return m_KeyType->Type->BaseType;
}

HazeDefineType ObjectHash::GetValueBaseType()
{
	return m_ValueType->Type->BaseType;
}

ObjectHashNode* ObjectHash::GetFreeNode()
{
	while (m_LastFreeNode > m_Data)
	{
		m_LastFreeNode--;
		if (m_LastFreeNode->IsNone())
		{
			return m_LastFreeNode;
		}
	}
	return nullptr;
}

void ObjectHash::Rehash()
{
	x_uint64 newCapacity = 1;
	x_uint64 doubleLength = (m_Length + 1) * 2;
	while (newCapacity < doubleLength)
	{
		newCapacity = newCapacity << 1;
	}

	auto pair = HazeMemory::GetMemory()->AllocaGCData(newCapacity * sizeof(ObjectHashNode), GC_ObjectType::HashData);
	m_DataGCIndex = pair.second;

	auto oldData = m_Data;
	m_Data = (ObjectHashNode*)pair.first;

	m_Capacity = newCapacity;
	m_LastFreeNode = m_Data + newCapacity;

	auto length = m_Length;
	m_Length = 0;
	for (x_uint64 i = 0; i < length; i++)
	{
		if (oldData + i)
		{
			Add(oldData[i].Key, oldData[i].Value, nullptr);
		}
	}
}

void ObjectHash::GetLength(HAZE_OBJECT_CALL_PARAM)
{
	ObjectHash* obj;

	GET_PARAM_START();
	GET_OBJ(obj);

	SET_RET_BY_TYPE(HazeValueType::UInt64, obj->m_Length);
}

void ObjectHash::Get(HAZE_OBJECT_CALL_PARAM)
{
	ObjectHash* obj;
	char* key;

	GET_PARAM_START();
	GET_OBJ(obj);

	auto size = obj->GetKeyBaseType().GetTypeSize();
	GET_PARAM_ADDRESS(key, size);

	x_uint64 hashValue = GetHash(obj, key, stack);
	auto node = &obj->m_Data[hashValue];

	HazeValue tempKey;
	SetHazeValueByData(tempKey, obj->GetKeyBaseType().PrimaryType, key);

	HazeValue value;
	while (true)
	{
		if (IsEqualByType(obj->GetKeyBaseType().PrimaryType, node->Key, tempKey))
		{
			value = node->Value;
			break;
		}

		node += node->Next;

		if (node->IsNone())
		{
			break;
		}
	}

	SET_RET_BY_TYPE(obj->GetValueBaseType().PrimaryType, value);
	//HAZE_LOG_INFO("Array Get <%d>\n", offset);
}

void ObjectHash::Set(HAZE_OBJECT_CALL_PARAM)
{
	Add(HAZE_STD_CALL_PARAM_VAR);
}

x_uint64 ObjectHash::GetHash(ObjectHash* obj, void* value, HazeStack* stack)
{
	x_uint64 hashValue = 0;
	switch (obj->GetValueBaseType().PrimaryType)
	{
		case HazeValueType::Bool:
			hashValue = __ObjectHash_Hash__((bool*)value);
			break;
		case HazeValueType::Int8:
			hashValue = __ObjectHash_Hash__((x_int8*)value);
			break;
		case HazeValueType::Int16:
			hashValue = __ObjectHash_Hash__((x_int16*)value);
			break;
		case HazeValueType::Int32:
			hashValue = __ObjectHash_Hash__((x_int32*)value);
			break;
		case HazeValueType::Int64:
			hashValue = __ObjectHash_Hash__((x_int64*)value);
			break;
		case HazeValueType::UInt8:
			hashValue = __ObjectHash_Hash__((x_int8*)value);
			break;
		case HazeValueType::UInt16:
			hashValue = __ObjectHash_Hash__((x_uint16*)value);
			break;
		case HazeValueType::UInt32:
			hashValue = __ObjectHash_Hash__((x_uint32*)value);
			break;
		case HazeValueType::UInt64:
			hashValue = __ObjectHash_Hash__((x_uint64*)value);
			break;
		case HazeValueType::Float32:
			hashValue = __ObjectHash_Hash__((x_float32*)value);
			break;
		case HazeValueType::Float64:
			hashValue = __ObjectHash_Hash__((x_float64*)value);
			break;
		case HazeValueType::String:
			hashValue = __ObjectHash_Hash__((ObjectString*)value);
			break;
		case HazeValueType::Class:
			hashValue = __ObjectHash_Hash__((ObjectClass*)value);
			break;
		case HazeValueType::ObjectBase:
			hashValue = __ObjectHash_Hash__((ObjectBase*)value);
			break;
		case HazeValueType::Enum:
			hashValue = __ObjectHash_Hash__((x_int8*)value);
			break;
		case HazeValueType::Array:
			hashValue = __ObjectHash_Hash__(value);
			break;
		default:
			OBJECT_ERR_W("����<%s>���ܹ�ϣ", GetHazeValueTypeString(obj->GetValueBaseType().PrimaryType));
			break;
	}
	return hashValue % ((obj->m_Capacity - 1) | 1);
}

void ObjectHash::Add(HazeValue key, HazeValue value, HazeStack* stack)
{
	x_uint64 hashValue = GetHash(this, (void*)(&value), stack);

	auto node = &m_Data[hashValue];
	if (!node->IsNone())
	{
		auto freeNode = GetFreeNode();
		if (freeNode)
		{
			auto nodeHashValue = GetHash(this, &node->Key, stack);
			auto otherNode = &m_Data[nodeHashValue];
			if (otherNode != node)
			{
				while (otherNode + otherNode->Next != node)
				{
					otherNode += otherNode->Next;
				}

				otherNode->Next = freeNode - otherNode;
				*freeNode = *node;

				if (node->Next != 0)
				{
					freeNode->Next += node - freeNode;
					node->Next = 0;
				}

				node->Value.Value.Pointer = 0;
			}
			else
			{
				if (node->Next != 0)
				{
					freeNode->Next = node + node->Next - freeNode;
				}
				else
				{
					assert(freeNode->Next == 0);
				}

				node->Next = freeNode - node;
				node = freeNode;
			}
		}
		else
		{
			Rehash();
			return Add(key, value, stack);
		}
	}


	node->Key = key;
	node->Value = value;
	m_Length++;
}

void ObjectHash::Add(HAZE_OBJECT_CALL_PARAM)
{
	ObjectHash* obj;
	char* key = nullptr;
	char* value = nullptr;

	GET_PARAM_START();
	GET_OBJ(obj);

	GET_PARAM_ADDRESS(key, obj->GetKeyBaseType().GetTypeSize());
	GET_PARAM_ADDRESS(value, obj->GetValueBaseType().GetTypeSize());

	HazeValue tempKey;
	HazeValue tempValue;
	SetHazeValueByData(tempKey, obj->GetKeyBaseType().PrimaryType, key);
	SetHazeValueByData(tempValue, obj->GetValueBaseType().PrimaryType, value);
	obj->Add(tempKey, tempValue, stack);
}

void ObjectHash::Remove(HAZE_OBJECT_CALL_PARAM)
{
	ObjectHash* obj;
	char* value = nullptr;

	GET_PARAM_START();
	GET_PARAM(obj);
	if (!obj)
	{
		auto& var = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - 2].Operator[0];
		OBJECT_ERR_W("��ϣ����<%s>Ϊ��", var.Variable.Name.c_str());
		return;
	}

	x_uint64 newCapacity = 1;
	x_uint64 doubleLength = (obj->m_Length - 1) * 2;
	while (newCapacity < doubleLength)
	{
		newCapacity = newCapacity << 1;
	}

	if (newCapacity < obj->m_Capacity)
	{
		// ����Hash
		obj->Rehash();
	}
}
