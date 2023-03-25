#include <filesystem>
#include <unordered_set>

#include "HazeLog.h"
#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerClass.h"

HazeCompilerModule::HazeCompilerModule(const HAZE_STRING& ModuleName)
	: IsStdLib(false)
{

	HAZE_STRING Path = std::filesystem::current_path();

#if HAZE_I_CODE_ENABLE

	HAZE_STRING I_CodePath = Path + HAZE_TEXT("\\HazeICode\\");
	I_CodePath += ModuleName + HAZE_TEXT(".Hzic");

	FS_I_Code.imbue(std::locale("chs"));
	FS_I_Code.open(I_CodePath);

#endif
}

HazeCompilerModule::~HazeCompilerModule()
{
	if (FS_I_Code.is_open())
	{
		FS_I_Code.close();
	}
}

void HazeCompilerModule::MarkStandardLibrary()
{
	IsStdLib = true;
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
}

std::shared_ptr<HazeCompilerClass> HazeCompilerModule::CreateClass(const HAZE_STRING& Name, std::vector<HazeDefineVariable*>& ClassData)
{
	std::shared_ptr<HazeCompilerClass> Class = FindClass(Name);
	if (!Class)
	{
		HashMap_Class[Name] = std::make_shared<HazeCompilerClass>(this, Name, ClassData);
		Class = HashMap_Class[Name];

		Class->InitThisValue();
	}

	CurrClass = Name;
	return Class;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetCurrFunction()
{
	if (CurrFunction.empty())
	{
		return nullptr;
	}

	auto Iter = HashMap_Function.find(CurrFunction);
	if (Iter == HashMap_Function.end())
	{
		return FindClass(CurrClass)->FindFunction(CurrFunction);
	}

	return Iter->second;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::GetFunction(const HAZE_STRING& Name)
{
	auto It = HashMap_Function.find(Name);
	if (It != HashMap_Function.end())
	{
		return It->second;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param)
{
	auto It = HashMap_Function.find(Name);
	if (It == HashMap_Function.end())
	{
		HashMap_Function[Name] = std::make_shared<HazeCompilerFunction>(this, Name, Type, Param);
		CurrFunction = Name;

		return HashMap_Function[Name];
	}
	return It->second;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(std::shared_ptr<HazeCompilerClass> Class, HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param)
{
	std::shared_ptr<HazeCompilerFunction> Function = Class->FindFunction(Name);
	if (!Function)
	{
		Function = std::make_shared<HazeCompilerFunction>(this, Name, Type, Param, Class.get());
		Class->AddFunction(Function);

		CurrFunction = Name;
	}

	return Function;
}

void HazeCompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	CurrFunction = HAZE_TEXT("");
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::ADD);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, InstructionOpCode IO_Code)
{
	static std::unordered_set<InstructionOpCode> HashSet_NoTemp =
	{
		InstructionOpCode::MOV
	};


	std::shared_ptr<HazeCompilerValue> Ret = nullptr;
	auto Function = GetCurrFunction();

	bool NeedTemp = HashSet_NoTemp.find(IO_Code) == HashSet_NoTemp.end();

	if (CurrFunction.empty())
	{

	}
	else
	{
		HAZE_STRING_STREAM SStream;
		HAZE_STRING VarName;

		if (NeedTemp && !Left->IsTemp())
		{
			HazeDefineVariable Var;
			Var.Name = HAZE_TEXT("TempBinaryValue");
			Var.Type.Type = GetStrongerType(Left->GetValue().Type, Right->GetValue().Type);
			Ret = Function->GetTopBaseBlock()->CreateTempAlloce(Var);

			GenIRCode_BinaryOperater(Ret, Left, InstructionOpCode::MOV);

			Ret->StoreValue(Left);
		
			SStream << GetInstructionString(IO_Code) << " " << HAZE_CAST_VALUE_TYPE(Var.Type.Type) << " " << Var.Name << " " << (unsigned int)InstructionScopeType::Temp;
		}
		else
		{
			SStream << GetInstructionString(IO_Code) << " " << HAZE_CAST_VALUE_TYPE(Left->GetValue().Type) << " ";

			if (Left->IsConstant())
			{
				HazeCompilerStream(SStream, Left.get());
				SStream << " " << (unsigned int)InstructionScopeType::Constant;
			}
			/*else if (Left->IsRegister())
			{
				SStream << (unsigned int)InstructionScopeType::Register << " ";
				SStream << HazeCompiler::GetRegisterName(Left);
			}*/
			else
			{
				bool bFind = Function->GetLocalVariableName(Left, VarName);

				if (bFind)
				{
					SStream << VarName;
					SStream << " " << (unsigned int)Left->GetScope();
				}
				else
				{
					GetGlobalVariableName(Left, VarName);
					SStream << VarName;
					SStream << " " << (unsigned int)InstructionScopeType::Global;
				}
			}

		}

		SStream << " " << HAZE_CAST_VALUE_TYPE(Right->GetValue().Type) << " " ;

		if (Right->IsConstant())
		{
			HazeCompilerStream(SStream, Right.get());
			SStream << " " << (unsigned int)InstructionScopeType::Constant;
		}
		else if (Right->IsRegister())
		{
			SStream << HazeCompiler::GetRegisterName(Right);
			SStream << " " << (unsigned int)Right->GetScope();
		}
		else
		{
			bool bFind = Function->GetLocalVariableName(Right, VarName);

			if (bFind)
			{
				SStream << VarName;
				SStream << " " << (unsigned int)Right->GetScope();
			}
			else
			{
				GetGlobalVariableName(Right, VarName);
				SStream << VarName;
				SStream << " " << (unsigned int)InstructionScopeType::Global;
			}
		}

		SStream << std::endl;

		std::shared_ptr<HazeBaseBlock>& BB = Function->GetTopBaseBlock();
		BB->PushIRCode(SStream.str());
	}

	return Ret;
}

void HazeCompilerModule::GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> Value)
{
	auto Function = GetCurrFunction();
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::RET) << " " << HAZE_CAST_VALUE_TYPE(Value->GetValue().Type) << " ";

	HAZE_STRING VarName;
	if (Value->IsLocal())
	{
		Function->GetLocalVariableName(Value, VarName);
		SStream << VarName;
		SStream << " " << (unsigned int)InstructionScopeType::Local;
	}
	else if (Value->IsGlobal())
	{
		GetGlobalVariableName(Value, VarName);
		SStream << VarName;
		SStream << " " << (unsigned int)InstructionScopeType::Global;
	}
	else if (Value->IsTemp())
	{
		Function->GetLocalVariableName(Value, VarName);
		SStream << VarName;
		SStream << " " << (unsigned int)InstructionScopeType::Temp;
	}
	else if (Value->IsConstant())
	{
		HazeCompilerStream(SStream, Value.get());
		SStream << " " << (unsigned int)InstructionScopeType::Constant;
	}

	SStream << std::endl;

	std::shared_ptr<HazeBaseBlock>& BB = Function->GetTopBaseBlock();
	BB->PushIRCode(SStream.str());
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& Var)
{
	for (auto& it : Vector_Variable)
	{
		if (it.first == Var.Name)
		{
			HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("编译器错误 添加全局变量重复"));
			return nullptr;
		}
	}

	Vector_Variable.push_back({ Var.Name, CreateVariable(this, Var, InstructionScopeType::Global) });

	auto& CompilerValue = Vector_Variable.back().second;

	return CompilerValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> CallFunction, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& Param)
{
	std::shared_ptr<HazeBaseBlock>& BB = GetCurrFunction()->GetTopBaseBlock();

	HAZE_STRING_STREAM SStream;
	
	for (int i = (int)Param.size() -1; i >= 0; i--)
	{
		SStream << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CAST_VALUE_TYPE(Param[i].second->GetValue().Type) << " ";
	
		if (Param[i].second->IsConstant())
		{
			HazeCompilerStream(SStream, Param[i].second.get());
			SStream << " " << (unsigned int)InstructionScopeType::Constant;
		}
		/*else if (iter->IsRegister())
		{
			SStream << HazeCompiler::GetRegisterName(iter);
			SStream << " " << (unsigned int)InstructionScopeType::Register;
		}*/
		else
		{
			SStream << Param[i].first;

			auto Var = GetCurrFunction()->GetLocalVariable(Param[i].first); //CallFunction->GetFunctionParamNameByIndex(i, VarName);

			if (Var)
			{
				SStream << " " << (unsigned int)InstructionScopeType::Local;
			}
			else
			{
				SStream << " " << (unsigned int)InstructionScopeType::Global;
			}
		}

		SStream << std::endl;
		BB->PushIRCode(SStream.str());
		SStream.str(HAZE_TEXT(""));
	}

	SStream << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CAST_VALUE_TYPE(HazeValueType::Int) << " " << HAZE_CALL_PUSH_ADDRESS_NAME
		<< " " << (unsigned int)InstructionScopeType::Address << std::endl;

	SStream << GetInstructionString(InstructionOpCode::CALL) << " " << CallFunction->GetName() << " " << Param.size() << std::endl;
	BB->PushIRCode(SStream.str());

	return HazeCompiler::GetRegister(RET_REGISTER);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GetGlobalStringVariable(const HAZE_STRING& String)
{
	auto it = HashMap_StringTable.find(String);
	if (it != HashMap_StringTable.end())
	{
		return it->second;
	}
	HashMap_StringTable[String] = nullptr;

	it = HashMap_StringTable.find(String);

	HashMap_StringMapping[(int)HashMap_StringMapping.size()] = &it->first;

	HazeValue Value;
	//Value.Type = HazeValueType::String;
	//Value.Value.Extra.StringTableIndex = (int)HashMap_StringMapping.size() - 1;

	it->second = CreateVariable(Value, InstructionScopeType::String);
	return it->second;
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

std::shared_ptr<HazeCompilerClass> HazeCompilerModule::FindClass(const HAZE_STRING& ClassName)
{
	auto Iter = HashMap_Class.find(ClassName);
	if (Iter != HashMap_Class.end())
	{
		return Iter->second;
	}
	
	return nullptr;
}

void HazeCompilerModule::GenICode()
{
	//版本 2个字节
	//FS_Ass << "1 1" << std::endl;

	//堆栈 4个字节
	//FS_Ass << 1024 << std::endl;

	//是不是标准库
	FS_I_Code << IsStdLib << std::endl;

	/*
	*	全局数据 ：	个数 
	*				数据名 数据类型 数据			
	*/
	FS_I_Code << GetGlobalDataHeaderString() << std::endl;
	FS_I_Code << Vector_Variable.size() << std::endl;

	for (auto& iter : Vector_Variable)
	{
		FS_I_Code << iter.first << " " << HAZE_CAST_VALUE_TYPE(iter.second->GetValue().Type) << " ";
		HazeCompilerOFStream(FS_I_Code, iter.second.get());
		FS_I_Code << std::endl;
	}

	/*
	*	字符串表 ：	个数
	*				字符串长度 字符串
	*/
	if (HashMap_StringMapping.size() != HashMap_StringTable.size())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("Parse header file string table size error : mapping size %d, table size %d"),
			HashMap_StringMapping.size(), HashMap_StringTable.size());
		return;
	}
	FS_I_Code << GetStringTableHeaderString() << std::endl;
	FS_I_Code << HashMap_StringMapping.size() << std::endl;

	for (auto& iter : HashMap_StringMapping)
	{
		FS_I_Code /*<< iter.second->length() << " "*/ << *iter.second << std::endl;
	}

	/*
	*	类表 ：	个数
	*				名称 指令流
	*
	*/
	size_t FunctionSize = 0;
	
	FS_I_Code << GetClassTableHeaderString() << std::endl;
	FS_I_Code << HashMap_Class.size() << std::endl;

	for (auto& iter : HashMap_Class)
	{
		iter.second->GenClassData_I_Code(this, FS_I_Code);
		FunctionSize += iter.second->GetFunctionSize();
	}

	/*
	*	函数表 ：	个数
	*				名称 指令流
	* 
	*/
	FS_I_Code << GetFucntionTableHeaderString() << std::endl;
	FS_I_Code << HashMap_Function.size() + FunctionSize << std::endl;

	for (auto& iter : HashMap_Class)
	{
		iter.second->GenClassFunction_I_Code(this, FS_I_Code);
	}

	for (auto& iter : HashMap_Function)
	{
		iter.second->GenI_Code(this, FS_I_Code);
	}

	/*Main函数是否存在
	auto MainFuncIter = MapGlobalFunction.find(HAZE_MAIN_FUNCTION_TEXT);
	bool HasMain = MainFuncIter != MapGlobalFunction.end();
	FS_Ass << HasMain << std::endl;*/
}
