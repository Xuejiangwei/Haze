#include <filesystem>
#include <unordered_set>

#include "HazeLog.h"
#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerInitlistValue.h"
#include "HazeCompilerFunction.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerClass.h"

HazeCompilerModule::HazeCompilerModule(HazeCompiler* Compiler, const HAZE_STRING& ModuleName)
	: Compiler(Compiler), IsStdLib(false)
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

std::shared_ptr<HazeCompilerClass> HazeCompilerModule::CreateClass(const HAZE_STRING& Name, std::vector<std::pair<HazeDataDesc, std::vector<HazeDefineVariable*>>>& ClassData)
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
	else
	{
		bool IsPointer;
		return GetObjectFunction(this, Name, IsPointer);
	}
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
{
	auto It = HashMap_Function.find(Name);
	if (It == HashMap_Function.end())
	{
		HashMap_Function[Name] = std::make_shared<HazeCompilerFunction>(this, Name, Type, Param);
		CurrFunction = Name;

		HashMap_Function[Name]->InitEntryBlock(HazeBaseBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, HashMap_Function[Name], nullptr));

		return HashMap_Function[Name];
	}
	return It->second;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(std::shared_ptr<HazeCompilerClass> Class, const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
{
	std::shared_ptr<HazeCompilerFunction> Function = Class->FindFunction(Name);
	if (!Function)
	{
		Function = std::make_shared<HazeCompilerFunction>(this, GetHazeClassFunctionName(Class->GetName(), Name), Type, Param, Class.get());
		Class->AddFunction(Function);

		Function->InitEntryBlock(HazeBaseBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, Function, nullptr));

		CurrFunction = Name;
	}

	return Function;
}

void HazeCompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	CurrFunction = HAZE_TEXT("");
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateAdd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::ADD_ASSIGN : InstructionOpCode::ADD);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateSub(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::SUB_ASSIGN : InstructionOpCode::SUB);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateMul(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::MUL_ASSIGN : InstructionOpCode::MUL);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateDiv(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::DIV_ASSIGN : InstructionOpCode::DIV);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateMod(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::MOD_ASSIGN : InstructionOpCode::MOD);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::BIT_AND_ASSIGN : InstructionOpCode::BIT_AND);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::BIT_OR_ASSIGN : InstructionOpCode::BIT_OR);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitXor(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::BIT_XOR_ASSIGN : InstructionOpCode::BIT_XOR);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateShl(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::SHL_ASSIGN : InstructionOpCode::SHL);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateShr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, bool IsAssign)
{
	return GenIRCode_BinaryOperater(Left, Right, IsAssign ? InstructionOpCode::SHR_ASSIGN : InstructionOpCode::SHR);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateAnd(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::AND);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateOr(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::OR);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateNot(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::NOT);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateInc(std::shared_ptr<HazeCompilerValue> Value, bool IsPreInc)
{
	std::shared_ptr<HazeCompilerValue> Ret = Value;
	if (IsHazeDefaultType(Value->GetValue().Type))
	{
		if (IsPreInc)
		{
			Ret = Compiler->GetTempRegister();
			Compiler->CreateMov(Ret, Value);
		}
		Compiler->CreateMov(Value, CreateAdd(Value, Compiler->GetConstantValueInt_1()));
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Create inc compiler error\n"));
	}
	return Ret;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateDec(std::shared_ptr<HazeCompilerValue> Value, bool IsPreDec)
{
	std::shared_ptr<HazeCompilerValue> Ret = Value;
	if (IsHazeDefaultType(Value->GetValue().Type))
	{
		if (IsPreDec)
		{
			Ret = Compiler->GetTempRegister();
			Compiler->CreateMov(Ret, Value);
		}
		Compiler->CreateMov(Value, CreateSub(Value, Compiler->GetConstantValueInt_1()));
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Create dec compiler error\n"));
	}
	return Ret;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateOpAssign(HazeOperatorAssign Type, std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	if (IsHazeDefaultType(Left->GetValue().Type))
	{
		switch (Type)
		{
		case HazeOperatorAssign::None:
			HAZE_LOG_ERR(HAZE_TEXT("Create OpAssign type none error\n"));
			break;
		case HazeOperatorAssign::AddAssign:
			CreateAdd(Left, Right);
			break;
		case HazeOperatorAssign::SubAssign:
			break;
		case HazeOperatorAssign::MulAssign:
			break;
		case HazeOperatorAssign::DivAssign:
			break;
		case HazeOperatorAssign::ModAssign:
			break;
		case HazeOperatorAssign::BitAndAssign:
			break;
		case HazeOperatorAssign::BitOrAssign:
			break;
		case HazeOperatorAssign::BitXorAssign:
			break;
		case HazeOperatorAssign::ShlAssign:
			break;
		case HazeOperatorAssign::ShrAssign:
			break;
		default:
			break;
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Create OpAssign is not haze default type error\n"));
	}

	return Left;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateArrayInit(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> InitList)
{
	auto ArrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(Array);
	auto InitListValue = std::dynamic_pointer_cast<HazeCompilerInitListValue>(InitList);

	HAZE_STRING_STREAM SStream;
	HAZE_STRING VarName;

	if (IsHazeDefaultType(ArrayValue->GetArrayType().SecondaryType))
	{
		for (size_t i = 0; i < ArrayValue->GetArrayLength(); i++)
		{
			SStream << GetInstructionString(InstructionOpCode::MOV) << " ";

			GenValueHzicText(this, SStream, Array, (int)i);

			SStream << " ";
			GenValueHzicText(this, SStream, InitListValue->GetList()[i]);

			SStream << std::endl;

			std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
			BB->PushIRCode(SStream.str());
			SStream.str(HAZE_TEXT(""));
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Array only haze default type!\n"));
	}

	return Array;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, InstructionOpCode IO_Code)
{
	static std::unordered_set<InstructionOpCode> HashSet_NoTemp =
	{
		InstructionOpCode::MOV,
		InstructionOpCode::CMP,
		InstructionOpCode::ADD_ASSIGN,
		InstructionOpCode::SUB_ASSIGN,
		InstructionOpCode::MUL_ASSIGN,
		InstructionOpCode::DIV_ASSIGN,
		InstructionOpCode::MOD_ASSIGN,
		InstructionOpCode::BIT_AND_ASSIGN,
		InstructionOpCode::BIT_OR_ASSIGN,
		InstructionOpCode::BIT_XOR_ASSIGN,
		InstructionOpCode::SHL_ASSIGN,
		InstructionOpCode::SHR_ASSIGN,
	};

	std::shared_ptr<HazeCompilerValue> Ret = Left;
	auto Function = GetCurrFunction();

	bool NeedTemp = HashSet_NoTemp.find(IO_Code) == HashSet_NoTemp.end();

	if (CurrFunction.empty())
	{

	}
	else
	{
		HAZE_STRING_STREAM SStream;

		if (NeedTemp)
		{
			if (!Left->IsRegister(HazeDataDesc::RegisterTemp) && !Right->IsRegister(HazeDataDesc::RegisterTemp))
			{
				Ret = Compiler->CreateMov(Compiler->GetTempRegister(), Left);
			}
			else
			{
				if (Right->IsRegister(HazeDataDesc::RegisterTemp))
				{
					Ret = Right;
					Right = Left;
				}
			}
		}

		SStream << GetInstructionString(IO_Code) << " ";
		GenValueHzicText(this, SStream, Ret);

		SStream << " ";
		GenValueHzicText(this, SStream, Right);

		SStream << std::endl;

		std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
		BB->PushIRCode(SStream.str());
	}

	return Ret;
}

void HazeCompilerModule::GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> Value)
{
	auto Function = GetCurrFunction();
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::RET) << " ";
	GenValueHzicText(this, SStream, Value);

	SStream << std::endl;

	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	BB->PushIRCode(SStream.str());
}

void HazeCompilerModule::GenIRCode_Cmp(HazeCmpType CmpType, std::shared_ptr<HazeBaseBlock> IfJmpBlock, std::shared_ptr<HazeBaseBlock> ElseJmpBlock, bool IfNullJmpOut, bool ElseNullJmpOut)
{
	HAZE_STRING_STREAM SStream;

	if (CmpType == HazeCmpType::None)
	{
		HAZE_LOG_ERR(HAZE_TEXT("cmp type is none %s\n"));
	}

	SStream << GetInstructionString(GetInstructionOpCodeByCmpType(CmpType)) << " ";
	
	if (IfJmpBlock)
	{
		SStream << IfJmpBlock->GetName() << " ";
	}
	else
	{
		if (IfNullJmpOut)
		{
			SStream << HAZE_JMP_OUT << " ";
		}
		else
		{
			SStream << HAZE_JMP_NULL << " ";
		}
	}

	if (ElseJmpBlock)
	{
		SStream << ElseJmpBlock->GetName();
	}
	else
	{
		if (ElseNullJmpOut)
		{
			SStream << HAZE_JMP_OUT << " ";
		}
		else
		{
			SStream << HAZE_JMP_NULL << " ";
		}
	}

	SStream << std::endl;

	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	BB->PushIRCode(SStream.str());
}

void HazeCompilerModule::GenIRCode_JmpFrom(std::shared_ptr<HazeBaseBlock> FormBlock, std::shared_ptr<HazeBaseBlock> ToBlock, bool IsJmpL)
{
	HAZE_STRING_STREAM HSS;

	if (IsJmpL)
	{
		HSS << GetInstructionString(InstructionOpCode::JMPL);
	}
	else
	{
		HSS << GetInstructionString(InstructionOpCode::JMP);
	}

	HSS << " " << ToBlock->GetName() << std::endl;

	FormBlock->PushIRCode(HSS.str());
}

void HazeCompilerModule::GenIRCode_JmpTo(std::shared_ptr<HazeBaseBlock> Block, bool IsJmpL)
{
	auto TopBlock = Compiler->GetInsertBlock();
	HAZE_STRING_STREAM HSS;

	if (IsJmpL)
	{
		HSS << GetInstructionString(InstructionOpCode::JMPL);
	}
	else
	{
		HSS << GetInstructionString(InstructionOpCode::JMP);
	}

	HSS << " " << Block->GetName() << std::endl;
	
	TopBlock->PushIRCode(HSS.str());
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> ArraySize)
{
	for (auto& it : Vector_Variable)
	{
		if (it.first == Var.Name)
		{
			HAZE_LOG_ERR(HAZE_TEXT("编译器错误 添加全局变量重复\n"));
			return nullptr;
		}
	}

	Vector_Variable.push_back({ Var.Name, CreateVariable(this, Var, HazeDataDesc::Global, 0, ArraySize) });

	auto& CompilerValue = Vector_Variable.back().second;

	return CompilerValue;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> CallFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo)
{
	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();

	HAZE_STRING_STREAM SStream;
	
	uint32 Size = 0;

	HAZE_STRING Name;

	for (int i = (int)Param.size() -1; i >= 0; i--)
	{
		SStream << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CAST_VALUE_TYPE(Param[i]->GetValue().Type) << " ";
	
		if (!GetCurrFunction()->FindLocalVariableName(Param[i], Name))
		{
			if (!GetGlobalVariableName(Param[i], Name))
			{
				HAZE_LOG_ERR(HAZE_TEXT("Haze parse function call not find variable name\n"));
			}
		}

		if (Param[i]->IsConstant())
		{
			HazeCompilerStream(SStream, Param[i].get());
			SStream << " " << (uint32)Param[i]->GetScope();
		}
		else if (Param[i]->IsString())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Param[i]);
			SStream << HAZE_CONSTANT_STRING_NAME << " " << (uint32)Param[i]->GetScope() << " " << (uint32)PointerValue->GetPointerType().SecondaryType
				<< " " << Param[i]->GetValue().Value.Extra.StringTableIndex;
		}
		else if (Param[i]->IsPointerBase())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Param[i]);
			SStream << Name << " " << (uint32)Param[i]->GetScope() << " " << (uint32)PointerValue->GetPointerType().SecondaryType;
		}
		else if (Param[i]->IsPointerClass())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Param[i]);
			SStream << Name << " " << (uint32)Param[i]->GetScope() << " " << PointerValue->GetPointerType().CustomName;
		}
		else if (Param[i]->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Param[i]);
			SStream << Name << " " << (uint32)Param[i]->GetScope() << " " << ClassValue->GetOwnerClassName();
		}
		else if (Param[i]->IsArrayElement())
		{
			auto ArrayElementValue = std::dynamic_pointer_cast<HazeCompilerArrayElementValue>(Param[i]);
			SStream << Name << " " << (uint32)Param[i]->GetScope() << " ";
			if (ArrayElementValue->GetIndex()->IsConstant())
			{
				HazeCompilerStream(SStream, ArrayElementValue->GetIndex());
			}
			else
			{
				HAZE_STRING VarName;
				if (GetCurrFunction())
				{
					if (GetCurrFunction()->FindLocalVariableName(ArrayElementValue->GetIndex(), VarName))
					{
						SStream << VarName;
					}
					else if (GetGlobalVariableName(ArrayElementValue->GetIndex(), Name))
					{
						SStream << VarName;
					}
					else
					{
						HAZE_LOG_ERR(HAZE_TEXT("Function call param array element not find\n"));
					}
				}

			}
		}
		else
		{
			SStream << Name << " " << (uint32)Param[i]->GetScope();
		}

		SStream << std::endl;
		BB->PushIRCode(SStream.str());
		SStream.str(HAZE_TEXT(""));

		Size += Param[i]->GetSize();
		Name.clear();
	}

	if (ThisPointerTo)
	{
		if (!GetCurrFunction()->FindLocalVariableName(ThisPointerTo, Name))
		{
			if (!GetGlobalVariableName(ThisPointerTo, Name))
			{
				HAZE_LOG_ERR(HAZE_TEXT("Haze parse function call not find variable name\n"));
			}
		}

		SStream << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CAST_VALUE_TYPE(HazeValueType::PointerClass) << " ";
		if (ThisPointerTo->IsPointerClass())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(ThisPointerTo);
			SStream << Name << " " << HAZE_CAST_SCOPE_TYPE(HazeDataDesc::ClassThis) << " " << PointerValue->GetPointerType().CustomName;
		}
		else if (ThisPointerTo->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(ThisPointerTo);
			SStream << Name << " " << HAZE_CAST_SCOPE_TYPE(HazeDataDesc::ClassThis) << " " << ClassValue->GetOwnerClassName();
		}

		SStream << std::endl;
		BB->PushIRCode(SStream.str());
		SStream.str(HAZE_TEXT(""));

		Size += ThisPointerTo->GetSize();
	}

	SStream << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CAST_VALUE_TYPE(HazeValueType::Int) << " " << HAZE_CALL_PUSH_ADDRESS_NAME
		<< " " << (uint32)HazeDataDesc::Address << std::endl;
	BB->PushIRCode(SStream.str());
	SStream.str(HAZE_TEXT(""));

	SStream << GetInstructionString(InstructionOpCode::CALL) << " " << CallFunction->GetName() << " " << Param.size() << " " << Size << std::endl;
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

	HazeDefineVariable Define;
	Define.Type.PrimaryType = HazeValueType::PointerBase;
	Define.Type.SecondaryType = HazeValueType::Char;

	it->second = CreateVariable(this, Define, HazeDataDesc::ConstantString, 0);

	it->second->GetValue().Value.Extra.StringTableIndex = (int)HashMap_StringMapping.size() - 1;

	return it->second;
}

uint32 HazeCompilerModule::GetGlobalStringIndex(std::shared_ptr<HazeCompilerValue> Value)
{
	for (auto& Iter : HashMap_StringTable)
	{
		if (Value == Iter.second)
		{
			for (auto& It : HashMap_StringMapping)
			{
				if (&Iter.first == It.second)
				{
					return It.first;
				}
			}
		}
	}

	return 0;
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

bool HazeCompilerModule::GetGlobalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	for (auto& it : Vector_Variable)
	{
		if (TrtGetVariableName(nullptr, it, Value, OutName))
		{
			return true;
		}
	}

	for (auto& it : HashMap_StringTable)
	{
		if (it.second == Value)
		{
			OutName = it.first;
			return true;
		}
	}

	return false;
}

bool HazeCompilerModule::GetGlobalVariableName(const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	for (auto& it : Vector_Variable)
	{
		if (TrtGetVariableName(nullptr, it, Value, OutName))
		{
			return true;
		}
	}

	for (auto& it : HashMap_StringTable)
	{
		if (it.second.get() == Value)
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

uint32 HazeCompilerModule::GetClassSize(const HAZE_STRING& ClassName)
{
	return FindClass(ClassName)->GetDataSize();
}

void HazeCompilerModule::GenValueHzicText(HazeCompilerModule* Module, HAZE_STRING_STREAM& HSS, const std::shared_ptr<HazeCompilerValue>& Value, int Index)
{
	bool StreamExtra = false;
	HAZE_STRING VarName;

	std::shared_ptr<HazeCompilerArrayValue> ArrayValue = nullptr;
	if (Value->IsArray() && Index >= 0)
	{
		ArrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(Value);
		HSS << (uint32)ArrayValue->GetArrayType().SecondaryType;
	}
	else
	{
		HSS << (uint32)Value->GetValue().Type;
	}

	if (Value->IsConstant())
	{
		HSS << " ";
		HazeCompilerStream(HSS, Value.get());
		HSS << " " << (uint32)HazeDataDesc::Constant;
	}
	else if (Value->IsString())
	{
		HSS << " " << HAZE_CONSTANT_STRING_NAME;			//空名字
		HSS << " " << (uint32)Value->GetScope() << " " << (uint32)HazeValueType::Char;
		HSS << " " << Module->GetGlobalStringIndex(Value);
	}
	else if (Value->IsRegister())
	{
		HSS << " " << HazeCompiler::GetRegisterName(Value);
		HSS << " " << (uint32)Value->GetScope();

		StreamExtra = true;
	}
	else
	{
		bool bFind = Module->GetCurrFunction()->FindLocalVariableName(Value, VarName);

		if (bFind)
		{
			HSS << " " << VarName;
			ArrayValue ? HSS << " " << (uint32)HazeDataDesc::ArrayElement : HSS << " " << (uint32)Value->GetScope();
		}
		else
		{
			Module->GetGlobalVariableName(Value, VarName);
			HSS << " " << VarName;
			ArrayValue ? HSS << " " << (uint32)HazeDataDesc::ArrayElement : HSS << " " << (uint32)HazeDataDesc::Global;
		}

		StreamExtra = true;
	}

	if (StreamExtra)
	{
		if (Value->IsPointerBase())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
			HSS << " " << (uint32)PointerValue->GetPointerType().SecondaryType;
		}
		else if (Value->IsPointerClass())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
			HSS << " " << PointerValue->GetPointerType().CustomName;
		}
		else if (Value->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value);
			HSS << " " << ClassValue->GetOwnerClassName();
		}
		else if (Value->IsArray() && Index >= 0)
		{
			/*if (Value->GetArrayType())
			{
				auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
				HSS << " " << (uint32)PointerValue->GetPointerType().SecondaryType;
			}
			else if (Value->IsPointerClass())
			{
				auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
				HSS << " " << PointerValue->GetPointerType().CustomName;
			}
			else if (Value->IsClass())
			{
				auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value);
				HSS << " " << ClassValue->GetOwnerClassName();
			}*/

			HSS << " " << Index;
		}
		else if (Value->IsArrayElement())
		{
			auto ArrayElementValue = std::dynamic_pointer_cast<HazeCompilerArrayElementValue>(Value);

			HSS << " ";
			if (ArrayElementValue->GetIndex()->IsConstant())
			{
				HazeCompilerStream(HSS, ArrayElementValue->GetIndex());
			}
			else
			{
				HAZE_STRING Name;
				if (Module->GetCurrFunction())
				{
					if (Module->GetCurrFunction()->FindLocalVariableName(ArrayElementValue->GetIndex(), Name))
					{
						HSS << Name;
					}
					else if (Module->GetGlobalVariableName(ArrayElementValue->GetIndex(), Name))
					{
						HSS << Name;
					}
					else
					{
						HAZE_LOG_ERR(HAZE_TEXT("GenValueHzicText variable not find\n"));
					}
				}
				
			}
		}
	}
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
		FS_I_Code << iter.first << " " << HAZE_CAST_VALUE_TYPE(iter.second->GetValue().Type);
		HazeCompilerOFStream(FS_I_Code, iter.second, true);
		FS_I_Code << std::endl;
	}

	/*
	*	字符串表 ：	个数
	*				字符串长度 字符串
	*/
	if (HashMap_StringMapping.size() != HashMap_StringTable.size())
	{
		HAZE_LOG_ERR(HAZE_TEXT("Parse header file string table size error : mapping size %d, table size %d"),
			HashMap_StringMapping.size(), HashMap_StringTable.size());
		return;
	}
	FS_I_Code << GetStringTableHeaderString() << std::endl;
	FS_I_Code << HashMap_StringMapping.size() << std::endl;

	for (auto& iter : HashMap_StringMapping)
	{
		FS_I_Code << iter.second->length() << " " << *iter.second << std::endl;
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
		iter.second->GenClassData_I_Code(FS_I_Code);
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
		iter.second->GenClassFunction_I_Code(FS_I_Code);
	}

	for (auto& iter : HashMap_Function)
	{
		iter.second->GenI_Code(FS_I_Code);
	}

	/*Main函数是否存在
	auto MainFuncIter = MapGlobalFunction.find(HAZE_MAIN_FUNCTION_TEXT);
	bool HasMain = MainFuncIter != MapGlobalFunction.end();
	FS_Ass << HasMain << std::endl;*/
}
