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
	FS << "1 1 1 1" << std::endl;
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

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetCurrFunction()
{
	return MapGlobalFunction[CurrFunction];
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetFunction(HAZE_STRING& Name)
{
	auto It = MapGlobalFunction.find(Name);
	if (It != MapGlobalFunction.end())
	{
		return It->second;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::AddFunction(HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
{
	auto It = MapGlobalFunction.find(Name);
	if (It == MapGlobalFunction.end())
	{
		MapGlobalFunction[Name] = std::make_shared<HazeCompilerFunction>(this, Name, Type, Param);
		CurrFunction = Name;
		return MapGlobalFunction[Name];
	}
	return It->second;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::AddGlobalVariable(const HazeDefineVariable& Var)
{
	auto It = MapGlobalVariable.find(Var.second);
	if (It != MapGlobalVariable.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加全局变量重复"));
		return nullptr;
	}

	MapGlobalVariable[Var.second] = std::make_shared<HazeCompilerValue>(this, Var.first);

	return MapGlobalVariable[Var.second];
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::AddDataVariable(HazeValue& Value)
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
		auto It = MapIntDataValue.find(Value.Value.Long);
		if (It == MapIntDataValue.end())
		{
			MapIntDataValue[Value.Value.Long] = std::make_shared<HazeCompilerValue>(this, Value);
		}

		return MapIntDataValue[Value.Value.Long];
	}
	break;
	case HazeValueType::Float:
	case HazeValueType::Double:
	{
		auto It = MapFloatDataValue.find(Value.Value.Double);
		if (It == MapFloatDataValue.end())
		{
			MapFloatDataValue[Value.Value.Double] = std::make_shared<HazeCompilerValue>(this, Value);
		}

		return MapFloatDataValue[Value.Value.Double];
	}
	break;
	case HazeValueType::Bool:
	{
		auto It = MapBoolDataValue.find(Value.Value.Bool);
		if (It == MapBoolDataValue.end())
		{
			MapBoolDataValue[Value.Value.Double] = std::make_shared<HazeCompilerValue>(this, Value);
		}

		return MapBoolDataValue[Value.Value.Double];
	}
	break;
	default:
		break;
	}

	return nullptr;
}

HazeValueType HazeCompilerModule::FindClass(const HAZE_STRING& ClassName)
{
	return HazeValueType::Class;
}