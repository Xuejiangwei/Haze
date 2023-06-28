#include <filesystem>
#include <unordered_set>

#include "HazeLog.h"
#include "HazeFilePathHelper.h"

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
	: Compiler(Compiler), ModuleLibraryType(HazeLibraryType::Normal)
{
#if HAZE_I_CODE_ENABLE

	FS_I_Code.imbue(std::locale("chs"));
	FS_I_Code.open(GetIntermediateModuleFile(ModuleName));

#endif
}

HazeCompilerModule::~HazeCompilerModule()
{
	if (FS_I_Code.is_open())
	{
		FS_I_Code.close();
	}
}

const HAZE_STRING& HazeCompilerModule::GetName() const
{
	return *Compiler->GetModuleName(this);
}

void HazeCompilerModule::MarkLibraryType(HazeLibraryType Type)
{
	ModuleLibraryType = Type;
}

void HazeCompilerModule::FinishModule()
{
	CurrFunction.clear();
	GenCodeFile();
}

void HazeCompilerModule::GenCodeFile()
{
#if HAZE_I_CODE_ENABLE

	//�����м�����Ȳ���Ҫ����symbol table���е�ƫ�ƣ���ͳһ�����ֽ���ʱ�ڽ����滻
	if (FS_I_Code.is_open())
	{
		GenICode();
		FS_I_Code.close();
	}

#endif
}

std::shared_ptr<HazeCompilerClass> HazeCompilerModule::CreateClass(const HAZE_STRING& Name,
	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& ClassData)
{
	std::shared_ptr<HazeCompilerClass> Class = GetClass(Name);
	if (!Class)
	{
		HashMap_Class[Name] = std::make_shared<HazeCompilerClass>(this, Name, ClassData);
		Class = HashMap_Class[Name];
		Class->InitThisValue();
	}

	CurrClass = Name;
	return Class;
}

void HazeCompilerModule::FinishCreateClass()
{
	CurrClass.clear();
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
		return GetClass(CurrClass)->FindFunction(CurrFunction);
	}

	return Iter->second;
}

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> HazeCompilerModule::GetFunction(const HAZE_STRING& Name)
{
	auto It = HashMap_Function.find(Name);
	if (It != HashMap_Function.end())
	{
		return { It->second, nullptr };
	}
	else if (!CurrClass.empty())
	{
		auto Iter = HashMap_Class.find(CurrClass);
		if (Iter != HashMap_Class.end())
		{
			return { Iter->second->FindFunction(Name), nullptr };
		}
	}
	else
	{
		bool IsPointer;
		return GetObjectFunction(this, Name, IsPointer);
	}

	return { nullptr, nullptr };
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(const HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
{
	std::shared_ptr<HazeCompilerFunction> Function = nullptr;
	auto It = HashMap_Function.find(Name);
	if (It == HashMap_Function.end())
	{
		HashMap_Function[Name] = std::make_shared<HazeCompilerFunction>(this, Name, Type, Param);
		HashMap_Function[Name]->InitEntryBlock(HazeBaseBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, HashMap_Function[Name], nullptr));

		Function = HashMap_Function[Name];
	}
	else
	{
		Function = It->second;
	}

	CurrFunction = Name;
	return Function;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerModule::CreateFunction(std::shared_ptr<HazeCompilerClass> Class, const HAZE_STRING& Name,
	HazeDefineType& Type, std::vector<HazeDefineVariable>& Param)
{
	std::shared_ptr<HazeCompilerFunction> Function = Class->FindFunction(Name);
	if (!Function)
	{
		Function = std::make_shared<HazeCompilerFunction>(this, GetHazeClassFunctionName(Class->GetName(), Name), Type, Param, Class.get());
		Class->AddFunction(Function);

		Function->InitEntryBlock(HazeBaseBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, Function, nullptr));
	}

	CurrFunction = Name;
	return Function;
}

void HazeCompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	CurrFunction.clear();
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

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateBitNeg(std::shared_ptr<HazeCompilerValue> Value)
{
	return GenIRCode_BinaryOperater(Value, nullptr, InstructionOpCode::BIT_NEG);
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

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateNot(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right)
{
	return GenIRCode_BinaryOperater(Left, Right, InstructionOpCode::NOT);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateNeg(std::shared_ptr<HazeCompilerValue> Value)
{
	if (Value->IsConstant())
	{
		Value = Compiler->GenConstantValue(Value->GetValueType().PrimaryType, GetNegValue(Value->GetValueType().PrimaryType, Value->GetValue()));
	}
	else
	{
		auto TempRegister = Compiler->GetTempRegister();
		Compiler->CreateMov(TempRegister, Value);

		HAZE_STRING_STREAM SStream;
		SStream << GetInstructionString(InstructionOpCode::NEG) << " ";
		GenVariableHzic(this, SStream, TempRegister);
		SStream << std::endl;

		std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
		BB->PushIRCode(SStream.str());

		Value = TempRegister;
	}

	return Value;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateInc(std::shared_ptr<HazeCompilerValue> Value, bool IsPreInc)
{
	std::shared_ptr<HazeCompilerValue> Ret = Value;
	if (IsHazeDefaultType(Value->GetValueType().PrimaryType) || Value->IsPointer())
	{
		if (!IsPreInc)
		{
			Ret = Compiler->GetTempRegister();
			Compiler->CreateMov(Ret, Value);
		}
		//Compiler->CreateMov(Value, CreateAdd(Value, Compiler->GetConstantValueInt(1)));
		GenIRCode_UnaryOperator(Value, InstructionOpCode::INC);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("<%s>���Ͳ���ʹ��Inc����\n"), GetHazeValueTypeString(Value->GetValueType().PrimaryType));
	}
	return Ret;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateDec(std::shared_ptr<HazeCompilerValue> Value, bool IsPreDec)
{
	std::shared_ptr<HazeCompilerValue> Ret = Value;
	if (IsHazeDefaultType(Value->GetValueType().PrimaryType) || Value->IsPointer())
	{
		if (!IsPreDec)
		{
			Ret = Compiler->GetTempRegister();
			Compiler->CreateMov(Ret, Value);
		}
		//Compiler->CreateMov(Value, CreateSub(Value, Compiler->GetConstantValueInt(1)));
		GenIRCode_UnaryOperator(Value, InstructionOpCode::DEC);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("<%s>���Ͳ���ʹ��Dec����\n"), GetHazeValueTypeString(Value->GetValueType().PrimaryType));
	}
	return Ret;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateArrayInit(std::shared_ptr<HazeCompilerValue> Array, std::shared_ptr<HazeCompilerValue> InitList)
{
	auto ArrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(Array);
	auto InitListValue = std::dynamic_pointer_cast<HazeCompilerInitListValue>(InitList);

	HAZE_STRING_STREAM SStream;
	HAZE_STRING VarName;

	if (IsHazeDefaultType(ArrayValue->GetValueType().SecondaryType))
	{
		for (int i = 0; i < (int)ArrayValue->GetArrayLength(); i++)
		{
			SStream << GetInstructionString(InstructionOpCode::MOV) << " ";

			GenVariableHzic(this, SStream, Array, i);

			SStream << " ";

			GenVariableHzic(this, SStream, InitListValue->GetList()[i]);

			SStream << std::endl;

			std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
			BB->PushIRCode(SStream.str());
			SStream.str(HAZE_TEXT(""));
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("Ŀǰֻ֧��HazeĬ�����͵������ʼ����ֵ!\n"));
	}

	return Array;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GenIRCode_BinaryOperater(std::shared_ptr<HazeCompilerValue> Left, std::shared_ptr<HazeCompilerValue> Right, InstructionOpCode IO_Code)
{
	static std::unordered_set<InstructionOpCode> HashSet_NoTemp =
	{
		InstructionOpCode::MOV,
		InstructionOpCode::MOVPV,
		InstructionOpCode::MOVTOPV,
		InstructionOpCode::LEA,
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

	bool NeedTemp = HashSet_NoTemp.find(IO_Code) == HashSet_NoTemp.end();

	if (!CurrFunction.empty())
	{
		HAZE_STRING_STREAM SStream;

		if (NeedTemp)
		{
			if ((!Left->IsRegister(HazeDataDesc::RegisterTemp) && Right == nullptr) ||
				(!Left->IsRegister(HazeDataDesc::RegisterTemp) && !Right->IsRegister(HazeDataDesc::RegisterTemp)))
			{
				Ret = Compiler->CreateMov(Compiler->GetTempRegister(), Left);
			}
			else
			{
				if (Right->IsRegister(HazeDataDesc::RegisterTemp) && Right->GetValueType() == Left->GetValueType())
				{
					Ret = Right;
					Right = Left;
				}
			}
		}

		SStream << GetInstructionString(IO_Code) << " ";
		GenVariableHzic(this, SStream, Ret);

		if (Right)
		{
			SStream << " ";
			GenVariableHzic(this, SStream, Right);
		}

		SStream << std::endl;

		std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
		BB->PushIRCode(SStream.str());
	}

	return Ret;
}

void HazeCompilerModule::GenIRCode_UnaryOperator(std::shared_ptr<HazeCompilerValue> Value, InstructionOpCode IO_Code)
{
	auto Function = GetCurrFunction();
	HAZE_STRING_STREAM SStream;

	if (CurrFunction.empty())
	{
		HAZE_TO_DO(ȫ�������ʱ������);
	}
	else
	{
		SStream << GetInstructionString(IO_Code) << " ";
		GenVariableHzic(this, SStream, Value);

		SStream << std::endl;
	}

	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	BB->PushIRCode(SStream.str());
}

void HazeCompilerModule::GenIRCode_Ret(std::shared_ptr<HazeCompilerValue> Value)
{
	auto Function = GetCurrFunction();
	HAZE_STRING_STREAM SStream;
	SStream << GetInstructionString(InstructionOpCode::RET) << " ";
	GenVariableHzic(this, SStream, Value);

	SStream << std::endl;

	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	BB->PushIRCode(SStream.str());
}

void HazeCompilerModule::GenIRCode_Cmp(HazeCmpType CmpType, std::shared_ptr<HazeBaseBlock> IfJmpBlock, std::shared_ptr<HazeBaseBlock> ElseJmpBlock)
{
	HAZE_STRING_STREAM SStream;

	if (CmpType == HazeCmpType::None)
	{
		HAZE_LOG_ERR(HAZE_TEXT("�Ƚ�ʧ��,�Ƚ�����Ϊ��,��ǰ����<%s>!\n"), GetCurrFunction()->GetName().c_str());
	}

	SStream << GetInstructionString(GetInstructionOpCodeByCmpType(CmpType)) << " ";

	if (IfJmpBlock)
	{
		SStream << IfJmpBlock->GetName() << " ";
	}
	else
	{
		SStream << HAZE_JMP_NULL << " ";
	}

	if (ElseJmpBlock)
	{
		SStream << ElseJmpBlock->GetName();
	}
	else
	{
		SStream << HAZE_JMP_NULL << " ";
	}

	SStream << std::endl;

	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	BB->PushIRCode(SStream.str());
}

void HazeCompilerModule::GenIRCode_JmpTo(std::shared_ptr<HazeBaseBlock> Block)
{
	auto TopBlock = Compiler->GetInsertBlock();
	HAZE_STRING_STREAM HSS;
	HSS << GetInstructionString(InstructionOpCode::JMP) << " " << Block->GetName() << std::endl;

	TopBlock->PushIRCode(HSS.str());
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateGlobalVariable(const HazeDefineVariable& Var, std::shared_ptr<HazeCompilerValue> RefValue,
	std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize, std::vector<HazeDefineType>* Vector_Param)
{
	for (auto& it : Vector_Variable)
	{
		if (it.first == Var.Name)
		{
			HAZE_LOG_ERR(HAZE_TEXT("����������,���ȫ�ֱ����ظ�\n"));
			return nullptr;
		}
	}

	Vector_Variable.push_back({ Var.Name,
		CreateVariable(this, Var, HazeVariableScope::Global, HazeDataDesc::None, 0, RefValue, ArraySize, Vector_Param) });

	auto& CompilerValue = Vector_Variable.back().second;

	return CompilerValue;
}

void HazeCompilerModule::FunctionCall(HAZE_STRING_STREAM& SStream, const HAZE_STRING& CallName, uint32& Size, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo)
{
	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	HAZE_STRING Name;

	for (size_t i = 0; i < Param.size(); i++)
	{
		auto Variable = Param[i];
		if (Param[i]->IsRef())
		{
			Variable = Compiler->CreateMovPV(Compiler->GetTempRegister(), Param[i]);
		}
		else if (Param[i]->IsArrayElement())
		{
			Variable = GetArrayElementToValue(this, Param[i], Compiler->GetTempRegister());
		}

		SStream << GetInstructionString(InstructionOpCode::PUSH) << " ";
		GenVariableHzic(this, SStream, Variable);

		/*if (GetCurrFunction())
		{
			if (!GetCurrFunction()->FindLocalVariableName(Variable, Name))
			{
				if (!GetGlobalVariableName(Variable, Name))
				{
					if (Variable->IsRegister())
					{
						Name = Compiler->GetRegisterName(Variable);
					}
					else if (Variable->IsLocalVariable() || Variable->IsGlobalVariable() || Variable->IsRegister())
					{
						HAZE_LOG_ERR(HAZE_TEXT("��������<%s>����,δ���ҵ�����!\n"), CallName.c_str());
					}
				}
			}
		}
		else
		{
			if (!GetGlobalVariableName(Variable, Name))
			{
				if (Variable->IsRegister())
				{
					Name = Compiler->GetRegisterName(Variable);
				}
				else if (Variable->IsLocalVariable() || Variable->IsGlobalVariable() || Variable->IsRegister())
				{
					HAZE_LOG_ERR(HAZE_TEXT("��������<%s>����,δ���ҵ�����!\n"), CallName.c_str());
				}
			}
		}

		if (Variable->IsConstant())
		{
			HazeCompilerStream(SStream, Variable.get());
			SStream << " " << (uint32)Variable->GetVariableDesc();
		}
		else if (Variable->IsString())
		{
			SStream << HAZE_CONSTANT_STRING_NAME << " " << (uint32)Variable->GetVariableDesc() << " " << (uint32)Variable->GetValueType().SecondaryType
				<< " " << Variable->GetValue().Value.Extra.StringTableIndex;
		}
		else if (Variable->IsPointerBase() || Variable->IsRefBase())
		{
			SStream << Name << " " << (uint32)Variable->GetVariableDesc() << " " << (uint32)Variable->GetValueType().SecondaryType;
		}
		else if (Variable->IsPointerClass() || Variable->IsRefClass())
		{
			SStream << Name << " " << (uint32)Variable->GetVariableDesc() << " " << Variable->GetValueType().CustomName;
		}
		else if (Variable->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Variable);
			SStream << Name << " " << (uint32)Variable->GetVariableDesc() << " " << ClassValue->GetOwnerClassName();
		}
		else if (Variable->IsArrayElement())
		{
			Variable = GetArrayElementToValue(this, Variable);

			if (!GetCurrFunction()->FindLocalVariableName(Variable, Name))
			{
				if (!GetGlobalVariableName(Variable, Name))
				{
					if (Variable->IsRegister())
					{
						Name = Compiler->GetRegisterName(Variable);
					}
					else if (Variable->IsLocalVariable() || Variable->IsGlobalVariable() || Variable->IsRegister())
					{
						HAZE_LOG_ERR(HAZE_TEXT("Haze parse function call not find variable name!\n"));
					}
				}
			}

			SStream << Name << " " << (uint32)Variable->GetVariableDesc() << " ";
		}
		else
		{
			SStream << Name << " " << (uint32)Variable->GetVariableDesc();
		}*/

		SStream << std::endl;
		BB->PushIRCode(SStream.str());
		SStream.str(HAZE_TEXT(""));

		Size += Variable->GetSize();
		Name.clear();
	}

	if (ThisPointerTo)
	{
		if (!GetCurrFunction()->FindLocalVariableName(ThisPointerTo, Name))
		{
			if (!GetGlobalVariableName(ThisPointerTo, Name))
			{
				HAZE_LOG_ERR_W("���ɺ���<%s>���ô���,û���ҵ�������������!\n", CallName.c_str());
			}
		}

		SStream << GetInstructionString(InstructionOpCode::PUSH) << " ";
		if (ThisPointerTo->IsPointerClass())
		{
			auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(ThisPointerTo);
			SStream << Name << " " << CAST_SCOPE(PointerValue->GetVariableScope()) << " " << CAST_DESC(HazeDataDesc::ClassThis) << " ";
		}
		else if (ThisPointerTo->IsClass())
		{
			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(ThisPointerTo);
			SStream << Name << " " << CAST_SCOPE(ClassValue->GetVariableScope()) << " " << CAST_TYPE(HazeDataDesc::ClassThis) << " " <<
				CAST_TYPE(HazeValueType::PointerClass) << " " << ClassValue->GetOwnerClassName();
		}

		SStream << std::endl;
		BB->PushIRCode(SStream.str());
		SStream.str(HAZE_TEXT(""));

		Size += GetSizeByHazeType(HazeValueType::PointerClass);
	}

	SStream << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CALL_PUSH_ADDRESS_NAME << " " << CAST_SCOPE(HazeVariableScope::None)
		<< " " << (uint32)HazeDataDesc::Address << " " << CAST_TYPE(HazeValueType::Int) << std::endl;
	BB->PushIRCode(SStream.str());
	SStream.str(HAZE_TEXT(""));
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerFunction> CallFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo)
{
	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	HAZE_STRING_STREAM SStream;
	uint32 Size = 0;

	FunctionCall(SStream, CallFunction->GetName(), Size, Param, ThisPointerTo);

	SStream << GetInstructionString(InstructionOpCode::CALL) << " " << CallFunction->GetName() << " " << CAST_TYPE(HazeValueType::Function) << " " << Param.size()
		<< " " << Size << " " << CallFunction->GetModule()->GetName() << std::endl;
	BB->PushIRCode(SStream.str());

	auto RetRegister = HazeCompiler::GetRegister(RET_REGISTER);

	auto& RetRegisterType = const_cast<HazeDefineType&>(RetRegister->GetValueType());
	RetRegisterType = CallFunction->GetFunctionType();
	return RetRegister;
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::CreateFunctionCall(std::shared_ptr<HazeCompilerValue> PointerFunction, std::vector<std::shared_ptr<HazeCompilerValue>>& Param, std::shared_ptr<HazeCompilerValue> ThisPointerTo)
{
	std::shared_ptr<HazeBaseBlock> BB = Compiler->GetInsertBlock();
	HAZE_STRING_STREAM SStream;
	uint32 Size = 0;

	HAZE_STRING VarName;
	GetGlobalVariableName(PointerFunction, VarName);
	if (VarName.empty())
	{
		GetCurrFunction()->FindLocalVariableName(PointerFunction, VarName);
		if (VarName.empty())
		{
			HAZE_LOG_ERR(HAZE_TEXT("����ָ�����ʧ��!\n"));
			return nullptr;
		}
	}

	FunctionCall(SStream, VarName, Size, Param, ThisPointerTo);

	SStream << GetInstructionString(InstructionOpCode::CALL) << " " << VarName << " " << CAST_TYPE(HazeValueType::PointerFunction) << " " << Param.size()
		<< " " << Size << " " << Compiler->GetCurrModuleName() << std::endl;
	BB->PushIRCode(SStream.str());

	return HazeCompiler::GetRegister(RET_REGISTER);
}

std::shared_ptr<HazeCompilerValue> HazeCompilerModule::GetOrCreateGlobalStringVariable(const HAZE_STRING& String)
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

	it->second = CreateVariable(this, Define, HazeVariableScope::Global, HazeDataDesc::ConstantString, 0);

	HazeValue& V = const_cast<HazeValue&>(it->second->GetValue());
	V.Value.Extra.StringTableIndex = (int)HashMap_StringMapping.size() - 1;

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
		else if (it.second->IsClass())
		{
			auto Ret = GetObjectMember(this, Name);
			if (Ret)
			{
				return Ret;
			}
		}
	}

	for (auto& Module : Vector_ImportModule)
	{
		auto Ret = Module->GetGlobalVariable(Name);
		if (Ret)
		{
			return Ret;
		}
	}

	return nullptr;
}

bool HazeCompilerModule::GetGlobalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	if (Value->IsRegister())
	{
		OutName = Compiler->GetRegisterName(Value);
		return true;
	}

	for (auto& It : Vector_Variable)
	{
		if (TrtGetVariableName(nullptr, It, Value, OutName))
		{
			return true;
		}
	}

	for (auto& It : Vector_ImportModule)
	{
		if (It->GetGlobalVariableName(Value, OutName))
		{
			return true;
		}
	}

	for (auto& It : HashMap_StringTable)
	{
		if (It.second == Value)
		{
			OutName = HAZE_CONSTANT_STRING_NAME;//It.first;
			return true;
		}
	}

	return Value->TryGetVariableName(OutName);
}

bool HazeCompilerModule::GetGlobalVariableName(const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	for (auto& It : Vector_Variable)
	{
		if (TrtGetVariableName(nullptr, It, Value, OutName))
		{
			return true;
		}
	}

	for (auto& It : HashMap_StringTable)
	{
		if (It.second.get() == Value)
		{
			OutName = It.first;
			return true;
		}
	}

	return false;
}

std::shared_ptr<HazeCompilerClass> HazeCompilerModule::GetClass(const HAZE_STRING& ClassName)
{
	auto Iter = HashMap_Class.find(ClassName);
	if (Iter != HashMap_Class.end())
	{
		return Iter->second;
	}

	for (auto& It : Vector_ImportModule)
	{
		auto Ret = It->GetClass(ClassName);
		if (Ret)
		{
			return Ret;
		}
	}

	return nullptr;
}

uint32 HazeCompilerModule::GetClassSize(const HAZE_STRING& ClassName)
{
	return GetClass(ClassName)->GetDataSize();
}

//void HazeCompilerModule::GenValueHzicText(HazeCompilerModule* Module, HAZE_STRING_STREAM& HSS, const std::shared_ptr<HazeCompilerValue>& Value, int Index)
//{
//	bool StreamExtra = false;
//	HAZE_STRING VarName;
//
//	std::shared_ptr<HazeCompilerArrayValue> ArrayValue = nullptr;
//	if (Value->IsArray() && Index >= 0)
//	{
//		ArrayValue = std::dynamic_pointer_cast<HazeCompilerArrayValue>(Value);
//		HSS << (uint32)ArrayValue->GetValueType().SecondaryType;
//	}
//	else
//	{
//		HSS << (uint32)Value->GetValueType().PrimaryType;
//	}
//
//	if (Value->IsConstant())
//	{
//		HSS << " ";
//		HazeCompilerStream(HSS, Value.get());
//		HSS << " " << (uint32)HazeDataDesc::Constant;
//	}
//	else if (Value->IsString())
//	{
//		HSS << " " << HAZE_CONSTANT_STRING_NAME;			//������
//		HSS << " " << (uint32)Value->GetVariableDesc() << " " << (uint32)HazeValueType::Char;
//		HSS << " " << Module->GetGlobalStringIndex(Value);
//	}
//	else if (Value->IsNullPtr())
//	{
//		HSS << " " << 0 << " " << (uint32)Value->GetVariableDesc();
//	}
//	else if (Value->IsRegister())
//	{
//		HSS << " " << HazeCompiler::GetRegisterName(Value);
//		HSS << " " << (uint32)Value->GetVariableDesc();
//
//		StreamExtra = true;
//	}
//	else
//	{
//		bool bFind = Module->GetCurrFunction()->FindLocalVariableName(Value, VarName);
//
//		if (bFind)
//		{
//			HSS << " " << VarName;
//			ArrayValue ? HSS << " " << (uint32)HazeDataDesc::ArrayElement : HSS << " " << (uint32)Value->GetVariableDesc();
//		}
//		else
//		{
//			bFind = Module->GetGlobalVariableName(Value, VarName);
//			if (bFind)
//			{
//				HSS << " " << VarName;
//				ArrayValue ? HSS << " " << (uint32)HazeDataDesc::ArrayElement : HSS << " " << (uint32)HazeVariableScope::Global;
//			}
//			else if (Value->IsPointerFunction())
//			{
//				HSS << " " << Value->GetValueType().CustomName << " " << (uint32)HazeDataDesc::FunctionAddress;		//��ʾ��Ҫ�����в��Һ�����ַ
//			}
//
//			else
//			{
//				HAZE_LOG_ERR(HAZE_TEXT("�����м������:�����ҵ�����! ��ǰ����<%s>\n"), Module->CurrFunction.empty() ? HAZE_TEXT("None") : Module->GetCurrFunction()->GetName().c_str());
//			}
//		}
//
//		StreamExtra = true;
//	}
//
//	if (StreamExtra)
//	{
//		if (Value->IsPointerBase() || Value->IsRefBase())
//		{
//			HSS << " " << (uint32)Value->GetValueType().SecondaryType;
//		}
//		else if (Value->IsPointerClass() || Value->IsRefClass())
//		{
//			HSS << " " << Value->GetValueType().CustomName;
//		}
//		else if (Value->IsClass())
//		{
//			auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value);
//			HSS << " " << ClassValue->GetOwnerClassName();
//		}
//		else if (Value->IsArray())
//		{
//			if (Index >= 0)
//			{
//				HSS << " " << Index;
//			}
//
//			/*if (Value->GetArrayType())
//			{
//				auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
//				HSS << " " << (uint32)PointerValue->GetPointerType().SecondaryType;
//			}
//			else if (Value->IsPointerClass())
//			{
//				auto PointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(Value);
//				HSS << " " << PointerValue->GetPointerType().CustomName;
//			}
//			else if (Value->IsClass())
//			{
//				auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(Value);
//				HSS << " " << ClassValue->GetOwnerClassName();
//			}*/
//		}
//		else if (Value->IsArrayElement())
//		{
//			auto ArrayElementValue = std::dynamic_pointer_cast<HazeCompilerArrayElementValue>(Value);
//
//			HSS << " ";
//			if (ArrayElementValue->GetIndex()[0]->IsConstant())
//			{
//				HazeCompilerStream(HSS, ArrayElementValue->GetIndex()[0]);
//			}
//			else
//			{
//				HAZE_STRING Name;
//				if (Module->GetCurrFunction())
//				{
//					if (Module->GetCurrFunction()->FindLocalVariableName(ArrayElementValue->GetIndex()[0], Name))
//					{
//						HSS << Name;
//					}
//					else if (Module->GetGlobalVariableName(ArrayElementValue->GetIndex()[0], Name))
//					{
//						HSS << Name;
//					}
//					else
//					{
//						HAZE_LOG_ERR(HAZE_TEXT("����ʧ��,����δ���ҵ�!\n"));
//					}
//				}
//			}
//		}
//	}
//}

void HazeCompilerModule::GenVariableHzic(HazeCompilerModule* Module, HAZE_STRING_STREAM& HSS, const std::shared_ptr<HazeCompilerValue>& Value, int Index)
{
	static HAZE_STRING Name;

	bool Find = false;

	Name.clear();
	if (Value->IsGlobalVariable())
	{
		Find = Module->GetGlobalVariableName(Value, Name);
	}
	else if (Value->IsLocalVariable())
	{
		Find = Module->GetCurrFunction()->FindLocalVariableName(Value, Name);
	}
	else
	{
		HAZE_LOG_ERR_W("�����м�������,�������������!\n");
		return;
	}

	if (!Find)
	{
		HAZE_LOG_ERR_W("�����м�������,δ���ҵ�����!\n");
		return;
	}

	HSS << Name << " " << CAST_SCOPE(Value->GetVariableScope()) << " " << CAST_DESC(Value->GetVariableDesc()) << " ";
	Value->GetValueType().StringStreamTo(HSS);

	if (Value->IsString())
	{
		Index = Module->GetGlobalStringIndex(Value);
	}

	if (Index >= 0)
	{
		HSS << " " << Index;
	}
}

void HazeCompilerModule::GenICode()
{
	//�汾 2���ֽ�
	//FS_Ass << "1 1" << std::endl;

	//��ջ 4���ֽ�
	//FS_Ass << 1024 << std::endl;

	//������
	HAZE_STRING_STREAM HSS_I_Code;
	HSS_I_Code << (uint32)ModuleLibraryType << std::endl;
	/*
	*	ȫ������ ��	����
	*				������ �������� ����
	*/
	HSS_I_Code << GetGlobalDataHeaderString() << std::endl;
	HSS_I_Code << Vector_Variable.size() << std::endl;

	for (auto& iter : Vector_Variable)
	{
		HSS_I_Code << iter.first << " " << iter.second->GetSize() << " " << CAST_TYPE(iter.second->GetValueType().PrimaryType) << " ";

		if (IsHazeDefaultType(iter.second->GetValueType().PrimaryType))
		{
			HazeCompilerStream(HSS_I_Code, iter.second);
		}
		else
		{
			if (iter.second->IsClass())
			{
				auto ClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(iter.second);
				HSS_I_Code << ClassValue->GetOwnerClassName();
			}
		}

		HSS_I_Code << std::endl;
	}

	/*
	*	�ַ����� ��	����
	*				�ַ������� �ַ���
	*/
	if (HashMap_StringMapping.size() != HashMap_StringTable.size())
	{
		HAZE_LOG_ERR(HAZE_TEXT("�����ַ�����ʧ��!"),
			HashMap_StringMapping.size(), HashMap_StringTable.size());
		return;
	}
	HSS_I_Code << GetStringTableHeaderString() << std::endl;
	HSS_I_Code << HashMap_StringMapping.size() << std::endl;

	for (auto& iter : HashMap_StringMapping)
	{
		HSS_I_Code << iter.second->length() << " " << *iter.second << std::endl;
	}

	/*
	*	��� ��	����
	*				���� ָ����
	*
	*/
	size_t FunctionSize = 0;

	HSS_I_Code << GetClassTableHeaderString() << std::endl;
	HSS_I_Code << HashMap_Class.size() << std::endl;

	for (auto& iter : HashMap_Class)
	{
		iter.second->GenClassData_I_Code(HSS_I_Code);
		FunctionSize += iter.second->GetFunctionNum();
	}

	/*
	*	������ ��	����
	*				���� ָ����
	*
	*/
	HSS_I_Code << GetFucntionTableHeaderString() << std::endl;
	HSS_I_Code << HashMap_Function.size() + FunctionSize << std::endl;

	for (auto& iter : HashMap_Class)
	{
		iter.second->GenClassFunction_I_Code(HSS_I_Code);
	}

	for (auto& iter : HashMap_Function)
	{
		iter.second->GenI_Code(HSS_I_Code);
	}

	FS_I_Code << HSS_I_Code.str();
}