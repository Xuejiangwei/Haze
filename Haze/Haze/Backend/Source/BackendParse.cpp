#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeUtility.h"
#include "HazeVM.h"
#include "HazeExecuteFile.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

#define BACKEND_INSTRUCTION_LOG			0

static std::pair<bool, int> ParseStringCount = { false, 0 };

static void FindObjectAndMemberName(const HAZE_STRING& InName, HAZE_STRING& OutObjectName, HAZE_STRING& OutMemberName, bool& ObjectIsPointer)
{
	size_t Pos = InName.find(HAZE_CLASS_POINTER_ATTR);
	if (Pos != HAZE_STRING::npos)
	{
		OutObjectName = InName.substr(0, Pos);
		OutMemberName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());
		ObjectIsPointer = true;
	}
	else
	{
		Pos = InName.find(HAZE_CLASS_ATTR);
		if (Pos != HAZE_STRING::npos)
		{
			OutObjectName = InName.substr(0, Pos);
			OutMemberName = InName.substr(Pos + HAZE_STRING(HAZE_CLASS_ATTR).size());
			ObjectIsPointer = false;
		}
	}
}

static bool IsIgnoreFindAddressInsCode(ModuleUnit::FunctionInstruction& Ins)
{
	if (Ins.InsCode == InstructionOpCode::LINE)
	{
		return true;
	}

	if (Ins.InsCode == InstructionOpCode::CALL && Ins.Operator[0].Variable.Type.PrimaryType == HazeValueType::Function)
	{
		return true;
	}

	if (Ins.InsCode == InstructionOpCode::RET && IsVoidType(Ins.Operator[0].Variable.Type.PrimaryType))
	{
		return true;
	}

	return false;
}

static bool IsIgnoreFindAddress(InstructionData& Operator)
{
	if (Operator.Desc == HazeDataDesc::Constant)
	{
		Operator.AddressType = InstructionAddressType::Constant;
		return true;
	}

	if (Operator.Desc == HazeDataDesc::ConstantString)
	{
		Operator.AddressType = InstructionAddressType::ConstantString;
		return true;
	}

	if (Operator.Desc == HazeDataDesc::CallFunctionModule)
	{
		return true;
	}

	if (Operator.Variable.Name == HAZE_CALL_PUSH_ADDRESS_NAME)
	{
		Operator.AddressType = InstructionAddressType::Local;
		return true;
	}

	if (IsRegisterDesc(Operator.Desc))
	{
		Operator.AddressType = InstructionAddressType::Register;
		return true;
	}

	return false;
}

BackendParse::BackendParse(HazeVM* VM) : VM(VM), CurrCode(nullptr)
{
}

BackendParse::~BackendParse()
{
}

void BackendParse::Parse()
{
	HAZE_STRING I_CodePath;

	auto& Modules = VM->GetReferenceModules();

	HAZE_STRING CodeText;

	for (auto& Module : Modules)
	{
		CurrParseModule = std::make_shared<ModuleUnit>(Module);
		HashMap_Modules[Module] = CurrParseModule;

		HAZE_IFSTREAM FS(GetIntermediateModuleFile(Module));
		FS.imbue(std::locale("chs"));

		HAZE_STRING Content(std::istreambuf_iterator<HAZE_CHAR>(FS), {});

		CodeText = std::move(Content);
		CurrCode = CodeText.c_str();

		Parse_I_Code();

		FS.close();
	}

	GenOpCodeFile();
}

void BackendParse::GetNextLexeme()
{
	CurrLexeme.clear();
	while (HazeIsSpace(*CurrCode))
	{
		CurrCode++;
	}

	CurrLexeme.clear();
	while (!HazeIsSpace(*CurrCode) || (ParseStringCount.first && ParseStringCount.second > 0))
	{
		CurrLexeme += *(CurrCode++);
		ParseStringCount.second--;
	}
}

void BackendParse::Parse_I_Code()
{
	//Standard lib
	GetNextLexeme();
	CurrParseModule->LibraryType = (HazeLibraryType)StringToStandardType<uint32>(CurrLexeme);

	//Global data
	GetNextLexeme();
	Parse_I_Code_GlobalTable();

	//String table
	GetNextLexeme();
	Parse_I_Code_StringTable();

	//Class table
	GetNextLexeme();
	Parse_I_Code_ClassTable();

	//Function table
	GetNextLexeme();
	Parse_I_Code_FunctionTable();
}

void BackendParse::Parse_I_Code_GlobalTable()
{
	if (CurrLexeme == GetGlobalDataHeaderString())
	{
		GetNextLexeme();
		uint32 Num = StringToStandardType<uint32>(CurrLexeme);

		ModuleUnit::GlobalDataTable& Table = CurrParseModule->Table_GlobalData;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextLexmeAssign_HazeString(Table.Vector_Data[i].Name);

			GetNextLexmeAssign_StandardType(Table.Vector_Data[i].Size);

			GetNextLexmeAssign_CustomType<uint32>(Table.Vector_Data[i].Type.PrimaryType);

			if (IsHazeDefaultType(Table.Vector_Data[i].Type.PrimaryType))
			{
				GetNextLexeme();
				StringToHazeValueNumber(CurrLexeme, Table.Vector_Data[i].Type.PrimaryType, Table.Vector_Data[i].Value);
			}
			else
			{
				if (IsClassType(Table.Vector_Data[i].Type.PrimaryType))
				{
					GetNextLexmeAssign_HazeString(Table.Vector_Data[i].Type.CustomName);
					Table.ClassObjectAllSize += Table.Vector_Data[i].Size;
				}
			}
		}
	}
}

void BackendParse::Parse_I_Code_StringTable()
{
	if (CurrLexeme == GetStringTableHeaderString())
	{
		uint32 Num;
		GetNextLexmeAssign_StandardType(Num);

		ModuleUnit::StringTable& Table = CurrParseModule->Table_String;
		Table.Vector_String.resize(Num);

		ParseStringCount.first = true;
		for (size_t i = 0; i < Table.Vector_String.size(); i++)
		{
			GetNextLexmeAssign_StandardType(ParseStringCount.second);
			GetNextLexmeAssign_HazeString(Table.Vector_String[i].String);
		}
		ParseStringCount.first = false;
	}
}

void BackendParse::Parse_I_Code_ClassTable()
{
	if (CurrLexeme == GetClassTableHeaderString())
	{
		uint32 Num;
		GetNextLexmeAssign_StandardType(Num);

		ModuleUnit::ClassTable& Table = CurrParseModule->Table_Class;
		Table.Vector_Class.resize(Num);

		for (size_t i = 0; i < Table.Vector_Class.size(); i++)
		{
			GetNextLexeme();

			if (CurrLexeme == GetClassLabelHeader())
			{
				auto& Class = Table.Vector_Class[i];

				GetNextLexmeAssign_HazeString(Class.Name);
				GetNextLexmeAssign_StandardType(Class.Size);
				GetNextLexmeAssign_StandardType(Num);
				Class.Vector_Member.resize(Num);

				for (size_t j = 0; j < Class.Vector_Member.size(); j++)
				{
					auto& ClassMember = Class.Vector_Member[j];

					GetNextLexmeAssign_HazeString(ClassMember.Variable.Name);

					GetNextLexmeAssign_CustomType<uint32>(ClassMember.Variable.Type.PrimaryType);

					if (ClassMember.Variable.Type.PrimaryType == HazeValueType::PointerClass || ClassMember.Variable.Type.PrimaryType == HazeValueType::Class)
					{
						GetNextLexmeAssign_HazeString(ClassMember.Variable.Type.CustomName);
					}
					else if (ClassMember.Variable.Type.PrimaryType == HazeValueType::PointerBase)
					{
						GetNextLexmeAssign_CustomType<uint32>(ClassMember.Variable.Type.SecondaryType);
					}

					GetNextLexmeAssign_CustomType<uint32>(ClassMember.Offset);
					GetNextLexmeAssign_CustomType<uint32>(ClassMember.Size);
				}
			}
		}
	}
}

void BackendParse::Parse_I_Code_FunctionTable()
{
	if (CurrLexeme == GetFucntionTableHeaderString())
	{
		uint32 Num;
		GetNextLexmeAssign_StandardType(Num);

		ModuleUnit::FunctionTable& Table = CurrParseModule->Table_Function;
		Table.Vector_Function.resize(Num);

		for (size_t i = 0; i < Table.Vector_Function.size(); i++)
		{
			GetNextLexeme();

			if (CurrLexeme == GetFunctionLabelHeader())
			{
				Table.Vector_Function[i].DescType = GetFunctionTypeByLibraryType(CurrParseModule->LibraryType);

				GetNextLexmeAssign_HazeString(Table.Vector_Function[i].Name);

				GetNextLexmeAssign_CustomType<uint32>(Table.Vector_Function[i].Type);

				GetNextLexeme();
				while (CurrLexeme == GetFunctionParamHeader())
				{
					HazeDefineVariable Param;
					GetNextLexmeAssign_HazeString(Param.Name);
					Param.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeString, &BackendParse::GetNextLexmeAssign_CustomType<uint32>);

					Table.Vector_Function[i].Vector_Param.push_back(std::move(Param));

					GetNextLexeme();
				}

				while (CurrLexeme == GetFunctionVariableHeader())
				{
					HazeVariableData Var;
					GetNextLexmeAssign_HazeString(Var.Variable.Name);

					Var.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeString, &BackendParse::GetNextLexmeAssign_CustomType<uint32>);
					GetNextLexmeAssign_CustomType<int>(Var.Offset);
					GetNextLexmeAssign_CustomType<uint32>(Var.Size);
					GetNextLexmeAssign_CustomType<uint32>(Var.Line);

					Table.Vector_Function[i].Vector_Variable.push_back(std::move(Var));
					GetNextLexeme();
				}

				if (CurrLexeme == GetFunctionStartHeader())
				{
					GetNextLexeme();
					while (CurrLexeme == BLOCK_START)
					{
						GetNextLexeme();
						Table.Vector_Function[i].Vector_Block.push_back({ CurrLexeme, 0, (int)Table.Vector_Function[i].Vector_Instruction.size() });
						auto& CurrBlock = Table.Vector_Function[i].Vector_Block.back();

						GetNextLexeme();
						while (CurrLexeme != BLOCK_START && CurrLexeme != GetFunctionEndHeader())
						{
							ModuleUnit::FunctionInstruction Instruction;
							Instruction.InsCode = GetInstructionByString(CurrLexeme);

							ParseInstruction(Instruction);

							Table.Vector_Function[i].Vector_Instruction.push_back(Instruction);

							CurrBlock.InstructionNum++;

							GetNextLexeme();
						}

						if (CurrLexeme == GetFunctionEndHeader())
						{
							break;
						}
					}
				}
			}
		}
	}
}

void BackendParse::ParseInstructionData(InstructionData& Data)
{
	GetNextLexmeAssign_HazeString(Data.Variable.Name);
	GetNextLexmeAssign_CustomType<uint32>(Data.Scope);
	GetNextLexmeAssign_CustomType<uint32>(Data.Desc);

	Data.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeString,
		&BackendParse::GetNextLexmeAssign_CustomType<uint32>);

	if (Data.Desc == HazeDataDesc::ArrayElement || Data.Desc == HazeDataDesc::ConstantString)
	{
		GetNextLexmeAssign_CustomType<uint32>(Data.Extra.Index);
	}
}

void BackendParse::ParseInstruction(ModuleUnit::FunctionInstruction& Instruction)
{
	switch (Instruction.InsCode)
	{
	case InstructionOpCode::NONE:
		break;
	case InstructionOpCode::MOV:
	case InstructionOpCode::MOVPV:
	case InstructionOpCode::MOVTOPV:
	case InstructionOpCode::LEA:
	case InstructionOpCode::ADD:
	case InstructionOpCode::SUB:
	case InstructionOpCode::MUL:
	case InstructionOpCode::DIV:
	case InstructionOpCode::MOD:
	case InstructionOpCode::CMP:
	case InstructionOpCode::NOT:
	case InstructionOpCode::BIT_AND:
	case InstructionOpCode::BIT_OR:
	case InstructionOpCode::BIT_XOR:
	case InstructionOpCode::SHL:
	case InstructionOpCode::SHR:
	case InstructionOpCode::ADD_ASSIGN:
	case InstructionOpCode::SUB_ASSIGN:
	case InstructionOpCode::MUL_ASSIGN:
	case InstructionOpCode::DIV_ASSIGN:
	case InstructionOpCode::MOD_ASSIGN:
	case InstructionOpCode::BIT_AND_ASSIGN:
	case InstructionOpCode::BIT_OR_ASSIGN:
	case InstructionOpCode::BIT_XOR_ASSIGN:
	case InstructionOpCode::SHL_ASSIGN:
	case InstructionOpCode::SHR_ASSIGN:
	{
		InstructionData OperatorOne;
		InstructionData OperatorTwo;

		ParseInstructionData(OperatorOne);
		ParseInstructionData(OperatorTwo);

		Instruction.Operator = { OperatorOne, OperatorTwo };
	}
	break;
	case InstructionOpCode::NEG:
	case InstructionOpCode::INC:
	case InstructionOpCode::DEC:
	case InstructionOpCode::BIT_NEG:
	case InstructionOpCode::PUSH:
	case InstructionOpCode::POP:
	case InstructionOpCode::RET:
	{
		InstructionData OperatorOne;
		ParseInstructionData(OperatorOne);

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::CALL:
	{
		InstructionData OperatorOne;
		InstructionData OperatorTwo;

		GetNextLexmeAssign_HazeString(OperatorOne.Variable.Name);

		GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Variable.Type.PrimaryType);

		if (IsPointerFunction(OperatorOne.Variable.Type.PrimaryType))
		{
			GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Scope);
			GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Desc);
		}

		GetNextLexmeAssign_CustomType<int>(OperatorOne.Extra.Call.ParamNum);
		GetNextLexmeAssign_CustomType<int>(OperatorOne.Extra.Call.ParamByteSize);

		GetNextLexmeAssign_HazeString(OperatorTwo.Variable.Name);
		OperatorTwo.Desc = HazeDataDesc::CallFunctionModule;

		Instruction.Operator = { OperatorOne, OperatorTwo };
	}
	break;
	case InstructionOpCode::NEW:
	{
		InstructionData OperatorOne;

		GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Variable.Type.PrimaryType);

		if (OperatorOne.Variable.Type.PrimaryType == HazeValueType::PointerClass)
		{
			GetNextLexmeAssign_HazeString(OperatorOne.Variable.Type.CustomName);
		}
		else
		{
			GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Variable.Type.SecondaryType);
		}

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::JMP:
	{
		InstructionData OperatorOne;
		GetNextLexmeAssign_HazeString(OperatorOne.Variable.Name);

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::JNE:
	case InstructionOpCode::JNG:
	case InstructionOpCode::JNL:
	case InstructionOpCode::JE:
	case InstructionOpCode::JG:
	case InstructionOpCode::JL:
	{
		InstructionData OperatorOne;
		InstructionData OperatorTwo;

		GetNextLexmeAssign_HazeString(OperatorOne.Variable.Name);

		GetNextLexmeAssign_HazeString(OperatorTwo.Variable.Name);

		Instruction.Operator = { OperatorOne, OperatorTwo };
	}
	break;
	case InstructionOpCode::LINE:
	{
		InstructionData OperatorOne;
		GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Extra.Line);

		Instruction.Operator = { OperatorOne };
	}
	break;
	default:
		break;
	}
}

void BackendParse::GenOpCodeFile()
{
	HazeExecuteFile ExeFile(ExeFileType::Out);

	if (VM->IsDebug())
	{
		ExeFile.WriteModule(HashMap_Modules);
	}

	ModuleUnit::GlobalDataTable NewGlobalDataTable;
	ModuleUnit::StringTable NewStringTable;
	ModuleUnit::ClassTable NewClassTable;
	ModuleUnit::FunctionTable NewFunctionTable;

	size_t FunctionCount = 0;
	for (auto& Module : HashMap_Modules)
	{
		NewFunctionTable.Vector_Function.insert(NewFunctionTable.Vector_Function.end(), Module.second->Table_Function.Vector_Function.begin(), Module.second->Table_Function.Vector_Function.end());
		ReplaceStringIndex(NewStringTable, NewFunctionTable, FunctionCount);

		NewGlobalDataTable.Vector_Data.insert(NewGlobalDataTable.Vector_Data.end(), Module.second->Table_GlobalData.Vector_Data.begin(), Module.second->Table_GlobalData.Vector_Data.end());
		NewGlobalDataTable.ClassObjectAllSize += Module.second->Table_GlobalData.ClassObjectAllSize;

		NewStringTable.Vector_String.insert(NewStringTable.Vector_String.end(), Module.second->Table_String.Vector_String.begin(), Module.second->Table_String.Vector_String.end());
		NewClassTable.Vector_Class.insert(NewClassTable.Vector_Class.end(), Module.second->Table_Class.Vector_Class.begin(), Module.second->Table_Class.Vector_Class.end());
	}

	FindAddress(NewGlobalDataTable, NewFunctionTable);

	ExeFile.WriteExecuteFile(NewGlobalDataTable, NewStringTable, NewClassTable, NewFunctionTable);
}

void BackendParse::ReplaceStringIndex(ModuleUnit::StringTable& NewStringTable, ModuleUnit::FunctionTable& NewFunctionTable, size_t& FunctionCount)
{
	for (; FunctionCount < NewFunctionTable.Vector_Function.size(); FunctionCount++)
	{
		for (auto& Iter : NewFunctionTable.Vector_Function[FunctionCount].Vector_Instruction)
		{
			for (auto& Operator : Iter.Operator)
			{
				if (Operator.Desc == HazeDataDesc::ConstantString)
				{
					Operator.Extra.Index += (uint32)NewStringTable.Vector_String.size();
					Operator.AddressType = InstructionAddressType::ConstantString;
				}
			}
		}
	}
}

void ResetFunctionBlockOffset(InstructionData& Operator, ModuleUnit::FunctionTableData& Function)
{
	if (Operator.Variable.Name != HAZE_JMP_NULL)
	{
		for (size_t i = 0; i < Function.Vector_Block.size(); i++)
		{
			if (Operator.Variable.Name == Function.Vector_Block[i].BlockName)
			{
				Operator.Extra.Jmp.StartAddress = Function.Vector_Block[i].StartAddress;
				Operator.Extra.Jmp.InstructionNum = Function.Vector_Block[i].InstructionNum;
				break;
			}
		}
	}
}

inline void BackendParse::ResetLocalOperatorAddress(InstructionData& Operator, ModuleUnit::FunctionTableData& Function, std::unordered_map<HAZE_STRING, int>& HashMap_LocalVariable, ModuleUnit::GlobalDataTable& NewGlobalDataTable)
{
	HAZE_STRING ObjName;
	HAZE_STRING MemberName;
	bool IsPointer = false;
	int Index = 0;

	if (IsClassMember(Operator.Desc))
	{
		FindObjectAndMemberName(Operator.Variable.Name, ObjName, MemberName, IsPointer);
		for (uint32 i = 0; i < Function.Vector_Variable.size(); i++)
		{
			if (Function.Vector_Variable[i].Variable.Name == ObjName)
			{
				auto Class = GetClass(Function.Vector_Variable[i].Variable.Type.CustomName);
				if (IsPointer)
				{
					Operator.Extra.Address.BaseAddress = Function.Vector_Variable[i].Offset;
					Operator.Extra.Address.Offset = GetMemberOffset(*Class, MemberName);
					Operator.AddressType = InstructionAddressType::Local_BasePointer_Offset;
					break;
				}
				else
				{
					Operator.Extra.Address.BaseAddress = Function.Vector_Variable[i].Offset + GetMemberOffset(*Class, MemberName);
					Operator.AddressType = InstructionAddressType::Local;
					break;
				}
			}
		}
	}
	else if (Operator.Desc == HazeDataDesc::ArrayElement)
	{
		auto Iter_Index = HashMap_LocalVariable.find(Operator.Variable.Name);
		if (Iter_Index != HashMap_LocalVariable.end())
		{
			//避免先把Index值替换掉
			Operator.Extra.Address.Offset = Operator.Extra.Index * GetSizeByType(Operator.Variable.Type, this);
			Operator.Extra.Address.BaseAddress = Function.Vector_Variable[Iter_Index->second].Offset;
			Operator.AddressType = InstructionAddressType::Local_Base_Offset;
		}
		else if (Function.Vector_Variable[0].Variable.Name.substr(0, 1) == HAZE_CLASS_THIS)
		{
			auto Class = GetClass(Function.Vector_Variable[0].Variable.Type.CustomName);
			if (Class)
			{
				Operator.Extra.Address.Offset = GetMemberOffset(*Class, Operator.Variable.Name) + Operator.Extra.Index * GetSizeByType(Operator.Variable.Type, this);
				Operator.Extra.Address.BaseAddress = Function.Vector_Variable[0].Offset;
				Operator.AddressType = InstructionAddressType::Local_BasePointer_Offset;
			}
			else
			{
				HAZE_LOG_ERR_W("查找变量<%s>的偏移地址错误,当前函数<%s>,当前类<%s>未找到!\n",
					Operator.Variable.Name.c_str(), Function.Name.c_str(), Function.Vector_Variable[0].Variable.Type.CustomName.c_str());
			}
		}
		else
		{
			HAZE_LOG_ERR_W("查找变量<%s>的偏移地址错误,当前函数<%s>!\n", Operator.Variable.Name.c_str(), Function.Name.c_str());
		}
	}
	else
	{
		auto Iter_Index = HashMap_LocalVariable.find(Operator.Variable.Name);
		if (Iter_Index != HashMap_LocalVariable.end())
		{
			Operator.Extra.Address.BaseAddress = Function.Vector_Variable[Iter_Index->second].Offset;
			Operator.AddressType = InstructionAddressType::Local;
		}
		else
		{
			HAZE_LOG_ERR_W("在函数<%s>中查找<%s>的偏移值失败!\n", Function.Name.c_str(), Operator.Variable.Name.c_str());
			return;
		}
	}
}

inline void BackendParse::ResetGlobalOperatorAddress(InstructionData& Operator, ModuleUnit::FunctionTableData& Function, std::unordered_map<HAZE_STRING, int>& HashMap_LocalVariable, ModuleUnit::GlobalDataTable& NewGlobalDataTable)
{
	HAZE_STRING ObjName;
	HAZE_STRING MemberName;
	bool IsPointer = false;
	int Index = 0;

	if (IsClassMember(Operator.Desc))
	{
		FindObjectAndMemberName(Operator.Variable.Name, ObjName, MemberName, IsPointer);
		Index = (int)NewGlobalDataTable.GetIndex(ObjName);
		if (Index >= 0)
		{
			auto Class = GetClass(NewGlobalDataTable.Vector_Data[Index].Type.CustomName);
			if (Class)
			{
				Operator.AddressType = InstructionAddressType::Global_Base_Offset;
				Operator.Extra.Index = Index;
				Operator.Extra.Address.Offset = GetMemberOffset(*Class, MemberName);
			}
		}
		else
		{
			HAZE_LOG_ERR_W("未能查找到全局变量类对象成员<%s>!\n", Operator.Variable.Name.c_str());
			return;
		}
	}
	else if (Operator.Desc == HazeDataDesc::ArrayElement)
	{
		Index = (int)NewGlobalDataTable.GetIndex(Operator.Variable.Name);
		if (Index >= 0)
		{
			//避免先把Index值替换掉
			Operator.Extra.Address.Offset = Operator.Extra.Index * GetSizeByType(Operator.Variable.Type, this);
			Operator.Extra.Index = Index;
			Operator.AddressType = InstructionAddressType::Global_Base_Offset;
		}
	}
	else
	{
		Index = (int)NewGlobalDataTable.GetIndex(Operator.Variable.Name);
		if (Index >= 0)
		{
			Operator.Extra.Index = Index;
			Operator.AddressType = InstructionAddressType::Global;
		}
		else
		{
			if (Operator.Desc == HazeDataDesc::Constant)
			{
				return;
			}

			HAZE_LOG_ERR_W("未能查找到全局变量<%s>!\n", Operator.Variable.Name.c_str());
			return;
		}
	}
}

void BackendParse::FindAddress(ModuleUnit::GlobalDataTable& NewGlobalDataTable, ModuleUnit::FunctionTable& NewFunctionTable)
{
	std::unordered_map<HAZE_STRING, size_t> HashMap_FunctionIndexAndAddress;
	for (size_t i = 0; i < NewFunctionTable.Vector_Function.size(); i++)
	{
		HashMap_FunctionIndexAndAddress[NewFunctionTable.Vector_Function[i].Name] = i;
	}

	//替换变量为索引或相对函数起始偏移
	for (size_t k = 0; k < NewFunctionTable.Vector_Function.size(); ++k)
	{
#if BACKEND_INSTRUCTION_LOG
		std::wcout << NewFunctionTable.Vector_Function[k].Name << std::endl;
#endif
		auto& CurrFunction = NewFunctionTable.Vector_Function[k];

		std::unordered_map<HAZE_STRING, int> HashMap_LocalVariable;
		for (size_t i = 0; i < CurrFunction.Vector_Variable.size(); i++)
		{
			HashMap_LocalVariable[CurrFunction.Vector_Variable[i].Variable.Name] = (int)i;
		}

		for (size_t i = 0; i < CurrFunction.Vector_Instruction.size(); ++i)
		{
			if (IsJmpOpCode(CurrFunction.Vector_Instruction[i].InsCode))
			{
				//设置 Block块 偏移值
				for (auto& Operator : CurrFunction.Vector_Instruction[i].Operator)
				{
					if (Operator.Variable.Name != HAZE_JMP_NULL)
					{
						ResetFunctionBlockOffset(Operator, CurrFunction);
					}
				}
			}
			/*else if (CurrFunction.Vector_Instruction[i].InsCode == InstructionOpCode::CALL
				&& CurrFunction.Vector_Instruction[i].Operator[0].Variable.Type.PrimaryType == HazeValueType::PointerFunction)
			{
				auto Iter_Index = HashMap_Variable.find(CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name);
				if (Iter_Index != HashMap_Variable.end())
				{
					CurrFunction.Vector_Instruction[i].Operator[1].Extra.Address.BaseAddress = CurrFunction.Vector_Variable[Iter_Index->second].Offset;
				}
				else
				{
					int Index = NewGlobalDataTable.GetIndex(CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name);
					if (Index >= 0)
					{
						CurrFunction.Vector_Instruction[i].Operator[1].Extra.Index = (int)Index;
					}
					else
					{
						HAZE_LOG_ERR(HAZE_TEXT("查找调用的函数指针<%s>失败!\n"), CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name.c_str());
					}
				}
			}*/
			else if (!IsIgnoreFindAddressInsCode(CurrFunction.Vector_Instruction[i]))
			{
				HAZE_STRING ObjName;
				HAZE_STRING MemberName;
				bool IsPointer = false;
				bool Find = false;

				for (auto& Operator : CurrFunction.Vector_Instruction[i].Operator)
				{
					if (!IsIgnoreFindAddress(Operator))
					{
						if (IS_SCOPE_LOCAL(Operator.Scope))
						{
							ResetLocalOperatorAddress(Operator, CurrFunction, HashMap_LocalVariable, NewGlobalDataTable);
						}
						else if (IS_SCOPE_GLOBAL(Operator.Scope))
						{
							ResetGlobalOperatorAddress(Operator, CurrFunction, HashMap_LocalVariable, NewGlobalDataTable);
						}
						else if (IS_SCOPE_TEMP(Operator.Scope))
						{
							if (Operator.Desc == HazeDataDesc::FunctionAddress)
							{
								Operator.AddressType = InstructionAddressType::FunctionAddress;
							}
							else
							{
								HAZE_LOG_ERR_W("寻找临时变量<%s>的地址失败!\n", Operator.Variable.Name.c_str());
							}
						}
						else
						{
							HAZE_LOG_ERR_W("寻找变量<%s>的地址失败!\n", Operator.Variable.Name.c_str());
						}
					}
				}
			}

#if BACKEND_INSTRUCTION_LOG
			HAZE_STRING_STREAM WSS;
			WSS << "Replace: " << GetInstructionString(NewFunctionTable.Vector_Function[k].Vector_Instruction[i].InsCode) << " ";
			for (auto& it : NewFunctionTable.Vector_Function[k].Vector_Instruction[i].Operator)
			{
				WSS << it.Variable.Name << " Base " << it.Extra.Address.BaseAddress << " Offset " << it.Extra.Address.Offset << " ";
			}
			WSS << std::endl;
			std::wcout << WSS.str();
#endif // ENABLE_BACKEND_INSTRUCTION_LOG
		}
	}
}

const ModuleUnit::ClassTableData* const BackendParse::GetClass(const HAZE_STRING& ClassName)
{
	for (auto& Module : HashMap_Modules)
	{
		for (auto& Class : Module.second->Table_Class.Vector_Class)
		{
			if (Class.Name == ClassName)
			{
				return &Class;
			}
		}
	}

	return nullptr;
}

uint32 const BackendParse::GetClassSize(const HAZE_STRING& ClassName)
{
	auto Class = GetClass(ClassName);
	if (Class)
	{
		return Class->Size;
	}
	return 0;
}

uint32 BackendParse::GetMemberOffset(const ModuleUnit::ClassTableData& Class, const HAZE_STRING& MemberName)
{
	for (auto& Iter : Class.Vector_Member)
	{
		if (Iter.Variable.Name == MemberName)
		{
			return Iter.Offset;
		}
	}

	return (uint32)-1;
}