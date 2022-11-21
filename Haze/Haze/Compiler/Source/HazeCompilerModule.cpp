#include <filesystem>

#include "HazeLog.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"

HazeCompilerModule::HazeCompilerModule(const HAZE_STRING& ModuleName)
{
	std::wstring Path = std::filesystem::current_path();
	Path += HAZE_TEXT("\\HazeOpCodeFile\\");
	Path += ModuleName + HAZE_TEXT(".Hzb");

	FS.imbue(std::locale("chs"));
	FS.open(Path);
}

HazeCompilerModule::~HazeCompilerModule()
{
	if (FS.is_open())
	{
		FS.close();
	}
}

void HazeCompilerModule::GenBinaryFile()
{
	//根据一定格式生成所有的字节码

	if (FS.is_open())
	{
		FS.close();
	}
}

HazeCompilerValue* HazeCompilerModule::AddGlobalVariable(const HAZE_STRING& Name, HazeValueType Type)
{
	auto It = MapGlobalVariable.find(Name);
	if (It != MapGlobalVariable.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加全局变量重复"));
		return nullptr;
	}

	MapGlobalVariable[Name] = HazeCompilerValue(this, Type);

	return &MapGlobalVariable[Name];
}

HazeCompilerValue* HazeCompilerModule::AddLocalVariable()
{
	return CurrFunction->AddLocalVariable(this);
}

HazeCompilerValue* HazeCompilerModule::AddDataVariable(HazeValue& Value)
{
	switch (Value.Type)
	{
	case HazeValueType::Byte:
	case HazeValueType::Short:
	case HazeValueType::Int:
	case HazeValueType::Long:
	case HazeValueType::UnsignedByte:
	case HazeValueType::UnsignedShort:
	case HazeValueType::UnsignedInt:
	case HazeValueType::UnsignedLong:
	{
		auto It = MapIntDataValue.find(Value.Value.LongValue);
		if (It == MapIntDataValue.end())
		{
			MapIntDataValue[Value.Value.LongValue] = HazeCompilerValue(this, Value);
		}

		return &MapIntDataValue[Value.Value.LongValue];
	}
	break;
	case HazeValueType::Float:
	case HazeValueType::Double:
	{
		auto It = MapFloatDataValue.find(Value.Value.DoubleValue);
		if (It == MapFloatDataValue.end())
		{
			MapFloatDataValue[Value.Value.DoubleValue] = HazeCompilerValue(this, Value);
		}

		return &MapFloatDataValue[Value.Value.DoubleValue];
	}
	break;
	case HazeValueType::Bool:
	{
		auto It = MapBoolDataValue.find(Value.Value.BoolValue);
		if (It == MapBoolDataValue.end())
		{
			MapBoolDataValue[Value.Value.DoubleValue] = HazeCompilerValue(this, Value);
		}

		return &MapBoolDataValue[Value.Value.DoubleValue];
	}
	break;
	default:
		break;
	}

	return nullptr;
}