#include <unordered_set>

#include "HazeValue.h"
#include "Haze.h"

bool IsHazeDefaultType(HazeValueType Type)
{
    return HazeValueType::Void <= Type && Type <= HazeValueType::UnsignedLong;
}

bool IsNumberType(HazeValueType Type)
{
    static std::unordered_set<HazeValueType> HashSet_Table =
    {
		HazeValueType::Short, HazeValueType::Int, HazeValueType::Float, HazeValueType::Long, HazeValueType::Double, 
        HazeValueType::UnsignedShort, HazeValueType::UnsignedInt, HazeValueType::UnsignedLong,
    };

    return HashSet_Table.find(Type) != HashSet_Table.end();
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
    case HazeValueType::Short:
    {
        CALC_VARIABLE_DEFINE_INIT(short, Source, Target);
        CALC_VARIABLE_CALCULATE(short, TypeCode);
        CALC_ASSIGN(short);
    }
    break;
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
        CALC_VARIABLE_DEFINE_INIT(long, Source, Target);
        CALC_VARIABLE_CALCULATE(long, TypeCode);
        CALC_ASSIGN(long);
    }
        break;
    case HazeValueType::Double:
    {
        CALC_VARIABLE_DEFINE_INIT(double, Source, Target);
        CALC_VARIABLE_CALCULATE(double, TypeCode);
        CALC_ASSIGN(double);
    }
        break;
    case HazeValueType::UnsignedShort:
    {
        CALC_VARIABLE_DEFINE_INIT(unsigned short, Source, Target);
        CALC_VARIABLE_CALCULATE(unsigned short, TypeCode);
        CALC_ASSIGN(unsigned short);
    }
        break;
    case HazeValueType::UnsignedInt:
    {
        CALC_VARIABLE_DEFINE_INIT(unsigned int, Source, Target);
        CALC_VARIABLE_CALCULATE(unsigned int, TypeCode);
        CALC_ASSIGN(unsigned int);
    }
        break;
    case HazeValueType::UnsignedLong:
    {
        CALC_VARIABLE_DEFINE_INIT(unsigned long, Source, Target);
        CALC_VARIABLE_CALCULATE(unsigned long, TypeCode);
        CALC_ASSIGN(unsigned long);
    }
        break;
    default:
        break;
    }

   
}
