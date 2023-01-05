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
		case Bool:
			Stream << V.Value.Bool;
			break;
		case Char:
			Stream << V.Value.Char;
			break;
		case Byte:
			Stream << V.Value.Byte;
			break;
		case UnsignedByte:
			Stream << V.Value.UnsignedByte;
			break;
		case Short:
			Stream << V.Value.Short;
			break;
		case UnsignedShort:
			Stream << V.Value.UnsignedShort;
			break;
		case Int:
			Stream << V.Value.Int;
			break;
		case Float:
			Stream << V.Value.Float;
			break;
		case UnsignedInt:
			Stream << V.Value.UnsignedInt;
			break;
		case Long:
			Stream << V.Value.Long;
			break;
		case Double:
			Stream << V.Value.Double;
			break;
		case UnsignedLong:
			Stream << V.Value.UnsignedLong;
			break;
		default:
			break;
		}
	}
}

void PushAssCode(HazeCompilerModule* Module, HazeCompilerValue* Value)
{
	const auto& V = Value->GetValue();
	switch (V.Type)
	{
	case Bool:
		Module->Push(V.Value.Bool);
		break;
	case Char:
		Module->Push(V.Value.Char);
		break;
	case Byte:
		Module->Push(V.Value.Byte);
		break;
	case UnsignedByte:
		Module->Push(V.Value.UnsignedByte);
		break;
	case Short:
		Module->Push(V.Value.Short);
		break;
	case UnsignedShort:
		Module->Push(V.Value.UnsignedShort);
		break;
	case Int:
		Module->Push(V.Value.Int);
		break;
	case Float:
		Module->Push(V.Value.Float);
		break;
	case UnsignedInt:
		Module->Push(V.Value.UnsignedInt);
		break;
	case Long:
		Module->Push(V.Value.Long);
		break;
	case Double:
		Module->Push(V.Value.Double);
		break;
	case UnsignedLong:
		Module->Push(V.Value.UnsignedLong);
		break;
	default:
		break;
	}
}

void StreamAssCode(HazeCompilerModule* Module, HazeCompilerValue* Value)
{
	const auto& V = Value->GetValue();
	switch (V.Type)
	{
	case Bool:
		Module->Stream(V.Value.Bool);
		break;
	case Char:
		Module->Stream(V.Value.Char);
		break;
	case Byte:
		Module->Stream(V.Value.Byte);
		break;
	case UnsignedByte:
		Module->Stream(V.Value.UnsignedByte);
		break;
	case Short:
		Module->Stream(V.Value.Short);
		break;
	case UnsignedShort:
		Module->Stream(V.Value.UnsignedShort);
		break;
	case Int:
		Module->Stream(V.Value.Int);
		break;
	case Float:
		Module->Stream(V.Value.Float);
		break;
	case UnsignedInt:
		Module->Stream(V.Value.UnsignedInt);
		break;
	case Long:
		Module->Stream(V.Value.Long);
		break;
	case Double:
		Module->Stream(V.Value.Double);
		break;
	case UnsignedLong:
		Module->Stream(V.Value.UnsignedLong);
		break;
	default:
		break;
	}
}
