#include <unordered_set>
#include <set>
#include "HazeValue.h"
#include "Haze.h"

uint GetSizeByHazeType(HazeValueType Type)
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
    static std::unordered_map<HazeToken, HazeValueType> MapHashTable =
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

    auto it = MapHashTable.find(Token);
    if (it != MapHashTable.end())
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

bool IsNumberType(HazeValueType Type)
{
    static std::unordered_set<HazeValueType> HashSet_Table =
    {
		HazeValueType::Int, HazeValueType::Float, HazeValueType::Long, HazeValueType::Double, 
        HazeValueType::UnsignedInt, HazeValueType::UnsignedLong,
    };

    return HashSet_Table.find(Type) != HashSet_Table.end();
}

void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValue& Value)
{
    HAZE_STRING_STREAM WSS;
    WSS << Str;

    switch (Value.Type)
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

#define CALC_VARIABLE_DEFINE_INIT(TYPE, SOURCE, TARGET) TYPE S, T; memcpy(&S, Source, sizeof(TYPE)); memcpy(&T, Target, sizeof(TYPE))
#define CALC_VARIABLE_CALCULATE(TYPE, OP) CalculateValue<TYPE>(OP, S, T)
#define CALC_ASSIGN(TYPE) memcpy((void*)Target, &T, sizeof(TYPE))

template<typename T>
void CalculateValue(InstructionOpCode TypeCode, T& Source, T& Target)
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
    }
}

void CalculateValueByType(HazeValueType Type, InstructionOpCode TypeCode, const char* Source, const char* Target)
{
    switch (Type)
    {
    case HazeValueType::Int:
    {
        CALC_VARIABLE_DEFINE_INIT(int, Source, Target);
        CALC_VARIABLE_CALCULATE(int, TypeCode);
        CALC_ASSIGN(int);
    }
        break;
    case HazeValueType::Float:
    {
        CALC_VARIABLE_DEFINE_INIT(float, Source, Target);
        CALC_VARIABLE_CALCULATE(float, TypeCode);
        CALC_ASSIGN(float);
    }
        break;
    case HazeValueType::Long:
    {
        CALC_VARIABLE_DEFINE_INIT(llong, Source, Target);
        CALC_VARIABLE_CALCULATE(llong, TypeCode);
        CALC_ASSIGN(llong);
    }
        break;
    case HazeValueType::Double:
    {
        CALC_VARIABLE_DEFINE_INIT(double, Source, Target);
        CALC_VARIABLE_CALCULATE(double, TypeCode);
        CALC_ASSIGN(double);
    }
        break;
    case HazeValueType::UnsignedInt:
    {
        CALC_VARIABLE_DEFINE_INIT(uint, Source, Target);
        CALC_VARIABLE_CALCULATE(uint, TypeCode);
        CALC_ASSIGN(uint);
    }
        break;
    case HazeValueType::UnsignedLong:
    {
        CALC_VARIABLE_DEFINE_INIT(ulong, Source, Target);
        CALC_VARIABLE_CALCULATE(ulong, TypeCode);
        CALC_ASSIGN(ulong);
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

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValue& Value)
{
    switch (Value.Type)
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
