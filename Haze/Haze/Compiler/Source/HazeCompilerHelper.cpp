#include "HazeCompilerHelper.h"

#include <fstream>
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"

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