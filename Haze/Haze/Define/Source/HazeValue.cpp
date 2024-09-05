#include "HazePch.h"
#include "HazeValue.h"
#include "HazeHeader.h"
#include "HazeLog.h"

static HashMap<HazeToken, HazeValueType> s_HashMap_Types =
{
	{ HazeToken::Void, HazeValueType::Void },
	{ HazeToken::Bool, HazeValueType::Bool },

	{ HazeToken::Int8, HazeValueType::Int8 },
	{ HazeToken::UInt8, HazeValueType::UInt8 },
	{ HazeToken::Int16, HazeValueType::Int16 },
	{ HazeToken::UInt16, HazeValueType::UInt16 },
	{ HazeToken::Int32, HazeValueType::Int32 },
	{ HazeToken::UInt32, HazeValueType::UInt32 },
	{ HazeToken::Int64, HazeValueType::Int64 },
	{ HazeToken::UInt64, HazeValueType::UInt64},
	
	{ HazeToken::Float32, HazeValueType::Float32 },
	{ HazeToken::Float64, HazeValueType::Float64 },

	{ HazeToken::String, HazeValueType::String },

	{ HazeToken::CustomClass, HazeValueType::Class },
	{ HazeToken::CustomEnum, HazeValueType::Enum },

	{ HazeToken::Function, HazeValueType::Function },

	{ HazeToken::Reference, HazeValueType::Refrence },

	{ HazeToken::MultiVariable, HazeValueType::MultiVariable },
};

uint32 GetSizeByHazeType(HazeValueType type)
{
	switch (type)
	{
	case HazeValueType::Bool:
	case HazeValueType::Int8:
	case HazeValueType::UInt8:
		return 1;
	case HazeValueType::Int16:
	case HazeValueType::UInt16:
		return 2;
	case HazeValueType::Int32:
	case HazeValueType::UInt32:
	case HazeValueType::Float32:
		return 4;
	case HazeValueType::Int64:
	case HazeValueType::UInt64:
	case HazeValueType::Float64:
	case HazeValueType::Refrence:
	case HazeValueType::Array:
	case HazeValueType::String:
	case HazeValueType::Class:
	case HazeValueType::Function:
		return 8;
	case HazeValueType::Void:
	case HazeValueType::MultiVariable:
		return 0;
	default:
		HAZE_LOG_ERR_W("获得类型<%s>的大小错误!\n", GetHazeValueTypeString(type));
		break;
	}
	return 0;
}

HazeToken GetTokenByValueType(HazeValueType type)
{
	static HashMap<HazeValueType, HazeToken> s_HashMap_Tokens;
	if (s_HashMap_Tokens.size() <= 0)
	{
		for (auto& iter : s_HashMap_Types)
		{
			s_HashMap_Tokens[iter.second] = iter.first;
		}
	}

	auto it = s_HashMap_Tokens.find(type);
	if (it != s_HashMap_Tokens.end())
	{
		return it->second;
	}

	return HazeToken::Identifier;
}

HazeValueType GetValueTypeByToken(HazeToken token)
{
	auto it = s_HashMap_Types.find(token);
	if (it != s_HashMap_Types.end())
	{
		return it->second;
	}

	return HazeValueType::None;
}

HazeValueType GetStrongerType(HazeValueType type1, HazeValueType type2, bool isLog)
{
	static HashMap<HazeValueType, Set<HazeValueType>> s_HashMap_Table =
	{
		{ HazeValueType::Int32, { HazeValueType::Int64, HazeValueType::UInt64, HazeValueType::Float32, HazeValueType::Float64 } },
		{ HazeValueType::UInt32, { HazeValueType::UInt64 } },
		{ HazeValueType::Float32, { HazeValueType::Float64 } },
		{ HazeValueType::Bool, { HazeValueType::Int8, HazeValueType::UInt8, HazeValueType::Int16, HazeValueType::UInt16,
			HazeValueType::Int32, HazeValueType::UInt32, HazeValueType::Int64, HazeValueType::UInt64 } },
	};

	if (type1 == type2)
	{
		return type1;
	}
	else
	{
		auto it1 = s_HashMap_Table.find(type1);
		auto it2 = s_HashMap_Table.find(type2);
		if (it1 != s_HashMap_Table.end() && it1->second.find(type2) != it1->second.end())
		{
			return type2;
		}
		else if (it2 != s_HashMap_Table.end() && it2->second.find(type1) != it2->second.end())
		{
			return type1;
		}
	}

	if (isLog)
	{
		HAZE_LOG_ERR_W("获得更强类型错误，<%s> <%s>!\n", GetHazeValueTypeString(type1), GetHazeValueTypeString(type2));
	}

	return HazeValueType::None;
}

bool IsNoneType(HazeValueType type)
{
	return type == HazeValueType::None;
}

bool IsVoidType(HazeValueType type)
{
	return type == HazeValueType::Void;
}

bool IsHazeBaseTypeAndVoid(HazeValueType type)
{
	return (HazeValueType::__BaseType_Begin < type && type < HazeValueType::__BaseType_End) || type == HazeValueType::Void;
}

bool IsHazeBaseType(HazeValueType type)
{
	return HazeValueType::__BaseType_Begin < type && type < HazeValueType::__BaseType_End;
}

bool IsAdvanceType(HazeValueType type)
{
	return HazeValueType::__Advance_Begin < type && type < HazeValueType::__Advance_End;
}

bool IsIntegerType(HazeValueType type)
{
	return HazeValueType::Int8 <= type && type <= HazeValueType::UInt64;
}

bool IsUnsignedIntegerType(HazeValueType type)
{
	return HazeValueType::UInt8 <= type && type <= HazeValueType::UInt64;
}

bool IsFloatingType(HazeValueType type)
{
	return type == HazeValueType::Float32 || type == HazeValueType::Float64;
}

bool IsClassType(HazeValueType type)
{
	return type == HazeValueType::Class;
}

bool IsFunctionType(HazeValueType type)
{
	return type == HazeValueType::Function;
}

bool IsNumberType(HazeValueType type)
{
	return IsIntegerType(type) || IsFloatingType(type);
}

bool IsEnumType(HazeValueType type)
{
	return type == HazeValueType::Enum;
}

bool IsArrayType(HazeValueType type)
{
	return type == HazeValueType::Array;
}

bool IsStringType(HazeValueType type)
{
	return type == HazeValueType::String;
}

bool IsRefrenceType(HazeValueType type)
{
	return type == HazeValueType::Refrence;
}

bool IsMultiVariableTye(HazeValueType type)
{
	return type == HazeValueType::MultiVariable;
}

void StringToHazeValueNumber(const HString& str, HazeValueType type, HazeValue& value)
{
	static HAZE_STRING_STREAM wss;
	wss.clear();
	wss << str;

	switch (type)
	{
	case HazeValueType::Int8:
	case HazeValueType::Int16:
		wss >> value.Value.Int16;
		break;
	case HazeValueType::UInt8:
	case HazeValueType::UInt16:
		wss >> value.Value.UInt16;
		break;
	case HazeValueType::Bool:
	case HazeValueType::Int32:
		wss >> value.Value.Int32;
		break;
	case HazeValueType::UInt32:
		wss >> value.Value.UInt32;
		break;
	case HazeValueType::Int64:
		wss >> value.Value.Int64;
		break;
	case HazeValueType::UInt64:
		wss >> value.Value.UInt64;
		break;
	case HazeValueType::Float32:
		wss >> value.Value.Float32;
		break;
	case HazeValueType::Float64:
		wss >> value.Value.Float64;
		break;
	default:
		break;
	}
}

#define TWO_VARIABLE_DEFINE_INIT(TYPE, OPER1, OPER2) TYPE T1, T2; memcpy(&T1, OPER1, sizeof(TYPE)); memcpy(&T2, OPER2, sizeof(TYPE))
#define CALC_ASSIGN_VALUE(TYPE, T_CODE, TARGET, OPER1, OPER2) TYPE T, T1, T2; \
	memcpy(&T, TARGET, sizeof(TYPE)); memcpy(&T1, OPER1, sizeof(TYPE)); memcpy(&T2, OPER2, sizeof(TYPE)); \
	CalculateValue<TYPE>(T_CODE, T, T1, T2); memcpy((void*)TARGET, &T, sizeof(TYPE))

#define VARIABLE_CALCULATE(TYPE, OP) CalculateValue<TYPE>(OP, T)

#define VARIABLE_COMPARE() bool CmpEqual = T1 == T2; bool CmpGreater = T1 > T2; bool CmpLess = T1 < T2
#define COMPARE_ASSIGN() hazeRegister->Data.resize(3); hazeRegister->Data[0] = CmpEqual; \
	hazeRegister->Data[1] = CmpGreater; hazeRegister->Data[2] = CmpLess

template<typename T>
void CalculateValue(InstructionOpCode typeCode, T& target, T& oper1, T& oper2)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
		target = oper1 + oper2;
		break;
	case InstructionOpCode::SUB:
		target = oper1 - oper2;
		break;
	case InstructionOpCode::MUL:
		target = oper1 * oper2;
		break;
	case InstructionOpCode::DIV:
		target = oper1 / oper2;
		break;
	case InstructionOpCode::MOD:
		target = oper1 % oper2;
		break;
	case InstructionOpCode::BIT_AND:
		target = oper1 & oper2;
		break;
	case InstructionOpCode::BIT_OR:
		target = oper1 | oper2;
		break;
	case InstructionOpCode::BIT_XOR:
		target = oper1 ^ oper2;
		break;
	case InstructionOpCode::SHL:
		target = oper1 << oper2;
		break;
	case InstructionOpCode::SHR:
		target = oper1 >> oper2;
		break;
	case InstructionOpCode::BIT_NEG:
		target = ~oper1;
		break;
	case InstructionOpCode::NEG:
		target = oper1 * -1;
		break;
	default:
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, float& target, float& oper1, float& oper2)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
		target = oper1 + oper2;
		break;
	case InstructionOpCode::SUB:
		target = oper1 - oper2;
		break;
	case InstructionOpCode::MUL:
		target = oper1 * oper2;
		break;
	case InstructionOpCode::DIV:
		target = oper1 / oper2;
		break;
	default:
		INS_ERR_CODE_W("32位浮点数计算错误", typeCode);
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, double& target, double& oper1, double& oper2)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
		target = oper1 + oper2;
		break;
	case InstructionOpCode::SUB:
		target = oper1 - oper2;
		break;
	case InstructionOpCode::MUL:
		target = oper1 * oper2;
		break;
	case InstructionOpCode::DIV:
		target = oper1 / oper2;
		break;
	default:
		INS_ERR_CODE_W("64位浮点数计算错误", typeCode);
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, bool& target, bool& oper1, bool& oper2)
{
	switch (typeCode)
	{
	case InstructionOpCode::NOT:
		target = !oper1;
		break;
	default:
		INS_ERR_CODE_W("布尔计算错误", typeCode);
		break;
	}
}

//template<>
//void CalculateValue(InstructionOpCode typeCode, uint32& target, uint32& oper1, uint32& oper2)
//{
//	switch (typeCode)
//	{
//	case InstructionOpCode::ADD:
//		target += source;
//		break;
//	case InstructionOpCode::SUB:
//		target -= source;
//		break;
//	case InstructionOpCode::MUL:
//		target *= source;
//		break;
//	case InstructionOpCode::DIV:
//		target /= source;
//		break;
//	case InstructionOpCode::MOD:
//		target %= source;
//		break;
//	case InstructionOpCode::BIT_AND:
//		target &= source;
//		break;
//	case InstructionOpCode::BIT_OR:
//		target |= source;
//		break;
//	case InstructionOpCode::BIT_XOR:
//		target ^= source;
//		break;
//	case InstructionOpCode::SHL:
//		target <<= source;
//		break;
//	case InstructionOpCode::SHR:
//		target >>= source;
//		break;
//	default:
//		break;
//	}
//}
//
//template<>
//void CalculateValue(InstructionOpCode typeCode, int64& source, int64& target)
//{
//	switch (typeCode)
//	{
//	case InstructionOpCode::ADD:
//		target += source;
//		break;
//	case InstructionOpCode::SUB:
//		target -= source;
//		break;
//	case InstructionOpCode::MUL:
//		target *= source;
//		break;
//	case InstructionOpCode::DIV:
//		target /= source;
//		break;
//	case InstructionOpCode::MOD:
//		target %= source;
//		break;
//	case InstructionOpCode::BIT_AND:
//		target &= source;
//		break;
//	case InstructionOpCode::BIT_OR:
//		target |= source;
//		break;
//	case InstructionOpCode::BIT_XOR:
//		target ^= source;
//		break;
//	case InstructionOpCode::SHL:
//		target <<= source;
//		break;
//	case InstructionOpCode::SHR:
//		target >>= source;
//		break;
//	case InstructionOpCode::NEG:
//		target = -source;
//		break;
//	default:
//		break;
//	}
//}
//
//template<>
//void CalculateValue(InstructionOpCode typeCode, uint64& source, uint64& target)
//{
//	switch (typeCode)
//	{
//	case InstructionOpCode::ADD:
//		target += source;
//		break;
//	case InstructionOpCode::SUB:
//		target -= source;
//		break;
//	case InstructionOpCode::MUL:
//		target *= source;
//		break;
//	case InstructionOpCode::DIV:
//		target /= source;
//		break;
//	case InstructionOpCode::MOD:
//		target %= source;
//		break;
//	case InstructionOpCode::BIT_AND:
//		target &= source;
//		break;
//	case InstructionOpCode::BIT_OR:
//		target |= source;
//		break;
//	case InstructionOpCode::BIT_XOR:
//		target ^= source;
//		break;
//	case InstructionOpCode::SHL:
//		target <<= source;
//		break;
//	case InstructionOpCode::SHR:
//		target >>= source;
//		break;
//	default:
//		break;
//	}
//}

void CalculateValueByType(HazeValueType type, InstructionOpCode typeCode, const void* source, const void* oper1, const void* oper2)
{
	switch (type)
	{
	case HazeValueType::Bool:
	{
		CALC_ASSIGN_VALUE(bool, typeCode, source, oper1, oper2);
	}
	break;
	case HazeValueType::Int32:
	{
		CALC_ASSIGN_VALUE(int, typeCode, source, oper1, oper2);
	}
	break;
	case HazeValueType::Float32:
	{
		CALC_ASSIGN_VALUE(float, typeCode, source, oper1, oper2);
	}
	break;
	case HazeValueType::Int64:
	{
		CALC_ASSIGN_VALUE(int64, typeCode, source, oper1, oper2);
	}
	break;
	case HazeValueType::Float64:
	{
		CALC_ASSIGN_VALUE(double, typeCode, source, oper1, oper2);
	}
	break;
	case HazeValueType::UInt32:
	{
		CALC_ASSIGN_VALUE(uint32, typeCode, source, oper1, oper2);
	}
	break;
	case HazeValueType::UInt64:
	{
		CALC_ASSIGN_VALUE(uint64, typeCode, source, oper1, oper2);
	}
	break;
	default:
		break;
	}
}

void CompareValueByType(HazeValueType type, HazeRegister* hazeRegister, const void* source, const void* target)
{
	switch (type)
	{
	case HazeValueType::Bool:
	{
		TWO_VARIABLE_DEFINE_INIT(bool, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::Int32:
	{
		TWO_VARIABLE_DEFINE_INIT(int32, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::UInt32:
	{
		TWO_VARIABLE_DEFINE_INIT(uint32, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::Int64:
	{
		TWO_VARIABLE_DEFINE_INIT(int64, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::UInt64:
	{
		TWO_VARIABLE_DEFINE_INIT(uint64, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::Float32:
	{
		TWO_VARIABLE_DEFINE_INIT(float32, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	
	case HazeValueType::Float64:
	{
		TWO_VARIABLE_DEFINE_INIT(float64, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::Array:
	case HazeValueType::Class:
	case HazeValueType::Function:
	{
		//uint64 s = *(uint64*)source, t = *(uint64*)target;
		TWO_VARIABLE_DEFINE_INIT(uint64, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	default:
		HAZE_LOG_ERR_W("类型比较错误!\n");
		break;
	}
}

size_t GetHazeCharPointerLength(const HChar* hChar)
{
	return wcslen(hChar);
}

const HChar* GetHazeValueTypeString(HazeValueType type)
{
	extern const HashMap<HString, HazeToken>& GetHashMap_Token();
	static HashMap<HazeValueType, const HChar*> s_HashMap_Code2String;

	if (s_HashMap_Code2String.size() <= 0)
	{
		for (auto& iter : GetHashMap_Token())
		{
			s_HashMap_Code2String[GetValueTypeByToken(iter.second)] = iter.first.c_str();
		}
	}

	auto iter = s_HashMap_Code2String.find(type);
	if (iter != s_HashMap_Code2String.end())
	{
		return iter->second;
	}

	return H_TEXT("None");
}

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType type, const HazeValue& value)
{
	switch (type)
	{
	case HazeValueType::Bool:
		return (HAZE_BINARY_CHAR*)&value.Value.Bool;
	case HazeValueType::Int32:
		return (HAZE_BINARY_CHAR*)&value.Value.Int32;
	case HazeValueType::UInt32:
		return (HAZE_BINARY_CHAR*)&value.Value.UInt32;
	case HazeValueType::Int64:
		return (HAZE_BINARY_CHAR*)&value.Value.Int64;
	case HazeValueType::UInt64:
		return (HAZE_BINARY_CHAR*)&value.Value.UInt64;
	case HazeValueType::Float32:
		return (HAZE_BINARY_CHAR*)&value.Value.Float32;
	case HazeValueType::Float64:
		return (HAZE_BINARY_CHAR*)&value.Value.Float64;
	case HazeValueType::Array:
	case HazeValueType::Class:
	case HazeValueType::Function:
		return (HAZE_BINARY_CHAR*)&value.Value.UInt64;
	default:
		break;
	}
	return nullptr;
}

HazeValue GetNegValue(HazeValueType type, const HazeValue& value)
{
	HazeValue ret;
	ret = 0;
	switch (type)
	{
	case HazeValueType::Int32:
		ret.Value.Int32 = -value.Value.Int32;
		break;
	case HazeValueType::Int64:
		ret.Value.Int64 = -value.Value.Int64;
		break;
	case HazeValueType::Float32:
		ret.Value.Float32 = -value.Value.Float32;
		break;
	case HazeValueType::Float64:
		ret.Value.Float64 = -value.Value.Float64;
		break;
	default:
		break;
	}

	return ret;
}

bool CanCVT(HazeValueType type1, HazeValueType type2)
{
	switch (type1)
	{
	case HazeValueType::Int32:
	{
		switch (type2)
		{
		case HazeValueType::UInt32:
		case HazeValueType::Int64:
		case HazeValueType::UInt64:
		case HazeValueType::Float32:
		case HazeValueType::Float64:
			return true;
		default:
			break;
		}
	}
	break;
	case HazeValueType::Float32:
		switch (type2)
		{
		case HazeValueType::Int32:
		case HazeValueType::UInt32:
		case HazeValueType::Int64:
		case HazeValueType::UInt64:
		case HazeValueType::Float64:
			return true;
		default:
			break;
		}
		break;
	case HazeValueType::Int64:
		switch (type2)
		{
		case HazeValueType::Int32:
		case HazeValueType::UInt32:
		case HazeValueType::UInt64:
		case HazeValueType::Float32:
		case HazeValueType::Float64:
			return true;
		default:
			break;
		}
		break;
	case HazeValueType::Float64:
		switch (type2)
		{
		case HazeValueType::Int32:
		case HazeValueType::UInt32:
		case HazeValueType::Int64:
		case HazeValueType::UInt64:
		case HazeValueType::Float32:
			return true;
		default:
			break;
		}
		break;
	case HazeValueType::UInt32:
		switch (type2)
		{
		case HazeValueType::Int32:
		case HazeValueType::Int64:
		case HazeValueType::UInt64:
		case HazeValueType::Float32:
		case HazeValueType::Float64:
			return true;
		default:
			break;
		}
		break;
	case HazeValueType::UInt64:
		switch (type2)
		{
		case HazeValueType::Int32:
		case HazeValueType::UInt32:
		case HazeValueType::Int64:
		case HazeValueType::Float32:
		case HazeValueType::Float64:
			return true;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return false;
}
