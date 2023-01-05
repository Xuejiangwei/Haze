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
	//����һ����ʽ�������е��ֽ���
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

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetFunction(const HAZE_STRING& Name)
{
	auto It = MapGlobalFunction.find(Name);
	if (It != MapGlobalFunction.end())
	{
		return It->second;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
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

void HazeCompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	CurrFunction = HAZE_TEXT("");
}

void HazeCompilerModule::GenASS_Label(HAZE_STRING& Label)
{
	FS_Ass << HAZE_TEXT("label_") << Label << std::endl;
}

void HazeCompilerModule::GenASS_Add(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	if (CurrFunction.empty())
	{

	}
	else
	{
		HAZE_STRING_STREAM SStream;
		SStream << HAZE_TEXT("Add ");

		if (Left->IsConstant())
		{
			HazeCompilerStream(SStream, Left.get());
		}
		else
		{
			SStream << Left->GetAddress() << HAZE_TEXT(" ");
		}

		if (Right->IsConstant())
		{
			HazeCompilerStream(SStream, Right.get());
		}
		else
		{
			SStream << Right->GetAddress() << HAZE_TEXT("\n");
		}

		GetCurrFunction()->FunctionASSCode += SStream.str();
	}
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& Var)
{
	for (auto& it : VectorGlobalVariable)
	{
		if (it.first == Var.second)
		{
			HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("���������� ���ȫ�ֱ����ظ�"));
			return nullptr;
		}
	}

	VectorGlobalVariable.push_back({ Var.second, std::make_shared<HazeCompilerValue>(this, Var.first, HazeCompilerValue::ValueSection::Global, (int)VectorGlobalVariable.size()) });

	auto& CompilerValue = VectorGlobalVariable.back().second;

	GlobalVariableSize += GetSize(CompilerValue->GetValue().Type);

	return CompilerValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> Function, std::vector<std::shared_ptr<HazeCompilerValue>>& Param)
{
#if HAZE_ASS_ENABLE
	if (FS_Ass.is_open())
	{
		int EBP = StackTop;
		
		//Push���в��������ҵ���
		for (auto& it : Param)
		{
			PushAssCode(this, it.get());
		}

		//Push ���ص�ַ
		FS_Ass << "Push " << EBP << std::endl;

		//Jmp ��ת������
		FS_Ass << "Jmp " << Function->GetName() << std::endl;
	}
#endif

#if HAZE_OP_CODE_ENABLE

	if (FS_OpCode.is_open())
	{
		FS_OpCode.close();
	}

#endif

	return Function->GetReturnValue();
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::AddGlobalStringVariable(const HazeDefineVariable& Var)
{
	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GetGlobalVariable(const HAZE_STRING& Name)
{
	for (auto& it : VectorGlobalVariable)
	{
		if (it.first == Name)
		{
			return it.second;
		}
	}

	return nullptr;
}

HazeValueType HazeCompilerModule::FindClass(const HAZE_STRING& ClassName)
{
	return HazeValueType::Class;
}

void HazeCompilerModule::GenASS_FileHeader()
{
	//�汾 2���ֽ�
	FS_Ass << "1 1" << std::endl;

	//��ջ 4���ֽ�
	FS_Ass << 1024 << std::endl;

	//ȫ�����ݴ�С 4���ֽ�
	FS_Ass << GlobalVariableSize << std::endl;

	//Main�����Ƿ����
	auto MainFuncIter = MapGlobalFunction.find(HAZE_MAIN_FUNCTION_TEXT);
	bool HasMain = MainFuncIter != MapGlobalFunction.end();
	FS_Ass << HasMain << std::endl;
}

void HazeCompilerModule::GenASS_Instruction()
{
	for (auto& it : MapGlobalFunction)
	{
		it.second->GenCode(this);
	}
}

void HazeCompilerModule::GenASS_GlobalData()
{
	for (auto& it : VectorGlobalVariable)
	{
		PushAssCode(this, it.second.get());
	}
}
