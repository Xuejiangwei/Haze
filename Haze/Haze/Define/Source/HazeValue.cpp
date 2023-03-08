#include <unordered_set>

#include "HazeValue.h"

bool IsNumberType(HazeValueType Type)
{
    static std::unordered_set<HazeValueType> HashSet_Table =
    {
		HazeValueType::Short, HazeValueType::Int, HazeValueType::Float, HazeValueType::Long, HazeValueType::Double, 
        HazeValueType::UnsignedShort, HazeValueType::UnsignedInt, HazeValueType::UnsignedLong,
    };

    return HashSet_Table.find(Type) != HashSet_Table.end();
}

void AddValueByType(HazeValueType Type, const char* Source, const char* Target)
{
    switch (Type)
    {
    case HazeValueType::Char:
        break;
    case HazeValueType::Short:
        break;
    case HazeValueType::Int:
    {
        int S;
        int T;
        memcpy(&S, Source, sizeof(int));
        memcpy(&T, Target, sizeof(int));
        T += S;
        memcpy((void*)Target, &T, sizeof(int));
    }
        break;
    case HazeValueType::Float:
        //Target += (float)&Source;
        break;
    case HazeValueType::Long:
        break;
    case HazeValueType::Double:
        break;
    case HazeValueType::UnsignedChar:
        break;
    case HazeValueType::UnsignedShort:
        break;
    case HazeValueType::UnsignedInt:
        break;
    case HazeValueType::UnsignedLong:
        break;
    case HazeValueType::Class:
        break;
    case HazeValueType::Function:
        break;
    default:
        break;
    }
}
