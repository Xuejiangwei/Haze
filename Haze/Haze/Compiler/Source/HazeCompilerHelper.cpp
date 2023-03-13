#include "HazeCompilerHelper.h"

#include <fstream>
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"

void HazeCompilerStream(HAZE_STRING_STREAM& Stream, HazeCompilerValue* Value)
{
	{
		const auto& V = Value->GetValue();
		switch (V.Type)
		{
		case HazeValueType::Bool:
			Stream << V.Value.Bool;
			break;
		case HazeValueType::Short:
			Stream << V.Value.Short;
			break;
		case HazeValueType::UnsignedShort:
			Stream << V.Value.UnsignedShort;
			break;
		case HazeValueType::Int:
			Stream << V.Value.Int;
			break;
		case HazeValueType::Float:
			Stream << V.Value.Float;
			break;
		case HazeValueType::UnsignedInt:
			Stream << V.Value.UnsignedInt;
			break;
		case HazeValueType::Long:
			Stream << V.Value.Long;
			break;
		case HazeValueType::Double:
			Stream << V.Value.Double;
			break;
		case HazeValueType::UnsignedLong:
			Stream << V.Value.UnsignedLong;
			break;
		default:
			break;
		}
	}
}

void HazeCompilerOFStream(HAZE_OFSTREAM& OFStream, HazeCompilerValue* Value)
{
	const auto& V = Value->GetValue();
	switch (V.Type)
	{
	case HazeValueType::Bool:
		OFStream << V.Value.Bool;
		break;
	case HazeValueType::Short:
		OFStream << V.Value.Short;
		break;
	case HazeValueType::UnsignedShort:
		OFStream << V.Value.UnsignedShort;
		break;
	case HazeValueType::Int:
		OFStream << V.Value.Int;
		break;
	case HazeValueType::Float:
		OFStream << V.Value.Float;
		break;
	case HazeValueType::UnsignedInt:
		OFStream << V.Value.UnsignedInt;
		break;
	case HazeValueType::Long:
		OFStream << V.Value.Long;
		break;
	case HazeValueType::Double:
		OFStream << V.Value.Double;
		break;
	case HazeValueType::UnsignedLong:
		OFStream << V.Value.UnsignedLong;
		break;
	case HazeValueType::String:
		OFStream << V.Value.String.StringTableIndex;
		break;
	default:
		break;
	}
}
