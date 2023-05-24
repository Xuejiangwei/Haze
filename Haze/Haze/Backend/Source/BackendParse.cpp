#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeUtility.h"
#include "HazeVM.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

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

BackendParse::BackendParse(HazeVM* VM) : VM(VM), CurrCode(nullptr)
{
	//���ö����ƵĻ���д��10���ᵱ�ɻ������⴦��д�������ַ� 0x0d 0x0a�����س����з�
	FS_OpCode.open(GetMainBinaryFilePath(), std::ios::out | std::ios::binary); 
}

BackendParse::~BackendParse()
{
	if (FS_OpCode.is_open())
	{
		FS_OpCode.close();
	}
}

void BackendParse::Parse()
{
	HAZE_STRING I_CodePath;

	auto& Modules = VM->GetReferenceModules();

	HAZE_STRING CodeText;

	for (auto& Module : Modules)
	{
		CurrParseModule = std::make_shared<ModuleUnit>();
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
	CurrParseModule->IsStdLib = StringToStandardType<uint32>(CurrLexeme);

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


			GetNextLexmeAssign_CustomType<uint32>(Table.Vector_Data[i].Type.PrimaryType);

			GetNextLexeme();
			StringToHazeValueNumber(CurrLexeme, Table.Vector_Data[i].Type.PrimaryType, Table.Vector_Data[i].Value);
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
				Table.Vector_Function[i].DescType = (InstructionFunctionType)CurrParseModule->IsStdLib;

				GetNextLexmeAssign_HazeString(Table.Vector_Function[i].Name);

				GetNextLexmeAssign_CustomType<uint32>(Table.Vector_Function[i].Type);

				GetNextLexeme();
				while (CurrLexeme == GetFunctionParamHeader())
				{
					HazeDefineVariable Param;
					GetNextLexmeAssign_HazeString(Param.Name);

					GetNextLexmeAssign_CustomType<uint32>(Param.Type.PrimaryType);

					if (Param.Type.PrimaryType == HazeValueType::PointerClass || Param.Type.PrimaryType == HazeValueType::Class)
					{
						GetNextLexmeAssign_HazeString(Param.Type.CustomName);

					}
					else if (Param.Type.PrimaryType == HazeValueType::PointerBase)
					{
						GetNextLexmeAssign_CustomType<uint32>(Param.Type.SecondaryType);
					}

					Table.Vector_Function[i].Vector_Param.push_back(std::move(Param));

					GetNextLexeme();
				}

				while (CurrLexeme == GetFunctionVariableHeader())
				{
					HazeLocalVariable Var;
					GetNextLexmeAssign_HazeString(Var.Variable.Name);

					GetNextLexmeAssign_CustomType<uint32>(Var.Variable.Type.PrimaryType);

					if (Var.Variable.Type.PrimaryType == HazeValueType::PointerClass || Var.Variable.Type.PrimaryType == HazeValueType::Class)
					{
						GetNextLexmeAssign_HazeString(Var.Variable.Type.CustomName);

					}
					else if (Var.Variable.Type.PrimaryType == HazeValueType::PointerBase)
					{
						GetNextLexmeAssign_CustomType<uint32>(Var.Variable.Type.SecondaryType);
					}

					GetNextLexmeAssign_CustomType<int>(Var.Offset);
					GetNextLexmeAssign_CustomType<uint32>(Var.Size);

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

void BackendParse::ParseInstructionData(InstructionData& Data, bool ParsePrimaryType)
{
	if (ParsePrimaryType)
	{
		GetNextLexmeAssign_CustomType<uint32>(Data.Variable.Type.PrimaryType);
	}
	
	GetNextLexmeAssign_HazeString(Data.Variable.Name);
	GetNextLexmeAssign_CustomType<uint32>(Data.Scope);

	if (Data.Scope == HazeDataDesc::ArrayElement)
	{
		GetNextLexmeAssign_CustomType<uint32>(Data.Extra.Index);
	}

	if ((Data.Variable.Type.PrimaryType == HazeValueType::PointerBase || Data.Variable.Type.PrimaryType == HazeValueType::ReferenceBase)
		&& Data.Scope != HazeDataDesc::NullPtr)
	{
		GetNextLexmeAssign_CustomType<uint32>(Data.Variable.Type.SecondaryType);

		if (Data.Scope == HazeDataDesc::ConstantString)
		{
			GetNextLexmeAssign_CustomType<int>(Data.Extra.Index);
		}
	}
	else if (Data.Variable.Type.PrimaryType == HazeValueType::PointerClass || Data.Variable.Type.PrimaryType == HazeValueType::ReferenceClass
		|| Data.Variable.Type.PrimaryType == HazeValueType::Class)
	{
		GetNextLexmeAssign_HazeString(Data.Variable.Type.CustomName);
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
	case InstructionOpCode::NOT:
	case InstructionOpCode::BIT_NEG:
	case InstructionOpCode::PUSH:
	case InstructionOpCode::POP:
	{
		InstructionData OperatorOne;
		ParseInstructionData(OperatorOne);

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::RET:
	{
		InstructionData OperatorOne;

		GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Variable.Type.PrimaryType);

		if (OperatorOne.Variable.Type.PrimaryType == HazeValueType::Void)
		{
			OperatorOne.Variable.Name = HAZE_TEXT("Void");
			OperatorOne.Scope = HazeDataDesc::None;
		}
		else
		{
			ParseInstructionData(OperatorOne, false);
		}

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::CALL:
	{
		InstructionData OperatorOne;


		GetNextLexmeAssign_HazeString(OperatorOne.Variable.Name);

		GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Variable.Type.PrimaryType);
		
		GetNextLexmeAssign_CustomType<int>(OperatorOne.Extra.Call.ParamNum);
		GetNextLexmeAssign_CustomType<int>(OperatorOne.Extra.Call.ParamByteSize);

		if (OperatorOne.Variable.Type.PrimaryType == HazeValueType::PointerFunction)
		{
			Instruction.Operator = { OperatorOne, InstructionData() };
		}
		else
		{
			Instruction.Operator = { OperatorOne };
		}
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
	case InstructionOpCode::JMPL:
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
	default:
		break;
	}
}

void BackendParse::GenOpCodeFile()
{
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
		NewStringTable.Vector_String.insert(NewStringTable.Vector_String.end(), Module.second->Table_String.Vector_String.begin(), Module.second->Table_String.Vector_String.end());
		NewClassTable.Vector_Class.insert(NewClassTable.Vector_Class.end(), Module.second->Table_Class.Vector_Class.begin(), Module.second->Table_Class.Vector_Class.end());
	}
	
	FindAddress(NewGlobalDataTable, NewFunctionTable);

	//��ȫ������д��
	uint32 UnsignedInt = (uint32)NewGlobalDataTable.Vector_Data.size();
	HAZE_BINARY_STRING BinaryString;
	
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
	for (auto& Iter : NewGlobalDataTable.Vector_Data)
	{
		BinaryString = WString2String(Iter.Name);
		UnsignedInt = (uint32)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		
		FS_OpCode.write(BinaryString.c_str(), UnsignedInt);
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Type.PrimaryType));
		FS_OpCode.write(GetBinaryPointer(Iter.Type.PrimaryType, Iter.Value), GetSizeByHazeType(Iter.Type.PrimaryType));
	}

	//���ַ�����д��
	UnsignedInt = (uint32)NewStringTable.Vector_String.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

	for (auto& Iter : NewStringTable.Vector_String)
	{
		BinaryString = WString2String(Iter.String);
		UnsignedInt = (uint32)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		FS_OpCode.write(BinaryString.data(), UnsignedInt);
	}

	//�����д��
	UnsignedInt = (uint32)NewClassTable.Vector_Class.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

	for (auto& Iter : NewClassTable.Vector_Class)
	{
		BinaryString = WString2String(Iter.Name);
		UnsignedInt = (uint32)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		FS_OpCode.write(BinaryString.data(), UnsignedInt);

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Size));

		UnsignedInt = (uint32)Iter.Vector_Member.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

		for (size_t i = 0; i < Iter.Vector_Member.size(); i++)
		{
			BinaryString = WString2String(Iter.Vector_Member[i].Variable.Name);
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.data(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Vector_Member[i].Variable.Type.PrimaryType));

			BinaryString = WString2String(HAZE_STRING(Iter.Vector_Member[i].Variable.Type.CustomName));
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.data(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Vector_Member[i].Offset));
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Vector_Member[i].Size));
		}
	}

	//��������д��
	UnsignedInt = (uint32)NewFunctionTable.Vector_Function.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

	uint32 InstructionStartAddr = 0;
	for (auto& Function : NewFunctionTable.Vector_Function)
	{
		BinaryString = WString2String(Function.Name);
		UnsignedInt = (uint32)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		FS_OpCode.write(BinaryString.data(), UnsignedInt);

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Function.Type));
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Function.DescType));

		UnsignedInt = (uint32)Function.Vector_Param.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		for (auto& Iter : Function.Vector_Param)
		{
			BinaryString = WString2String(Iter.Name);
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.c_str(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Type.PrimaryType));
			BinaryString = WString2String(Iter.Type.CustomName);
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.c_str(), UnsignedInt);
		}

		UnsignedInt = (uint32)Function.Vector_Variable.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		for (auto& Iter : Function.Vector_Variable)
		{
			BinaryString = WString2String(Iter.Variable.Name);
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.c_str(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Variable.Type.PrimaryType));
			BinaryString = WString2String(Iter.Variable.Type.CustomName);
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.c_str(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Offset));
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Size));
		}


		//д��ָ��������ʼ��ַ
		UnsignedInt = (uint32)Function.Vector_Instruction.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(InstructionStartAddr));
		InstructionStartAddr += UnsignedInt;
	}

	//д��ָ��
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(InstructionStartAddr));  //ָ���ܸ���
	for (auto& i : NewFunctionTable.Vector_Function)
	{
		for (auto& iter : i.Vector_Instruction)
		{
			WriteInstruction(FS_OpCode, iter);
		}
	}
}

void BackendParse::ReplaceStringIndex(ModuleUnit::StringTable& NewStringTable, ModuleUnit::FunctionTable& NewFunctionTable, size_t& FunctionCount)
{
	for (; FunctionCount < NewFunctionTable.Vector_Function.size(); FunctionCount++)
	{
		for (auto& Iter : NewFunctionTable.Vector_Function[FunctionCount].Vector_Instruction)
		{
			for (auto& Operator : Iter.Operator)
			{
				if (Operator.Scope == HazeDataDesc::ConstantString)
				{
					Operator.Extra.Index += (uint32)NewStringTable.Vector_String.size();
				}
			}
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

	//�滻����Ϊ��������Ժ�����ʼƫ��
	for (size_t k = 0; k < NewFunctionTable.Vector_Function.size(); ++k)
	{
#if BACKEND_INSTRUCTION_LOG
		std::wcout << NewFunctionTable.Vector_Function[k].Name << std::endl;
#endif
		auto& CurrFunction = NewFunctionTable.Vector_Function[k];

		std::unordered_map<HAZE_STRING, int> HashMap_Variable;
		for (size_t i = 0; i < CurrFunction.Vector_Variable.size(); i++)
		{
			HashMap_Variable[CurrFunction.Vector_Variable[i].Variable.Name] = (int)i;
		}

		for (size_t i = 0; i < CurrFunction.Vector_Instruction.size(); ++i)
		{
			if (IsJmpOpCode(CurrFunction.Vector_Instruction[i].InsCode))
			{
				//���� Block�� ƫ��ֵ
				for (auto& Operator : CurrFunction.Vector_Instruction[i].Operator)
				{
					if (Operator.Variable.Name != HAZE_JMP_NULL)
					{
						for (size_t j = 0; j < CurrFunction.Vector_Block.size(); j++)
						{
							if (Operator.Variable.Name == CurrFunction.Vector_Block[j].BlockName)
							{
								Operator.Extra.Jmp.StartAddress = CurrFunction.Vector_Block[j].StartAddress;
								Operator.Extra.Jmp.InstructionNum = CurrFunction.Vector_Block[j].InstructionNum;

								break;
							}
						}
					}
				}
			}
			else if (CurrFunction.Vector_Instruction[i].InsCode == InstructionOpCode::CALL 
				&& CurrFunction.Vector_Instruction[i].Operator[0].Variable.Type.PrimaryType == HazeValueType::PointerFunction)
			{
				auto Iter_Index = HashMap_Variable.find(CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name);
				if (Iter_Index != HashMap_Variable.end())
				{
					CurrFunction.Vector_Instruction[i].Operator[1].Extra.Address.BaseAddress = CurrFunction.Vector_Variable[Iter_Index->second].Offset;
				}
				else
				{
					uint64 Index = NewGlobalDataTable.GetIndex(CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name);
					if (Index != (uint64)-1)
					{

						CurrFunction.Vector_Instruction[i].Operator[1].Extra.Index = (int)Index;
					}
					else
					{
						HAZE_LOG_ERR(HAZE_TEXT("���ҵ��õĺ���ָ��<%s>ʧ��!\n"), CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name.c_str());
					}
				}
			}
			else
			{
				for (auto& it : CurrFunction.Vector_Instruction[i].Operator)
				{
					auto& VariableName = it.Variable.Name;
					if (it.Scope == HazeDataDesc::ArrayElement)
					{
						auto Iter_Index = HashMap_Variable.find(it.Variable.Name);
						if (Iter_Index != HashMap_Variable.end())
						{
							it.Extra.Address.Offset = it.Extra.Index * GetSizeByType(it.Variable.Type, this);
							it.Extra.Address.BaseAddress = CurrFunction.Vector_Variable[Iter_Index->second].Offset;
						}
						else if (CurrFunction.Vector_Variable[0].Variable.Name.substr(0, 1) == HAZE_CLASS_THIS)
						{
							auto Class = GetClass(CurrFunction.Vector_Variable[0].Variable.Type.CustomName);
							if (Class)
							{
								it.Extra.Address.BaseAddress = CurrFunction.Vector_Variable[0].Offset;
								it.Extra.Address.Offset = GetMemberOffset(*Class, it.Variable.Name);
								it.AddressType = InstructionAddressType::Pointer_Offset;
							}
							else
							{
								HAZE_LOG_ERR(HAZE_TEXT("���ұ���<%s>��ƫ�Ƶ�ַ����,��ǰ����<%s>,��ǰ��<%s>δ�ҵ�!\n"),
									it.Variable.Name.c_str(), CurrFunction.Name.c_str(), CurrFunction.Vector_Variable[0].Variable.Type.CustomName.c_str());
							}
						}
						else
						{
							HAZE_LOG_ERR(HAZE_TEXT("���ұ���<%s>��ƫ�Ƶ�ַ����,��ǰ����<%s>!\n"), it.Variable.Name.c_str(), CurrFunction.Name.c_str());
						}
					}
					else if (IsClassMember(it.Scope))
					{
						HAZE_STRING ObjName;
						HAZE_STRING MemberName;
						bool IsPointer = false;
						bool Find = false;
						{
							FindObjectAndMemberName(VariableName, ObjName, MemberName, IsPointer);

							for (uint32 j = 0; j < CurrFunction.Vector_Variable.size(); j++)
							{
								if (CurrFunction.Vector_Variable[j].Variable.Name == ObjName)
								{
									auto Class = GetClass(CurrFunction.Vector_Variable[j].Variable.Type.CustomName);
									if (IsPointer)
									{
										//��Ҫ�洢һ��ָ��ָ���ַ�����ƫ��
										it.Extra.Address.BaseAddress = CurrFunction.Vector_Variable[j].Offset;
										it.Extra.Address.Offset = GetMemberOffset(*Class, MemberName);
										it.AddressType = InstructionAddressType::Pointer_Offset;
										Find = true;
										break;
									}
									else
									{
										it.Extra.Address.BaseAddress = CurrFunction.Vector_Variable[j].Offset + GetMemberOffset(*Class, MemberName);
										Find = true;
										break;
									}
								}
							}

							if (Find)
							{
								continue;
							}
						}
					}
					else if (it.Scope == HazeDataDesc::Local || it.Scope == HazeDataDesc::ClassThis)
					{
						it.AddressType = InstructionAddressType::Address_Offset;

						auto Iter_Index = HashMap_Variable.find(it.Variable.Name);
						if (Iter_Index != HashMap_Variable.end())
						{
							it.Extra.Address.BaseAddress = CurrFunction.Vector_Variable[Iter_Index->second].Offset;
						}
						else
						{
							HAZE_LOG_ERR(HAZE_TEXT("�ں���<%s>�в���<%s>��ƫ��ֵʧ��!\n"), CurrFunction.Name.c_str(), it.Variable.Name.c_str());
						}
					}
					else if (it.Scope == HazeDataDesc::Global)
					{
						it.Extra.Index = (int)NewGlobalDataTable.GetIndex(it.Variable.Name);
					}
				}
			}

#if BACKEND_INSTRUCTION_LOG
			HAZE_STRING_STREAM WSS;
			WSS << "Replace: " <<GetInstructionString(NewFunctionTable.Vector_Function[k].Vector_Instruction[i].InsCode) << " ";
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

void BackendParse::WriteInstruction(HAZE_BINARY_OFSTREAM& B_OFS, ModuleUnit::FunctionInstruction& Instruction)
{
	uint32 UnsignedInt = 0;
	HAZE_BINARY_STRING BinaryString;

	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Instruction.InsCode));				//�ֽ���

	UnsignedInt = (uint32)Instruction.Operator.size();										
	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));						//����������

	//std::pair<InstructionDataType, std::pair<HAZE_STRING, uint>>;
	for (auto& Iter : Instruction.Operator)
	{
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Variable.Type.PrimaryType));		//����������

		BinaryString = WString2String(Iter.Variable.Name);
		UnsignedInt = (uint32)BinaryString.size();
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		B_OFS.write(BinaryString.data(), UnsignedInt);								//����������

		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Scope));								//������������
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Variable.Type.SecondaryType));		//������

		BinaryString = WString2String(Iter.Variable.Type.CustomName);
		UnsignedInt = (uint32)BinaryString.size();
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		B_OFS.write(BinaryString.data(), UnsignedInt);								//����������2

		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Extra.Index));								//����������
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.AddressType));
		
		if (IsClassMember(Iter.Scope) || IsJmpOpCode(Instruction.InsCode) || Iter.Scope == HazeDataDesc::ArrayElement
			|| (Instruction.InsCode == InstructionOpCode::CALL && &Iter == &Instruction.Operator[0]))
		{
			B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Extra.Address.Offset));						//��������ַƫ��
		}
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << "Write :" << GetInstructionString(Instruction.InsCode);
	for (auto& it : Instruction.Operator)
	{
		WSS << " ----" << it.Variable.Name << " Type :" << (uint32)it.Variable.Type.PrimaryType << " Scope: " << (uint32)it.Scope << " Base: " << it.Extra.Index
			<< " Offset: " << it.Extra.Address.Offset;
	}
	WSS << std::endl;

	std::wcout << WSS.str();
#endif
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
