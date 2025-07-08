#pragma once

#include "HazeDefine.h"
#include <iostream>

#define CAST_TYPE(V) (x_uint32)V
#define HAZE_TYPE_ID(TYPE) (x_uint32)TYPE
#define HAZE_ID_2_TYPE(ID) ((HazeValueType)(ID))
#define CAST_COMPLEX_INFO(INFO) (HazeComplexTypeInfoBase*)(&INFO)
#define ENUM_INT_TYPE HazeValueType::Int32

enum class HazeValueType : x_uint32
{
#define HAZE_TYPE_DEFINE(TYPE) TYPE,
	#include "HazeValueTypeTemplate"
#undef HAZE_TYPE_DEFINE
};

struct HazeValue
{
	union
	{
		bool Bool;

		x_int8 Int8;
		x_uint8 UInt8;
		x_int16 Int16;
		x_uint16 UInt16;
		x_int32 Int32;
		x_uint32 UInt32;
		x_int64 Int64;
		x_uint64 UInt64;

		x_float32 Float32;
		x_float64 Float64;

		const void* Pointer;

		union
		{
			int StringTableIndex;
			int MemorySize;
			//const HString* StringPointer;
		} Extra;
	} Value;

public:
	HazeValue& operator =(const HazeValue& V)
	{
		memcpy(this, &V, sizeof(V));
		return *this;
	}

	HazeValue& operator =(int64_t V)
	{
		memcpy(&this->Value, &V, sizeof(V));
		return *this;
	}
};

enum class InstructionOpCode : x_uint32;
enum class HazeToken : x_uint32;

x_uint32 GetSizeByHazeType(HazeValueType type);

HazeToken GetTokenByValueType(HazeValueType type);

HazeValueType GetValueTypeByToken(HazeToken token);

HazeValueType GetStrongerType(HazeValueType type1, HazeValueType type2, bool isLog = true);

bool IsNoneType(HazeValueType type);
bool IsVoidType(HazeValueType type);
bool IsHazeBaseTypeAndVoid(HazeValueType type);
bool IsHazeBaseType(HazeValueType type);
bool IsAdvanceType(HazeValueType type);
bool IsIntegerType(HazeValueType type);
bool IsUnsignedIntegerType(HazeValueType type);
bool IsFloatingType(HazeValueType type);
bool IsFunctionType(HazeValueType type);
bool IsNumberType(HazeValueType type);
bool IsClassType(HazeValueType type);
bool IsEnumType(HazeValueType type);
bool IsArrayType(HazeValueType type);
bool IsDynamicClassType(HazeValueType type);
bool IsDynamicClassUnknowType(HazeValueType type);
bool IsStringType(HazeValueType type);
bool IsPureStringType(HazeValueType type);
bool IsRefrenceType(HazeValueType type);
bool IsMultiVariableTye(HazeValueType type);
bool IsObjectFunctionType(HazeValueType type);
bool IsObjectBaseType(HazeValueType type);
bool IsHashType(HazeValueType type);
bool IsClosureType(HazeValueType type);

void StringToHazeValueNumber(const HString& str, HazeValueType type, HazeValue& value);
HString HazeValueNumberToString(HazeValueType type, HazeValue value);

void CalculateValueByType(HazeValueType type, InstructionOpCode typeCod, const void* source, const void* oper1, const void* oper2);

void CompareValueByType(HazeValueType type, struct HazeRegister* hazeRegister, const void* source, const void* target);
bool IsEqualByType(HazeValueType type, HazeValue v1, HazeValue v2);

size_t GetHazeCharPointerLength(const x_HChar* hChar);

const x_HChar* GetHazeValueTypeString(HazeValueType type);

void SetHazeValueByData(HazeValue& value, HazeValueType type, void* data);

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType type, const HazeValue& value);

HazeValue GetNegValue(HazeValueType type, const HazeValue& value);

bool CanPointer(HazeValueType type) { return IsHazeBaseType(type); }

bool CanCVT(HazeValueType type1, HazeValueType type2);

bool CanArray(HazeValueType type);

bool CanHash(HazeValueType type);

bool CanUseInTemplate(HazeValueType type);

bool IsUseTemplateType(HazeValueType type);
