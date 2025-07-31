#include "HazePch.h"
#include "ObjectHash.h"
#include "Compiler.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "HazeLibraryDefine.h"
#include "ObjectBase.h"
#include "ObjectString.h"
#include "HazeMemory.h"

#define DEFAULT_CAPACITY 8
#define FACTOR			0.5
#define GET_OBJ(OBJ) CHECK_GET_STACK_OBJECT(OBJ, "哈希")

template<typename T>
x_uint64 __ObjectHash_Hash__(T* v, HazeStack* stack)
{
	return *v;
}

template<>
x_uint64 __ObjectHash_Hash__(x_float32* v, HazeStack* stack)
{
	return *((x_uint32*)v);
}

template<>
x_uint64 __ObjectHash_Hash__(x_float64* v, HazeStack* stack)
{
	return *((x_uint64*)v);
}

template<>
x_uint64 __ObjectHash_Hash__(void* v, HazeStack* stack)
{
	return (x_uint64)v;
}

template<>
x_uint64 __ObjectHash_Hash__(ObjectString* v, HazeStack* stack)
{
	return v->Hash();
}

template<>
x_uint64 __ObjectHash_Hash__(ObjectClass* v, HazeStack* stack)
{
	return __ObjectHash_Hash__((void*)v, stack);
}

template<>
x_uint64 __ObjectHash_Hash__(ObjectBase* v, HazeStack* stack)
{
	extern x_uint64 __GetHashValue(HazeValueType type, void* value, HazeStack * stack);
	return __GetHashValue(v->GetBaseType(), v->GetBaseData(), stack);
}

x_uint64 __GetHashValue(HazeValueType type, void* value, HazeStack* stack)
{
	x_uint64 hashValue = 0;
	switch (type)
	{
		case HazeValueType::Bool:
			hashValue = __ObjectHash_Hash__((bool*)value, stack);
			break;
		case HazeValueType::Int8:
			hashValue = __ObjectHash_Hash__((x_int8*)value, stack);
			break;
		case HazeValueType::Int16:
			hashValue = __ObjectHash_Hash__((x_int16*)value, stack);
			break;
		case HazeValueType::Int32:
			hashValue = __ObjectHash_Hash__((x_int32*)value, stack);
			break;
		case HazeValueType::Int64:
			hashValue = __ObjectHash_Hash__((x_int64*)value, stack);
			break;
		case HazeValueType::UInt8:
			hashValue = __ObjectHash_Hash__((x_int8*)value, stack);
			break;
		case HazeValueType::UInt16:
			hashValue = __ObjectHash_Hash__((x_uint16*)value, stack);
			break;
		case HazeValueType::UInt32:
			hashValue = __ObjectHash_Hash__((x_uint32*)value, stack);
			break;
		case HazeValueType::UInt64:
			hashValue = __ObjectHash_Hash__((x_uint64*)value, stack);
			break;
		case HazeValueType::Float32:
			hashValue = __ObjectHash_Hash__((x_float32*)value, stack);
			break;
		case HazeValueType::Float64:
			hashValue = __ObjectHash_Hash__((x_float64*)value, stack);
			break;
		case HazeValueType::String:
			hashValue = __ObjectHash_Hash__(*((ObjectString**)value), stack);
			break;
		case HazeValueType::Class:
			hashValue = __ObjectHash_Hash__(*((ObjectClass**)value), stack);
			break;
		case HazeValueType::ObjectBase:
			hashValue = __ObjectHash_Hash__(*((ObjectBase**)value), stack);
			break;
		case HazeValueType::Enum:
			hashValue = __ObjectHash_Hash__((x_int8*)value, stack);
			break;
		case HazeValueType::Array:
			hashValue = __ObjectHash_Hash__(value, stack);
			break;
		default:
			OBJECT_ERR_W("类型<%s>不能哈希", GetHazeValueTypeString(type));
			break;
	}

	return hashValue;
}

//template<>
//x_uint64 __ObjectHash_Hash__(ObjectBase* v)
//{
//	return 0;
//}


ObjectHash::ObjectHash(x_uint32 gcIndex, HazeVM* vm, x_uint32 typeId)
	: GCObject(gcIndex), m_Capacity(DEFAULT_CAPACITY), m_Length(0)
{
	auto info = vm->GetTypeInfoMap()->GetTypeInfoById(typeId);
	if (IsHashType(info->Info.GetBaseType()))
	{
		m_KeyType = HazeVariableType(vm->GetTypeInfoMap()->GetTypeById(info->Info._Hash.TypeId1)->GetBaseType(), info->Info._Hash.TypeId1);
		m_ValueType = HazeVariableType(vm->GetTypeInfoMap()->GetTypeById(info->Info._Hash.TypeId2)->GetBaseType(), info->Info._Hash.TypeId2);
	}
	
	auto pair = HazeMemory::GetMemory()->AllocaGCData(m_Capacity * sizeof(ObjectHashNode), GC_ObjectType::HashData);
	m_DataGCIndex = pair.second;
	m_Data = (ObjectHashNode*)pair.first;
	m_LastFreeNode = m_Data + m_Capacity;
}

ObjectHash::~ObjectHash()
{
	HazeMemory::GetMemory()->Remove(m_Data, m_Capacity * sizeof(ObjectHashNode), m_DataGCIndex);
}

AdvanceClassInfo* ObjectHash::GetAdvanceClassInfo()
{
	static AdvanceClassInfo info;
	info.Add(H_TEXT("长度"), { &ObjectHash::GetLength, OBJ_TYPE_DEF(UInt64), {} });
	info.Add(H_TEXT("添加"), { &ObjectHash::Add, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(MultiVariable) } });
	info.Add(H_TEXT("移除"), { &ObjectHash::Remove, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(MultiVariable) } });

	info.Add(HAZE_ADVANCE_GET_FUNCTION, { &ObjectHash::Get, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(MultiVariable) } });
	info.Add(HAZE_ADVANCE_SET_FUNCTION, { &ObjectHash::Set, OBJ_TYPE_DEF(Void), { OBJ_TYPE_DEF(MultiVariable) } });

	return &info;
}

HazeVariableType ObjectHash::GetKeyBaseType()
{
	return m_KeyType;
}

HazeVariableType ObjectHash::GetValueBaseType()
{
	return m_ValueType;
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

	SET_RET_BY_TYPE(HazeVariableType(HazeValueType::UInt64), obj->m_Length);
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
	SetHazeValueByData(tempKey, obj->GetKeyBaseType().BaseType, key);

	HazeValue value;
	memset(&value, 0, sizeof(value));

	while (true)
	{
		if (IsEqualByType(obj->GetKeyBaseType().BaseType, node->Key, tempKey))
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

	SET_RET_BY_TYPE(obj->GetValueBaseType(), value);
	//HAZE_LOG_INFO("Array Get <%d>\n", offset);
}

void ObjectHash::Set(HAZE_OBJECT_CALL_PARAM)
{
	Add(HAZE_STD_CALL_PARAM_VAR);
}

x_uint64 ObjectHash::GetHash(ObjectHash* obj, void* value, HazeStack* stack)
{
	x_uint64 hashValue = __GetHashValue(obj->GetKeyBaseType().BaseType, value, stack);
	return hashValue % ((obj->m_Capacity - 1) | 1);
}

void ObjectHash::Add(HazeValue key, HazeValue value, HazeStack* stack)
{
	x_uint64 hashValue = GetHash(this, (void*)(&key), stack);

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
	SetHazeValueByData(tempKey, obj->GetKeyBaseType().BaseType, key);
	SetHazeValueByData(tempValue, obj->GetValueBaseType().BaseType, value);
	obj->Add(tempKey, tempValue, stack);
}

void ObjectHash::Remove(HAZE_OBJECT_CALL_PARAM)
{
	ObjectHash* obj;
	char* value = nullptr;

	GET_PARAM_START();
	GET_OBJ(obj);

	x_uint64 newCapacity = 1;
	x_uint64 doubleLength = (obj->m_Length - 1) * 2;
	while (newCapacity < doubleLength)
	{
		newCapacity = newCapacity << 1;
	}

	if (newCapacity < obj->m_Capacity)
	{
		// 重新Hash
		obj->Rehash();
	}
}
