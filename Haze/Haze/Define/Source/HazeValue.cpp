#include <unordered_set>
#include <set>
#include "HazeValue.h"
#include "Haze.h"
#include "HazeInstruction.h"
#include "HazeLog.h"

uint32 GetSizeByHazeType(HazeValueType Type)
{
    switch (Type)
    {
    case HazeValueType::Bool:
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
        return 8;
    default:
        break;
    }
    return 0;
}

HazeValueType GetValueTypeByToken(HazeToken Token)
{
    static std::unordered_map<HazeToken, HazeValueType> HashMap =
    {
        { HazeToken::Void, HazeValueType::Void },
        { HazeToken::Bool, HazeValueType::Bool },
        { HazeToken::Char, HazeValueType::Char },
        { HazeToken::Int, HazeValueType::Int },
        { HazeToken::Float, HazeValueType::Float },
        { HazeToken::Long, HazeValueType::Long },
        { HazeToken::Double, HazeValueType::Double },
        { HazeToken::UnsignedInt, HazeValueType::UnsignedInt },
        { HazeToken::UnsignedLong, HazeValueType::UnsignedLong},
        { HazeToken::CustomClass, HazeValueType::Class},
        { HazeToken::PointerBase, HazeValueType::PointerBase},
        { HazeToken::PointerClass, HazeValueType::PointerClass},
        { HazeToken::MultiVariable, HazeValueType::MultiVariable},
    };

    auto it = HashMap.find(Token);
    if (it != HashMap.end())
    {
        return it->second;
    }

    return HazeValueType::Void;
}

HazeValueType GetStrongerType(HazeValueType Type1, HazeValueType Type2)
{
    static std::unordered_map<HazeValueType, std::set<HazeValueType>> HashMap_Table =
    {
        { HazeValueType::Int, { HazeValueType::Long, HazeValueType::Float, HazeValueType::Double } },
        { HazeValueType::UnsignedInt, { HazeValueType::UnsignedLong } },
        { HazeValueType::Float, { HazeValueType::Double} }
    };

    if (Type1 == Type2)
    {
        return Type1;
    }
    else
    {
        auto it1 = HashMap_Table.find(Type1);
        auto it2 = HashMap_Table.find(Type2);
        if (it1 != HashMap_Table.end() && it1->second.find(Type2) != it1->second.end())
        {
            return Type2;
        }
        else if (it2 != HashMap_Table.end() && it2->second.find(Type1) != it2->second.end())
        {
            return Type1;
        }
    }

    static_assert(true, "Not find stronger type!");

    return HazeValueType::Void;
}

bool IsHazeDefaultType(HazeValueType Type)
{
    return HazeValueType::Void <= Type && Type <= HazeValueType::UnsignedLong;
}

bool IsIntegerType(HazeValueType Type)
{
    return Type == HazeValueType::Int || Type == HazeValueType::Long || Type == HazeValueType::UnsignedInt || Type == HazeValueType::UnsignedLong;
}

bool IsNumberType(HazeValueType Type)
{
    static std::unordered_set<HazeValueType> HashSet_Table =
    {
		HazeValueType::Int, HazeValueType::Float, HazeValueType::Long, HazeValueType::Double, 
        HazeValueType::UnsignedInt, HazeValueType::UnsignedLong,
    };

    return HashSet_Table.find(Type) != HashSet_Table.end();
}

void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValueType Type, HazeValue& Value)
{
    HAZE_STRING_STREAM WSS;
    WSS << Str;

    switch (Type)
    {
    case HazeValueType::Float:
        WSS >> Value.Value.Float;
        break;
    case HazeValueType::Double:
        WSS >> Value.Value.Double;
        break;
    case HazeValueType::Int:
        WSS >> Value.Value.Int;
        break;
    case HazeValueType::Long:
        WSS >> Value.Value.Long;
        break;
    case HazeValueType::UnsignedInt:
        WSS >> Value.Value.UnsignedInt;
        break;
    case HazeValueType::UnsignedLong:
        WSS >> Value.Value.UnsignedLong;
        break;
    default:
        break;
    }
}

#define VARIABLE_DEFINE_INIT(TYPE, SOURCE, TARGET) TYPE S, T; memcpy(&S, Source, sizeof(TYPE)); memcpy(&T, Target, sizeof(TYPE))

#define VARIABLE_CALCULATE(TYPE, OP) CalculateValue<TYPE>(OP, S, T)
#define ASSIGN(TYPE) memcpy((void*)Target, &T, sizeof(TYPE))

#define VARIABLE_COMPARE() bool CmpEqual = S == T; bool CmpGreater = S > T; bool CmpLess = S < T
#define COMPARE_ASSIGN() Register->Data.resize(3); Register->Data[0] = CmpEqual; Register->Data[1] = CmpGreater; Register->Data[2] = CmpLess

template<typename T>
void CalculateValue(InstructionOpCode TypeCode, T& Source, T& Target)
{
    switch (TypeCode)
    {
    case InstructionOpCode::ADD:
    case InstructionOpCode::ADD_ASSIGN:
        Target += Source;
        break;
    case InstructionOpCode::SUB:
    case InstructionOpCode::SUB_ASSIGN:
        Target -= Source;
        break;
    case InstructionOpCode::MUL:
    case InstructionOpCode::MUL_ASSIGN:
        Target *= Source;
        break;
    case InstructionOpCode::DIV:
    case InstructionOpCode::DIV_ASSIGN:
        Target /= Source;
        break;
    default:
        HAZE_LOG_ERR("Calculate error: operator %s  type %s\n", WString2String(GetInstructionString(TypeCode)).c_str(), typeid(T).name());
        break;
    }
}

template<>
void CalculateValue(InstructionOpCode TypeCode, int& Source, int& Target)
{
    switch (TypeCode)
    {
    case InstructionOpCode::ADD:
    case InstructionOpCode::ADD_ASSIGN:
        Target += Source;
        break;
    case InstructionOpCode::SUB:
    case InstructionOpCode::SUB_ASSIGN:
        Target -= Source;
        break;
    case InstructionOpCode::MUL:
    case InstructionOpCode::MUL_ASSIGN:
        Target *= Source;
        break;
    case InstructionOpCode::DIV:
    case InstructionOpCode::DIV_ASSIGN:
        Target /= Source;
        break;
    case InstructionOpCode::MOD:
    case InstructionOpCode::MOD_ASSIGN:
        Target %= Source;
        break;
    case InstructionOpCode::BIT_AND:
    case InstructionOpCode::BIT_AND_ASSIGN:
        Target &= Source;
        break;
    case InstructionOpCode::BIT_OR:
    case InstructionOpCode::BIT_OR_ASSIGN:
        Target |= Source;
        break;
    case InstructionOpCode::BIT_XOR:
    case InstructionOpCode::BIT_XOR_ASSIGN:
        Target ^= Source;
        break;
    case InstructionOpCode::SHL:
    case InstructionOpCode::SHL_ASSIGN:
        Target <<= Source;
        break;
    case InstructionOpCode::SHR:
    case InstructionOpCode::SHR_ASSIGN:
        Target >>= Source;
        break;
    default:
        break;
    }
}

template<>
void CalculateValue(InstructionOpCode TypeCode, uint32& Source, uint32& Target)
{
    switch (TypeCode)
    {
    case InstructionOpCode::ADD:
    case InstructionOpCode::ADD_ASSIGN:
        Target += Source;
        break;
    case InstructionOpCode::SUB:
    case InstructionOpCode::SUB_ASSIGN:
        Target -= Source;
        break;
    case InstructionOpCode::MUL:
    case InstructionOpCode::MUL_ASSIGN:
        Target *= Source;
        break;
    case InstructionOpCode::DIV:
    case InstructionOpCode::DIV_ASSIGN:
        Target /= Source;
        break;
    case InstructionOpCode::MOD:
    case InstructionOpCode::MOD_ASSIGN:
        Target %= Source;
        break;
    case InstructionOpCode::BIT_AND:
    case InstructionOpCode::BIT_AND_ASSIGN:
        Target &= Source;
        break;
    case InstructionOpCode::BIT_OR:
    case InstructionOpCode::BIT_OR_ASSIGN:
        Target |= Source;
        break;
    case InstructionOpCode::BIT_XOR:
    case InstructionOpCode::BIT_XOR_ASSIGN:
        Target ^= Source;
        break;
    case InstructionOpCode::SHL:
    case InstructionOpCode::SHL_ASSIGN:
        Target <<= Source;
        break;
    case InstructionOpCode::SHR:
    case InstructionOpCode::SHR_ASSIGN:
        Target >>= Source;
        break;
    default:
        break;
    }
}

template<>
void CalculateValue(InstructionOpCode TypeCode, int64& Source, int64& Target)
{
    switch (TypeCode)
    {
    case InstructionOpCode::ADD:
    case InstructionOpCode::ADD_ASSIGN:
        Target += Source;
        break;
    case InstructionOpCode::SUB:
    case InstructionOpCode::SUB_ASSIGN:
        Target -= Source;
        break;
    case InstructionOpCode::MUL:
    case InstructionOpCode::MUL_ASSIGN:
        Target *= Source;
        break;
    case InstructionOpCode::DIV:
    case InstructionOpCode::DIV_ASSIGN:
        Target /= Source;
        break;
    case InstructionOpCode::MOD:
    case InstructionOpCode::MOD_ASSIGN:
        Target %= Source;
        break;
    case InstructionOpCode::BIT_AND:
    case InstructionOpCode::BIT_AND_ASSIGN:
        Target &= Source;
        break;
    case InstructionOpCode::BIT_OR:
    case InstructionOpCode::BIT_OR_ASSIGN:
        Target |= Source;
        break;
    case InstructionOpCode::BIT_XOR:
    case InstructionOpCode::BIT_XOR_ASSIGN:
        Target ^= Source;
        break;
    case InstructionOpCode::SHL:
    case InstructionOpCode::SHL_ASSIGN:
        Target <<= Source;
        break;
    case InstructionOpCode::SHR:
    case InstructionOpCode::SHR_ASSIGN:
        Target >>= Source;
        break;
    default:
        break;
    }
}

template<>
void CalculateValue(InstructionOpCode TypeCode, uint64& Source, uint64& Target)
{
    switch (TypeCode)
    {
    case InstructionOpCode::ADD:
        Target += Source;
        break;
    case InstructionOpCode::SUB:
        Target -= Source;
        break;
    case InstructionOpCode::MUL:
        Target *= Source;
        break;
    case InstructionOpCode::DIV:
        Target /= Source;
        break;
    case InstructionOpCode::MOD:
        Target %= Source;
        break;
    case InstructionOpCode::BIT_AND:
        Target &= Source;
        break;
    case InstructionOpCode::BIT_OR:
        Target |= Source;
        break;
    case InstructionOpCode::BIT_XOR:
        Target ^= Source;
        break;
    case InstructionOpCode::SHL:
        Target <<= Source;
        break;
    case InstructionOpCode::SHR:
        Target >>= Source;
        break;
    default:
        break;
    }
}

void CalculateValueByType(HazeValueType Type, InstructionOpCode TypeCode, const void* Source, const void* Target)
{
    switch (Type)
    {
    case HazeValueType::Int:
    {
        VARIABLE_DEFINE_INIT(int, Source, Target);
        VARIABLE_CALCULATE(int, TypeCode);
        ASSIGN(int);
    }
        break;
    case HazeValueType::Float:
    {
        VARIABLE_DEFINE_INIT(float, Source, Target);
        VARIABLE_CALCULATE(float, TypeCode);
        ASSIGN(float);
    }
        break;
    case HazeValueType::Long:
    {
        VARIABLE_DEFINE_INIT(int64, Source, Target);
        VARIABLE_CALCULATE(int64, TypeCode);
        ASSIGN(int64);
    }
        break;
    case HazeValueType::Double:
    {
        VARIABLE_DEFINE_INIT(double, Source, Target);
        VARIABLE_CALCULATE(double, TypeCode);
        ASSIGN(double);
    }
        break;
    case HazeValueType::UnsignedInt:
    {
        VARIABLE_DEFINE_INIT(uint32, Source, Target);
        VARIABLE_CALCULATE(uint32, TypeCode);
        ASSIGN(uint32);
    }
        break;
    case HazeValueType::UnsignedLong:
    {
        VARIABLE_DEFINE_INIT(uint64, Source, Target);
        VARIABLE_CALCULATE(uint64, TypeCode);
        ASSIGN(uint64);
    }
        break;
    default:
        break;
    }
}

void CompareValueByType(HazeValueType Type, HazeRegister* Register, const void* Source, const void* Target)
{
    switch (Type)
    {
    case HazeValueType::Int:
    {
        VARIABLE_DEFINE_INIT(int, Source, Target);
        VARIABLE_COMPARE();
        COMPARE_ASSIGN();
    }
    break;
    case HazeValueType::Float:
    {
        VARIABLE_DEFINE_INIT(float, Source, Target);
        VARIABLE_COMPARE();
        COMPARE_ASSIGN();
    }
    break;
    case HazeValueType::Long:
    {
        VARIABLE_DEFINE_INIT(int64, Source, Target);
        VARIABLE_COMPARE();
        COMPARE_ASSIGN();
    }
    break;
    case HazeValueType::Double:
    {
        VARIABLE_DEFINE_INIT(double, Source, Target);
        VARIABLE_COMPARE();
        COMPARE_ASSIGN();
    }
    break;
    case HazeValueType::UnsignedInt:
    {
        VARIABLE_DEFINE_INIT(uint32, Source, Target);
        VARIABLE_COMPARE();
        COMPARE_ASSIGN();
    }
    break;
    case HazeValueType::UnsignedLong:
    {
        VARIABLE_DEFINE_INIT(uint64, Source, Target);
        VARIABLE_COMPARE();
        COMPARE_ASSIGN();
    }
    break;
    default:
        break;
    }
}

size_t GetHazeCharPointerLength(const HAZE_CHAR* Char)
{
    return wcslen(Char);
}

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType Type, HazeValue& Value)
{
    switch (Type)
    {
    case HazeValueType::Bool:
        return (HAZE_BINARY_CHAR*)&Value.Value.Bool;
    case HazeValueType::Int:
        return (HAZE_BINARY_CHAR*)&Value.Value.Int;
    case HazeValueType::Float:
        return (HAZE_BINARY_CHAR*)&Value.Value.Float;
    case HazeValueType::UnsignedInt:
        return (HAZE_BINARY_CHAR*)&Value.Value.UnsignedInt;
    case HazeValueType::Long:
        return (HAZE_BINARY_CHAR*)&Value.Value.Long;
    case HazeValueType::Double:
        return (HAZE_BINARY_CHAR*)&Value.Value.Double;
    case HazeValueType::UnsignedLong:
        return (HAZE_BINARY_CHAR*)&Value.Value.UnsignedLong;
    default:
        break;
    }
    return nullptr;
}
