#include <filesystem>

#include "HazeLog.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"

HazeCompilerModule::HazeCompilerModule(const HAZE_STRING& ModuleName)
{
	std::wstring Path = std::filesystem::current_path();
	
#if HAZE_ASS_ENABLE

	std::wstring ASSPath = Path + HAZE_TEXT("\\HazeASS\\");
	ASSPath += ModuleName + HAZE_TEXT(".Hza");

	FS_Ass.imbue(std::locale("chs"));
	FS_Ass.open(ASSPath);

#endif

#if HAZE_OP_CODE_ENABLE

	std::wstring OpCodePath = Path + HAZE_TEXT("\\HazeOpCode\\");
	OpCodePath += ModuleName + HAZE_TEXT(".Hzb");

	FS_OpCode.imbue(std::locale("chs"));
	FS_OpCode.open(OpCodePath);

#endif

	GlobalVariableSize = 0;
	StackTop = 0;
}

HazeCompilerModule::~HazeCompilerModule()
{
	FS_OpCode << "1 1 1 1" << std::endl;

	if (FS_Ass.is_open())
	{
		FS_Ass.close();
	}

	if (FS_OpCode.is_open())
	{
		FS_OpCode.close();
	}
}

void HazeCompilerModule::GenCodeFile()
{
	//根据一定格式生成所有的字节码
#if HAZE_ASS_ENABLE

	if (FS_Ass.is_open())
	{
		GenASS_FileHeader();
		GenASS_Instruction();
		GenASS_GlobalData();

		FS_Ass.close();
	}
#endif

#if HAZE_OP_CODE_ENABLE
	
	if (FS_OpCode.is_open())
	{
		FS_OpCode.close();
	}

#endif // HAZE_OP_CODE_ENABLE
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

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& Var)
{
	auto It = MapGlobalVariable.find(Var.second);
	if (It != MapGlobalVariable.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加全局变量重复"));
		return nullptr;
	}

	MapGlobalVariable[Var.second] = std::make_shared<HazeCompilerValue>(this, Var.first);

	auto CompilerValue = MapGlobalVariable[Var.second];

	GlobalVariableSize += GetSize(CompilerValue->GetValue().Type);

	return CompilerValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param)
{
#if HAZE_ASS_ENABLE
	if (FS_Ass.is_open())
	{
		int EBP = StackTop;
		for (auto& it : Param)
		{
			PushAssCode(this, it.get());
		}

		FS_Ass << "Push " << EBP << std::endl;
	}
#endif

#if HAZE_OP_CODE_ENABLE

	if (FS_OpCode.is_open())
	{
		FS_OpCode.close();
	}

#endif

	return std::shared_ptr<HazeCompilerValue>();
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::AddGlobalStringVariable(const HazeDefineVariable& Var)
{
	return nullptr;
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

void HazeCompilerModule::GenASS_FileHeader()
{
	//版本 2个字节
	FS_Ass << "1 1" << std::endl;

	//堆栈 4个字节
	FS_Ass << 1024 << std::endl;

	//全局数据大小 4个字节
	FS_Ass << GlobalVariableSize << std::endl;

	//Main函数是否存在
	auto MainFuncIter = MapGlobalFunction.find(HAZE_MAIN_FUNCTION_TEXT);
	bool HasMain = MainFuncIter != MapGlobalFunction.end();
	FS_Ass << HasMain << std::endl;
}

void HazeCompilerModule::GenASS_Instruction()
{
	for (auto& it : MapGlobalFunction)
	{
		it.second->GenASSCode(this);
	}
}

void HazeCompilerModule::GenASS_GlobalData()
{
	for (auto& it : MapGlobalVariable)
	{
		PushAssCode(this, it.second.get());
	}
}
