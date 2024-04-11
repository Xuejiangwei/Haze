#include <unordered_set>
#include <set>
#include "HazeValue.h"
#include "HazeHeader.h"
#include "HazeInstruction.h"
#include "HazeLog.h"

extern const std::unordered_map<HAZE_STRING, HazeToken>& GetHashMap_Token();

static std::unordered_map<HazeToken, HazeValueType> s_HashMap_Types =
{
	{ HazeToken::Void, HazeValueType::Void },
	{ HazeToken::Bool, HazeValueType::Bool },
	{ HazeToken::Byte, HazeValueType::Byte },
	{ HazeToken::Char, HazeValueType::Char },
	{ HazeToken::Int, HazeValueType::Int },
	{ HazeToken::Float, HazeValueType::Float },
	{ HazeToken::Long, HazeValueType::Long },
	{ HazeToken::Double, HazeValueType::Double },
	{ HazeToken::UnsignedInt, HazeValueType::UnsignedInt },
	{ HazeToken::UnsignedLong, HazeValueType::UnsignedLong},
	{ HazeToken::CustomClass, HazeValueType::Class},
	{ HazeToken::ReferenceBase, HazeValueType::ReferenceBase},
	{ HazeToken::ReferenceClass, HazeValueType::ReferenceClass},
	{ HazeToken::PointerBase, HazeValueType::PointerBase},
	{ HazeToken::PointerClass, HazeValueType::PointerClass},
	{ HazeToken::PointerPointer, HazeValueType::PointerPointer},
	{ HazeToken::MultiVariable, HazeValueType::MultiVariable},
	//{ HazeToken::Array, HazeValueType::Array }
};

uint32 GetSizeByHazeType(HazeValueType type)
{
	switch (type)
	{
	case HazeValueType::Bool:
	case HazeValueType::Byte:
		return 1;
	case HazeValueType::Char:
		return 2;
	case HazeValueType::Int:
	case HazeValueType::Float:
	case HazeValueType::UnsignedInt:
		return 4;
	case HazeValueType::Long:
	case HazeValueType::Double:
	case HazeValueType::UnsignedLong:
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerFunction:
	case HazeValueType::PointerPointer:
	case HazeValueType::ReferenceBase:
	case HazeValueType::ReferenceClass:
	case HazeValueType::ArrayBase:
	case HazeValueType::ArrayClass:
	case HazeValueType::ArrayPointer:
		return 8;
	default:
		break;
	}
	return 0;
}

HazeToken GetTokenByValueType(HazeValueType type)
{
	static std::unordered_map<HazeValueType, HazeToken> s_HashMap_Tokens;
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

HazeValueType GetStrongerType(HazeValueType type1, HazeValueType type2)
{
	static std::unordered_map<HazeValueType, std::set<HazeValueType>> s_HashMap_Table =
	{
		{ HazeValueType::Int, { HazeValueType::Long, HazeValueType::UnsignedLong, HazeValueType::Float, HazeValueType::Double } },
		{ HazeValueType::UnsignedInt, { HazeValueType::UnsignedLong } },
		{ HazeValueType::Float, { HazeValueType::Double} },
		{ HazeValueType::Bool, { HazeValueType::Char, HazeValueType::Int, HazeValueType::UnsignedInt, HazeValueType::Long, HazeValueType::UnsignedLong } },
		{ HazeValueType::Char, { HazeValueType::Int, HazeValueType::UnsignedInt, HazeValueType::Long, HazeValueType::UnsignedLong } }
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

	HAZE_LOG_ERR_W("获得更强类型错误，<%s> <%s>!\n", GetHazeValueTypeString(type1), GetHazeValueTypeString(type2));
	return HazeValueType::Void;
}

bool IsNoneType(HazeValueType type)
{
	return type == HazeValueType::None;
}

bool IsVoidType(HazeValueType type)
{
	return type == HazeValueType::Void;
}

bool IsHazeDefaultTypeAndVoid(HazeValueType type)
{
	return HazeValueType::Void <= type && type <= HazeValueType::UnsignedLong;
}

bool IsHazeDefaultType(HazeValueType type)
{
	return HazeValueType::Bool < type && type <= HazeValueType::UnsignedLong;
}

bool IsIntegerType(HazeValueType type)
{
	return type == HazeValueType::Int || type == HazeValueType::Long || type == HazeValueType::UnsignedInt || type == HazeValueType::UnsignedLong;
}

bool IsUnsignedLongType(HazeValueType type)
{
	return type == HazeValueType::UnsignedLong;
}

bool IsPointerType(HazeValueType type)
{
	return type >= HazeValueType::PointerBase && type <= HazeValueType::PointerPointer;
}

bool IsOneLevelPointerType(HazeValueType type)
{
	return type >= HazeValueType::PointerBase && type <= HazeValueType::PointerFunction;
}

bool IsPointerFunction(HazeValueType type)
{
	return type == HazeValueType::PointerFunction;
}

bool IsPointerPointer(HazeValueType type)
{
	return type == HazeValueType::PointerPointer;
}

bool IsNumberType(HazeValueType type)
{
	static std::unordered_set<HazeValueType> HashSet_Table =
	{
		HazeValueType::Int, HazeValueType::Float, HazeValueType::Long, HazeValueType::Double,
		HazeValueType::UnsignedInt, HazeValueType::UnsignedLong,
	};

	return HashSet_Table.find(type) != HashSet_Table.end();
}

bool IsClassType(HazeValueType type)
{
	return type == HazeValueType::Class;
}

bool IsArrayType(HazeValueType type)
{
	return type >= HazeValueType::ArrayBase && type <= HazeValueType::ArrayPointer;
}

bool IsArrayPointerType(HazeValueType type)
{
	return type == HazeValueType::ArrayPointer;
}

bool IsReferenceType(HazeValueType type)
{
	return type == HazeValueType::ReferenceBase || type == HazeValueType::ReferenceClass;
}

void StringToHazeValueNumber(const HAZE_STRING& str, HazeValueType type, HazeValue& value)
{
	HAZE_STRING_STREAM wss;
	wss << str;

	switch (type)
	{
	case HazeValueType::Float:
		wss >> value.Value.Float;
		break;
	case HazeValueType::Double:
		wss >> value.Value.Double;
		break;
	case HazeValueType::Bool:
	case HazeValueType::Int:
		wss >> value.Value.Int;
		break;
	case HazeValueType::Long:
		wss >> value.Value.Long;
		break;
	case HazeValueType::UnsignedInt:
		wss >> value.Value.UnsignedInt;
		break;
	case HazeValueType::UnsignedLong:
		wss >> value.Value.UnsignedLong;
		break;
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerFunction:
	case HazeValueType::PointerPointer:
		wss >> value.Value.UnsignedLong;
		break;
	default:
		break;
	}
}

#define VARIABLE_DEFINE_INIT(TYPE, TARGET) TYPE T; memcpy(&T, target, sizeof(TYPE))
#define TWO_VARIABLE_DEFINE_INIT(TYPE, SOURCE, TARGET) TYPE S, T; memcpy(&S, source, sizeof(TYPE)); memcpy(&T, target, sizeof(TYPE))

#define VARIABLE_CALCULATE(TYPE, OP) CalculateValue<TYPE>(OP, T)
#define TWO_VARIABLE_CALCULATE(TYPE, OP) CalculateValue<TYPE>(OP, S, T)

#define ASSIGN(TYPE) memcpy((void*)target, &T, sizeof(TYPE))

#define VARIABLE_COMPARE() bool CmpEqual = S == T; bool CmpGreater = S > T; bool CmpLess = S < T
#define COMPARE_ASSIGN() hazeRegister->Data.resize(3); hazeRegister->Data[0] = CmpEqual; \
	hazeRegister->Data[1] = CmpGreater; hazeRegister->Data[2] = CmpLess

template<typename T>
void CalculateValue(InstructionOpCode typeCode, T& target)
{
	switch (typeCode)
	{
	case InstructionOpCode::INC:
		++target;
		break;
	case InstructionOpCode::DEC:
		--target;
		break;
	default:
		HAZE_LOG_ERR(HAZE_TEXT("<%s>操作不支持!"), WString2String(GetInstructionString(typeCode)).c_str());
		break;
	}
}

template<typename T>
void CalculateValue(InstructionOpCode typeCode, T& source, T& target)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
	case InstructionOpCode::ADD_ASSIGN:
		target += source;
		break;
	case InstructionOpCode::SUB:
	case InstructionOpCode::SUB_ASSIGN:
		target -= source;
		break;
	case InstructionOpCode::MUL:
	case InstructionOpCode::MUL_ASSIGN:
		target *= source;
		break;
	case InstructionOpCode::DIV:
	case InstructionOpCode::DIV_ASSIGN:
		target /= source;
		break;
	default:
		HAZE_LOG_ERR("Calculate error: operator %s  type %s\n", WString2String(GetInstructionString(typeCode)).c_str(), typeid(T).name());
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, bool& source, bool& target)
{
	switch (typeCode)
	{
	case InstructionOpCode::NOT:
		target = !source;
		break;
	default:
		HAZE_LOG_ERR("Calculate error: bool value is not support operator %s!\n", WString2String(GetInstructionString(typeCode)).c_str());
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, int& source, int& target)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
	case InstructionOpCode::ADD_ASSIGN:
		target += source;
		break;
	case InstructionOpCode::SUB:
	case InstructionOpCode::SUB_ASSIGN:
		target -= source;
		break;
	case InstructionOpCode::MUL:
	case InstructionOpCode::MUL_ASSIGN:
		target *= source;
		break;
	case InstructionOpCode::DIV:
	case InstructionOpCode::DIV_ASSIGN:
		target /= source;
		break;
	case InstructionOpCode::MOD:
	case InstructionOpCode::MOD_ASSIGN:
		target %= source;
		break;
	case InstructionOpCode::BIT_AND:
	case InstructionOpCode::BIT_AND_ASSIGN:
		target &= source;
		break;
	case InstructionOpCode::BIT_OR:
	case InstructionOpCode::BIT_OR_ASSIGN:
		target |= source;
		break;
	case InstructionOpCode::BIT_NEG:
		target = ~source;
		break;
	case InstructionOpCode::BIT_XOR:
	case InstructionOpCode::BIT_XOR_ASSIGN:
		target ^= source;
		break;
	case InstructionOpCode::SHL:
	case InstructionOpCode::SHL_ASSIGN:
		target <<= source;
		break;
	case InstructionOpCode::SHR:
	case InstructionOpCode::SHR_ASSIGN:
		target >>= source;
		break;
	case InstructionOpCode::NEG:
		target = -source;
		break;
	default:
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, uint32& source, uint32& target)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
	case InstructionOpCode::ADD_ASSIGN:
		target += source;
		break;
	case InstructionOpCode::SUB:
	case InstructionOpCode::SUB_ASSIGN:
		target -= source;
		break;
	case InstructionOpCode::MUL:
	case InstructionOpCode::MUL_ASSIGN:
		target *= source;
		break;
	case InstructionOpCode::DIV:
	case InstructionOpCode::DIV_ASSIGN:
		target /= source;
		break;
	case InstructionOpCode::MOD:
	case InstructionOpCode::MOD_ASSIGN:
		target %= source;
		break;
	case InstructionOpCode::BIT_AND:
	case InstructionOpCode::BIT_AND_ASSIGN:
		target &= source;
		break;
	case InstructionOpCode::BIT_OR:
	case InstructionOpCode::BIT_OR_ASSIGN:
		target |= source;
		break;
	case InstructionOpCode::BIT_XOR:
	case InstructionOpCode::BIT_XOR_ASSIGN:
		target ^= source;
		break;
	case InstructionOpCode::SHL:
	case InstructionOpCode::SHL_ASSIGN:
		target <<= source;
		break;
	case InstructionOpCode::SHR:
	case InstructionOpCode::SHR_ASSIGN:
		target >>= source;
		break;
	default:
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, int64& source, int64& target)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
	case InstructionOpCode::ADD_ASSIGN:
		target += source;
		break;
	case InstructionOpCode::SUB:
	case InstructionOpCode::SUB_ASSIGN:
		target -= source;
		break;
	case InstructionOpCode::MUL:
	case InstructionOpCode::MUL_ASSIGN:
		target *= source;
		break;
	case InstructionOpCode::DIV:
	case InstructionOpCode::DIV_ASSIGN:
		target /= source;
		break;
	case InstructionOpCode::MOD:
	case InstructionOpCode::MOD_ASSIGN:
		target %= source;
		break;
	case InstructionOpCode::BIT_AND:
	case InstructionOpCode::BIT_AND_ASSIGN:
		target &= source;
		break;
	case InstructionOpCode::BIT_OR:
	case InstructionOpCode::BIT_OR_ASSIGN:
		target |= source;
		break;
	case InstructionOpCode::BIT_XOR:
	case InstructionOpCode::BIT_XOR_ASSIGN:
		target ^= source;
		break;
	case InstructionOpCode::SHL:
	case InstructionOpCode::SHL_ASSIGN:
		target <<= source;
		break;
	case InstructionOpCode::SHR:
	case InstructionOpCode::SHR_ASSIGN:
		target >>= source;
		break;
	case InstructionOpCode::NEG:
		target = -source;
		break;
	default:
		break;
	}
}

template<>
void CalculateValue(InstructionOpCode typeCode, uint64& source, uint64& target)
{
	switch (typeCode)
	{
	case InstructionOpCode::ADD:
		target += source;
		break;
	case InstructionOpCode::SUB:
		target -= source;
		break;
	case InstructionOpCode::MUL:
		target *= source;
		break;
	case InstructionOpCode::DIV:
		target /= source;
		break;
	case InstructionOpCode::MOD:
		target %= source;
		break;
	case InstructionOpCode::BIT_AND:
		target &= source;
		break;
	case InstructionOpCode::BIT_OR:
		target |= source;
		break;
	case InstructionOpCode::BIT_XOR:
		target ^= source;
		break;
	case InstructionOpCode::SHL:
		target <<= source;
		break;
	case InstructionOpCode::SHR:
		target >>= source;
		break;
	default:
		break;
	}
}

void OperatorValueByType(HazeValueType type, InstructionOpCode typeCode, const void* target)
{
	switch (type)
	{
		/*case HazeValueType::Bool:
		{
			VARIABLE_DEFINE_INIT(bool, Target);
			VARIABLE_CALCULATE(bool, TypeCode);
			ASSIGN(bool);
		}
		break;*/
	case HazeValueType::Int:
	{
		VARIABLE_DEFINE_INIT(int, target);
		VARIABLE_CALCULATE(int, typeCode);
		ASSIGN(int);
	}
	break;
	case HazeValueType::Float:
	{
		VARIABLE_DEFINE_INIT(float, target);
		VARIABLE_CALCULATE(float, typeCode);
		ASSIGN(float);
	}
	break;
	case HazeValueType::Long:
	{
		VARIABLE_DEFINE_INIT(int64, target);
		VARIABLE_CALCULATE(int64, typeCode);
		ASSIGN(int64);
	}
	break;
	case HazeValueType::Double:
	{
		VARIABLE_DEFINE_INIT(double, target);
		VARIABLE_CALCULATE(double, typeCode);
		ASSIGN(double);
	}
	break;
	case HazeValueType::UnsignedInt:
	{
		VARIABLE_DEFINE_INIT(uint32, target);
		VARIABLE_CALCULATE(uint32, typeCode);
		ASSIGN(uint32);
	}
	break;
	case HazeValueType::UnsignedLong:
	{
		VARIABLE_DEFINE_INIT(uint64, target);
		VARIABLE_CALCULATE(uint64, typeCode);
		ASSIGN(uint64);
	}
	break;
	default:
		break;
	}
}

void CalculateValueByType(HazeValueType type, InstructionOpCode typeCode, const void* source, const void* target)
{
	switch (type)
	{
	case HazeValueType::Bool:
	{
		TWO_VARIABLE_DEFINE_INIT(bool, source, target);
		TWO_VARIABLE_CALCULATE(bool, typeCode);
		ASSIGN(bool);
	}
	break;
	case HazeValueType::Int:
	{
		TWO_VARIABLE_DEFINE_INIT(int, source, target);
		TWO_VARIABLE_CALCULATE(int, typeCode);
		ASSIGN(int);
	}
	break;
	case HazeValueType::Float:
	{
		TWO_VARIABLE_DEFINE_INIT(float, source, target);
		TWO_VARIABLE_CALCULATE(float, typeCode);
		ASSIGN(float);
	}
	break;
	case HazeValueType::Long:
	{
		TWO_VARIABLE_DEFINE_INIT(int64, source, target);
		TWO_VARIABLE_CALCULATE(int64, typeCode);
		ASSIGN(int64);
	}
	break;
	case HazeValueType::Double:
	{
		TWO_VARIABLE_DEFINE_INIT(double, source, target);
		TWO_VARIABLE_CALCULATE(double, typeCode);
		ASSIGN(double);
	}
	break;
	case HazeValueType::UnsignedInt:
	{
		TWO_VARIABLE_DEFINE_INIT(uint32, source, target);
		TWO_VARIABLE_CALCULATE(uint32, typeCode);
		ASSIGN(uint32);
	}
	break;
	case HazeValueType::UnsignedLong:
	{
		TWO_VARIABLE_DEFINE_INIT(uint64, source, target);
		TWO_VARIABLE_CALCULATE(uint64, typeCode);
		ASSIGN(uint64);
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
	case HazeValueType::Int:
	{
		TWO_VARIABLE_DEFINE_INIT(int, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::Float:
	{
		TWO_VARIABLE_DEFINE_INIT(float, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::Long:
	{
		TWO_VARIABLE_DEFINE_INIT(int64, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::Double:
	{
		TWO_VARIABLE_DEFINE_INIT(double, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::UnsignedInt:
	{
		TWO_VARIABLE_DEFINE_INIT(uint32, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::UnsignedLong:
	{
		TWO_VARIABLE_DEFINE_INIT(uint64, source, target);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerFunction:
	case HazeValueType::PointerPointer:
	{
		uint64 s = *(uint64*)source, t = *(uint64*)target;
		TWO_VARIABLE_DEFINE_INIT(uint64, s, t);
		VARIABLE_COMPARE();
		COMPARE_ASSIGN();
	}
	break;
	default:
		HAZE_LOG_ERR_W("类型比较错误!\n");
		break;
	}
}

size_t GetHazeCharPointerLength(const HAZE_CHAR* hChar)
{
	return wcslen(hChar);
}

const HAZE_CHAR* GetHazeValueTypeString(HazeValueType type)
{
	static std::unordered_map<HazeValueType, const HAZE_CHAR*> s_HashMap_Code2String;

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

	return HAZE_TEXT("None");
}

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType type, const HazeValue& value)
{
	switch (type)
	{
	case HazeValueType::Bool:
		return (HAZE_BINARY_CHAR*)&value.Value.Bool;
	case HazeValueType::Int:
		return (HAZE_BINARY_CHAR*)&value.Value.Int;
	case HazeValueType::Float:
		return (HAZE_BINARY_CHAR*)&value.Value.Float;
	case HazeValueType::UnsignedInt:
		return (HAZE_BINARY_CHAR*)&value.Value.UnsignedInt;
	case HazeValueType::Long:
		return (HAZE_BINARY_CHAR*)&value.Value.Long;
	case HazeValueType::Double:
		return (HAZE_BINARY_CHAR*)&value.Value.Double;
	case HazeValueType::UnsignedLong:
		return (HAZE_BINARY_CHAR*)&value.Value.UnsignedLong;
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerFunction:
	case HazeValueType::PointerPointer:
		return (HAZE_BINARY_CHAR*)&value.Value.UnsignedLong;
	default:
		break;
	}
	return nullptr;
}

HazeValue GetNegValue(HazeValueType type, const HazeValue& value)
{
	HazeValue ret;
	switch (type)
	{
	case HazeValueType::Int:
		ret.Value.Int = -value.Value.Int;
		break;
	case HazeValueType::Float:
		ret.Value.Float = -value.Value.Float;
		break;
	case HazeValueType::Long:
		ret.Value.Long = -value.Value.Long;
		break;
	case HazeValueType::Double:
		ret.Value.Double = -value.Value.Double;
		break;
	default:
		break;
	}

	return ret;
}