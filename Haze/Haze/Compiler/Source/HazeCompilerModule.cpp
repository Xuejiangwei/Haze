#include <filesystem>

#include "HazeLog.h"
#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"

HazeCompilerModule::HazeCompilerModule(const HAZE_STRING& ModuleName)
{
	HAZE_STRING Path = std::filesystem::current_path();
	
#if HAZE_I_CODE_ENABLE

	HAZE_STRING I_CodePath = Path + HAZE_TEXT("\\HazeICode\\");
	I_CodePath += ModuleName + HAZE_TEXT(".Hzic");

	FS_I_Code.imbue(std::locale("chs"));
	FS_I_Code.open(I_CodePath);

#endif

#if HAZE_OP_CODE_ENABLE

	HAZE_STRING OpCodePath = Path + HAZE_TEXT("\\HazeOpCode\\");
	OpCodePath += ModuleName + HAZE_TEXT(".Hzb");

	FS_OpCode.imbue(std::locale("chs"));
	FS_OpCode.open(OpCodePath);

#endif
	StackTop = 0;
}

HazeCompilerModule::~HazeCompilerModule()
{
	FS_OpCode << "1 1 1 1" << std::endl;

	if (FS_I_Code.is_open())
	{
		FS_I_Code.close();
	}

	if (FS_OpCode.is_open())
	{
		FS_OpCode.close();
	}
}

void HazeCompilerModule::GenCodeFile()
{
#if HAZE_I_CODE_ENABLE

	//生成中间代码先不需要计算symbol table表中的偏移，等统一生成字节码时在进行替换
	if (FS_I_Code.is_open())
	{
		GenICode();
		FS_I_Code.close();
	}

#endif

	//字节码应统一生成与替换，此处需要修改
#if HAZE_OP_CODE_ENABLE
	
	if (FS_OpCode.is_open())
	{
		FS_OpCode.close();
	}

#endif // HAZE_OP_CODE_ENABLE
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetCurrFunction()
{
	if (CurrFunction.empty())
	{
		return nullptr;
	}
	return Map_Function[CurrFunction];
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetFunction(const HAZE_STRING& Name)
{
	auto It = Map_Function.find(Name);
	if (It != Map_Function.end())
	{
		return It->second;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
{
	auto It = Map_Function.find(Name);
	if (It == Map_Function.end())
	{
		Map_Function[Name] = std::make_shared<HazeCompilerFunction>(this, Name, Type, Param);
		CurrFunction = Name;

		return Map_Function[Name];
	}
	return It->second;
}

void HazeCompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	CurrFunction = HAZE_TEXT("");
}

void HazeCompilerModule::GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, InstructionOpCode IO_Code)
{
	auto Function = GetCurrFunction();

	if (CurrFunction.empty())
	{

	}
	else
	{
		HAZE_STRING_STREAM SStream;
		SStream << HazeCompiler::GetInstructionOpName(IO_Code) << " ";

		HAZE_STRING VarName;

		if (Left->IsConstant())
		{
			SStream << (unsigned int)InstructionDataType::Constant << " ";
			HazeCompilerStream(SStream, Left.get());
		}
		else if (Left->IsRegister())
		{
			SStream << (unsigned int)InstructionDataType::Register << " ";
			SStream << HazeCompiler::GetRegisterName(Left);
		}
		else
		{
			bool bFind = Function->GetLocalVariableName(Left, VarName);

			if (bFind)
			{
				SStream << (unsigned int)InstructionDataType::Local << " ";
				SStream << VarName;
			}
			else
			{
				GetGlobalVariableName(Left, VarName);
				SStream << (unsigned int)InstructionDataType::Global << " ";
				SStream << VarName;
			}
		}

		SStream << " ";

		if (Right->IsConstant())
		{
			SStream << (unsigned int)InstructionDataType::Constant << " ";
			HazeCompilerStream(SStream, Right.get());
		}
		else if (Right->IsRegister())
		{
			SStream << (unsigned int)InstructionDataType::Register << " ";
			SStream << HazeCompiler::GetRegisterName(Right);
		}
		else
		{
			bool bFind = Function->GetLocalVariableName(Right, VarName);

			if (bFind)
			{
				SStream << (unsigned int)InstructionDataType::Local << " ";
				SStream << VarName;
			}
			else
			{
				GetGlobalVariableName(Right, VarName);
				SStream << (unsigned int)InstructionDataType::Global << " ";
				SStream << VarName;
			}
		}

		SStream << std::endl;

		std::shared_ptr<HazeBaseBlock>& BB = Function->GetTopBaseBlock();
		BB->PushIRCode(SStream.str());
	}
}

void HazeCompilerModule::GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> Value)
{
	auto Function = GetCurrFunction();
	HAZE_STRING_STREAM SStream;
	SStream << HAZE_TEXT("Ret ");

	HAZE_STRING VarName;
	if (Value->IsLocal())
	{
		SStream << (unsigned int)InstructionDataType::Local << " ";
		Function->GetLocalVariableName(Value, VarName);

		SStream << VarName;
	}
	else if (Value->IsGlobal())
	{
		SStream << (unsigned int)InstructionDataType::Global << " ";
		GetGlobalVariableName(Value, VarName);
		SStream << VarName;
	}
	else if (Value->IsRegister())
	{
		SStream << (unsigned int)InstructionDataType::Register << " ";
		VarName = HazeCompiler::GetRegisterName(Value);
		SStream << VarName;
	}
	else if (Value->IsConstant())
	{
		SStream << (unsigned int)InstructionDataType::Constant << " ";
		HazeCompilerStream(SStream, Value.get());
	}

	SStream << std::endl;

	std::shared_ptr<HazeBaseBlock>& BB = Function->GetTopBaseBlock();
	BB->PushIRCode(SStream.str());
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& Var)
{
	for (auto& it : Vector_Variable)
	{
		if (it.first == Var.second)
		{
			HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加全局变量重复"));
			return nullptr;
		}
	}

	Vector_Variable.push_back({ Var.second, std::make_shared<HazeCompilerValue>(this, Var.first, HazeCompilerValue::ValueSection::Global) });

	auto& CompilerValue = Vector_Variable.back().second;

	return CompilerValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> CallFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param)
{

	std::shared_ptr<HazeBaseBlock>& BB = GetCurrFunction()->GetTopBaseBlock();

	HAZE_STRING VarName;
	
	for (auto& iter : Param)
	{
		if (iter->IsLocal())
		{
			CallFunction->GetLocalVariableName(iter, VarName);
		}
		else if (iter->IsGlobal())
		{
			GetGlobalVariableName(iter, VarName);
		}
		else if (iter->IsRegister())
		{
			VarName = HazeCompiler::GetRegisterName(iter);
		}
		else if (iter->IsConstant())
		{
			HAZE_STRING_STREAM HSS;
			HazeCompilerStream(HSS, iter.get());
			VarName = HSS.str();
		}

		VarName = HAZE_STRING(HAZE_TEXT("Push ")) + VarName + HAZE_TEXT("\n");
		BB->PushIRCode(VarName);
	}

	BB->PushIRCode(HAZE_STRING(HAZE_TEXT("Call ")) + CallFunction->GetName() + HAZE_TEXT("\n"));

	return HazeCompiler::GetReturnRegister();
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::AddGlobalStringVariable(const HazeDefineVariable& Var)
{
	return nullptr;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GetGlobalVariable(const HAZE_STRING& Name)
{
	for (auto& it : Vector_Variable)
	{
		if (it.first == Name)
		{
			return it.second;
		}
	}

	return nullptr;
}

bool HazeCompilerModule::GetGlobalVariableName(std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	for (auto& it : Vector_Variable)
	{
		if (it.second == Value)
		{
			OutName = it.first;
			return true;
		}
	}

	return false;
}

HazeValueType HazeCompilerModule::FindClass(const HAZE_STRING& ClassName)
{
	return HazeValueType::Class;
}

void HazeCompilerModule::GenICode()
{
	//版本 2个字节
	//FS_Ass << "1 1" << std::endl;

	//堆栈 4个字节
	//FS_Ass << 1024 << std::endl;

	/*
	*	全局数据 ：	个数 
	*				数据名 数据类型 数据			
	*/
	FS_I_Code << GetGlobalDataHeaderString() << std::endl;
	FS_I_Code << Vector_Variable.size() << std::endl;

	for (auto& iter : Vector_Variable)
	{
		FS_I_Code << iter.first << " " << iter.second->GetValue().Type << " ";
		HazeCompilerOFStream(FS_I_Code, iter.second.get());
		FS_I_Code << std::endl;
	}

	/*
	*	字符串表 ：	个数
	*				字符串长度 字符串
	*/
	FS_I_Code << GetStringTableHeaderString() << std::endl;
	FS_I_Code << Map_StringVariable.size() << std::endl;

	for (auto& iter : Map_StringVariable)
	{
		FS_I_Code << iter.first.length() << " " << iter.first << std::endl;
	}

	/*
	*	函数表 ：	个数
	*				名称 指令流
	* 
	*/
	FS_I_Code << GetFucntionTableHeaderString() << std::endl;
	FS_I_Code << Map_Function.size() << std::endl;

	for (auto& iter : Map_Function)
	{
		iter.second->GenI_Code(this, FS_I_Code);
	}

	/*Main函数是否存在
	auto MainFuncIter = MapGlobalFunction.find(HAZE_MAIN_FUNCTION_TEXT);
	bool HasMain = MainFuncIter != MapGlobalFunction.end();
	FS_Ass << HasMain << std::endl;*/
}
