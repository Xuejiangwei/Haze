#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeUtility.h"
#include "HazeVM.h"

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

BackendParse::BackendParse(HazeVM* VM) : VM(VM)
{
	CurrCode = nullptr;

	HAZE_STRING Path = std::filesystem::current_path();
	HAZE_STRING OpCodePath = Path + HAZE_TEXT("\\HazeOpCode\\");
	OpCodePath += HAZE_TEXT("Main.Hzb");

	FS_OpCode.open(OpCodePath, std::ios::out | std::ios::binary); //���ö����ƵĻ���д��10���ᵱ�ɻ������⴦��д�������ַ� 0x0d 0x0a�����س����з�
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
	HAZE_STRING Path = std::filesystem::current_path();

	HAZE_STRING I_CodePath;

	auto& Modules = VM->GetModules();

	HAZE_STRING CodeText;

	for (auto& iter : Modules)
	{
		CurrParseModule = std::make_shared<ModuleUnit>();
		HashMap_Modules[iter.first] = CurrParseModule;

		I_CodePath = Path + HAZE_TEXT("\\HazeICode\\");
		I_CodePath += iter.first + HAZE_TEXT(".Hzic");

		HAZE_IFSTREAM FS(I_CodePath);
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


			GetNextLexmeAssign_CustomType<uint32>(Table.Vector_Data[i].Value.Type);

			GetNextLexeme();
			StringToHazeValueNumber(CurrLexeme, Table.Vector_Data[i].Value);
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
				GetNextLexmeAssign_HazeString(Table.Vector_Class[i].Name);

				GetNextLexmeAssign_StandardType(Table.Vector_Class[i].Size);

				GetNextLexmeAssign_StandardType(Num);
				Table.Vector_Class[i].Vector_Member.resize(Num);

				for (size_t j = 0; j < Table.Vector_Class[i].Vector_Member.size(); j++)
				{
					GetNextLexmeAssign_HazeString(Table.Vector_Class[i].Vector_Member[j].Name);

					GetNextLexmeAssign_CustomType<uint32>(Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType);

					if (Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType == HazeValueType::PointerClass 
						|| Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType == HazeValueType::Class)
					{
						GetNextLexmeAssign_HazeString(Table.Vector_Class[i].Vector_Member[j].Type.CustomName);
					}
					else if (Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType == HazeValueType::PointerBase)
					{
						GetNextLexmeAssign_CustomType<uint32>(Table.Vector_Class[i].Vector_Member[j].Type.PointerToType);
					}
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
						GetNextLexmeAssign_CustomType<uint32>(Param.Type.PointerToType);
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
						GetNextLexmeAssign_CustomType<uint32>(Var.Variable.Type.PointerToType);
					}

					GetNextLexmeAssign_CustomType<int>(Var.Offset);

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

	if (Data.Variable.Type.PrimaryType == HazeValueType::PointerBase)
	{
		GetNextLexmeAssign_CustomType<uint32>(Data.Variable.Type.PointerToType);

		if (Data.Scope == HazeDataDesc::ConstantString)
		{
			GetNextLexmeAssign_CustomType<int>(Data.Extra.Index);
		}
	}
	else if (Data.Variable.Type.PrimaryType == HazeValueType::PointerClass || Data.Variable.Type.PrimaryType == HazeValueType::Class)
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
	case InstructionOpCode::ADD:
	case InstructionOpCode::SUB:
	case InstructionOpCode::MUL:
	case InstructionOpCode::DIV:
	case InstructionOpCode::MOD:
	case InstructionOpCode::CMP:
	{
		InstructionData OperatorOne;
		InstructionData OperatorTwo;

		ParseInstructionData(OperatorOne);
		ParseInstructionData(OperatorTwo);

		Instruction.Operator = { OperatorOne, OperatorTwo };
	}
	break;
	case InstructionOpCode::INC:
		break;
	case InstructionOpCode::DEC:
		break;
	case InstructionOpCode::AND:
		break;
	case InstructionOpCode::OR:
		break;
	case InstructionOpCode::NOT:
		break;
	case InstructionOpCode::XOR:
		break;
	case InstructionOpCode::SHL:
		break;
	case InstructionOpCode::SHR:
		break;
	case InstructionOpCode::PUSH:
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
	case InstructionOpCode::POP:
	{
		InstructionData OperatorOne;
		ParseInstructionData(OperatorOne);

		Instruction.Operator = { OperatorOne };
	}
		break;
	case InstructionOpCode::CALL:
	{
		InstructionData OperatorOne;

		OperatorOne.Scope = HazeDataDesc::None;

		GetNextLexmeAssign_HazeString(OperatorOne.Variable.Name);

		GetNextLexmeAssign_CustomType<int>(OperatorOne.Extra.Call.ParamNum);
		GetNextLexmeAssign_CustomType<int>(OperatorOne.Extra.Call.ParamByteSize);

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::NEW:
	{
		InstructionData OperatorOne;

		GetNextLexeme();
		OperatorOne.Scope = HazeDataDesc::None;

		GetNextLexmeAssign_CustomType<uint32>(OperatorOne.Variable.Type.PrimaryType);
		GetNextLexmeAssign_HazeString(OperatorOne.Variable.Name);

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

	for (auto& iter : HashMap_Modules)
	{
		NewGlobalDataTable.Vector_Data.insert(NewGlobalDataTable.Vector_Data.end(), iter.second->Table_GlobalData.Vector_Data.begin(), iter.second->Table_GlobalData.Vector_Data.end());
		NewStringTable.Vector_String.insert(NewStringTable.Vector_String.end(), iter.second->Table_String.Vector_String.begin(), iter.second->Table_String.Vector_String.end());
		NewClassTable.Vector_Class.insert(NewClassTable.Vector_Class.end(), iter.second->Table_Class.Vector_Class.begin(), iter.second->Table_Class.Vector_Class.end());
		NewFunctionTable.Vector_Function.insert(NewFunctionTable.Vector_Function.end(), iter.second->Table_Function.Vector_Function.begin(), iter.second->Table_Function.Vector_Function.end());
	}
	
	ReplaceOffset(NewGlobalDataTable, NewStringTable, NewClassTable, NewFunctionTable);

	//��ȫ������д��
	uint32 UnsignedInt = (uint32)NewGlobalDataTable.Vector_Data.size();
	HAZE_BINARY_STRING BinaryString;
	
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
	for (auto& i : NewGlobalDataTable.Vector_Data)
	{
		BinaryString = WString2String(i.Name);
		UnsignedInt = (uint32)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		
		FS_OpCode.write(BinaryString.c_str(), UnsignedInt);
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Value.Type));
		FS_OpCode.write(GetBinaryPointer(i.Value), GetSizeByHazeType(i.Value.Type));
	}

	//���ַ�����д��
	UnsignedInt = (uint32)NewStringTable.Vector_String.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

	for (auto& i : NewStringTable.Vector_String)
	{
		BinaryString = WString2String(i.String);
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
			BinaryString = WString2String(Iter.Vector_Member[i].Name);
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.data(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Vector_Member[i].Type.PrimaryType));

			BinaryString = WString2String(HAZE_STRING(Iter.Vector_Member[i].Type.CustomName));
			UnsignedInt = (uint32)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.data(), UnsignedInt);
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

void BackendParse::ReplaceOffset(ModuleUnit::GlobalDataTable& NewGlobalDataTable, ModuleUnit::StringTable& NewStringTable,
	ModuleUnit::ClassTable& NewClassTable, ModuleUnit::FunctionTable& NewFunctionTable)
{
	std::unordered_map<HAZE_STRING, size_t> HashMap_FunctionIndexAndAddress;
	for (size_t i = 0; i < NewFunctionTable.Vector_Function.size(); i++)
	{
		HashMap_FunctionIndexAndAddress[NewFunctionTable.Vector_Function[i].Name] = i;
	}

	//�滻����Ϊ��������Ժ�����ʼƫ��
	for (size_t k = 0; k < NewFunctionTable.Vector_Function.size(); ++k)
	{
		auto& CurrFunction = NewFunctionTable.Vector_Function[k];
		for (size_t i = 0; i < CurrFunction.Vector_Instruction.size(); ++i)
		{
			if (IsJmpOpCode(CurrFunction.Vector_Instruction[i].InsCode))
			{
				for (auto& Operator : CurrFunction.Vector_Instruction[i].Operator)
				{
					if (Operator.Variable.Name != HAZE_JMP_NULL && Operator.Variable.Name != HAZE_JMP_OUT)
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
			else
			{
				for (auto& it : CurrFunction.Vector_Instruction[i].Operator)
				{
					auto& VariableName = it.Variable.Name;
					if (it.Scope == HazeDataDesc::Global)
					{
						it.Extra.Index = NewGlobalDataTable.GetIndex(VariableName);
						it.AddressType = InstructionAddressType::Index;
					}
					else if (it.Scope == HazeDataDesc::ClassMember_Local_Public)
					{
						int AddressOffset = 0 - HAZE_ADDRESS_SIZE;	 	//��ջ�ķ��ص�ַ����ռ�ÿռ�Ϊ4���ֽ�

						HAZE_STRING ObjName;
						HAZE_STRING MemberName;
						bool IsPointer = false;
						FindObjectAndMemberName(VariableName, ObjName, MemberName, IsPointer);

						bool Find = false;
						for (int j = (int)CurrFunction.Vector_Param.size() - 1; j >= 0; --j)
						{
							AddressOffset -= GetSizeByType(CurrFunction.Vector_Param[j].Type, this);
							if (CurrFunction.Vector_Param[j].Name == ObjName)
							{
								auto Class = GetClass(CurrFunction.Vector_Param[j].Type.CustomName);
								if (IsPointer)
								{
									//��Ҫ�洢һ��ָ��ָ���ַ�����ƫ��
									it.Extra.Address.BaseAddress = AddressOffset;
									it.Extra.Address.Offset = GetMemberOffset(*Class, MemberName);
									it.AddressType = InstructionAddressType::Pointer_Offset;
									Find = true;
									break;
								}
								else
								{
									it.Extra.Address.BaseAddress = AddressOffset += GetMemberOffset(*Class, MemberName);
									Find = true;
									break;
								}
							}
						}

						if (Find)
						{
							continue;
						}

						int Offset = 0;
						for (uint32 j = 0; j < i; j++)
						{
							if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::PUSH)
							{
								if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name == ObjName)
								{
									auto Class = GetClass(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Type.CustomName);
									it.Extra.Address.BaseAddress = Offset + GetMemberOffset(*Class, MemberName);
									break;
								}
								else
								{
									Offset += GetSizeByType(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Type, this);
								}
							}
							else if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::POP)
							{
								Offset -= GetSizeByType(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Type, this);
							}
							else if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::CALL)
							{
								Offset -= HAZE_ADDRESS_SIZE;
								for (auto& Param : NewFunctionTable.Vector_Function[HashMap_FunctionIndexAndAddress[NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name]].Vector_Param)
								{
									Offset -= GetSizeByType(Param.Type, this);
								}
							}
						}
					}
					else if (it.Scope == HazeDataDesc::Local /*|| it.Scope == HazeDataDesc::Temp*/ || it.Scope == HazeDataDesc::ClassThis)
					{
						//��Ҫ���ж��ǲ��Ǻ�������������Ϊ��������
						int AddressOffset = 0 - HAZE_ADDRESS_SIZE;	 	//��ջ�ķ��ص�ַ����ռ�ÿռ�Ϊ4���ֽ�
						bool Find = false;
						for (int j = (int)NewFunctionTable.Vector_Function[k].Vector_Param.size() - 1; j >= 0; --j)
						{
							AddressOffset -= GetSizeByHazeType(NewFunctionTable.Vector_Function[k].Vector_Param[j].Type.PrimaryType);
							if (NewFunctionTable.Vector_Function[k].Vector_Param[j].Name == VariableName)
							{
								it.Extra.Address.BaseAddress = AddressOffset;
								Find = true;
								break;
							}
						}

						if (Find)
						{
							continue;
						}


						int Offset = 0;
						for (uint32 j = 0; j < i; j++)
						{
							if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::PUSH)
							{
								if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name == VariableName)
								{
									it.Extra.Address.BaseAddress = Offset;
									break;
								}
								else
								{
									Offset += GetSizeByType(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Type, this);
								}
							}
							else if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::POP)
							{
								Offset -= GetSizeByType(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Type, this);
							}
							else if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::CALL)
							{
								Offset -= HAZE_ADDRESS_SIZE;
								for (auto& Param : NewFunctionTable.Vector_Function[HashMap_FunctionIndexAndAddress[NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name]].Vector_Param)
								{
									Offset -= GetSizeByType(Param.Type, this);
								}
							}
						}
					}
				}
			}

#if BACKEND_INSTRUCTION_LOG
			HAZE_STRING_STREAM WSS;
			WSS << GetInstructionString(NewFunctionTable.Vector_Function[k].Vector_Instruction[i].InsCode) << " ";
			for (auto& it : NewFunctionTable.Vector_Function[k].Vector_Instruction[i].Operator)
			{
				WSS << it.Variable.Name << " Base " << it.Extra.Address.BaseAddress << " Offset " << it.Extra.Address.Offset << " ";
			}
			WSS << "       Replace" << std::endl;
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

		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Scope));						//������������

		BinaryString = WString2String(Iter.Variable.Type.CustomName);
		UnsignedInt = (uint32)BinaryString.size();
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		B_OFS.write(BinaryString.data(), UnsignedInt);								//����������2

		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Extra.Index));								//����������
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.AddressType));
		
		if (Iter.Scope == HazeDataDesc::ClassMember_Local_Public || IsJmpOpCode(Instruction.InsCode))
		{
			B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Extra.Address.Offset));						//��������ַƫ��
		}
	}

	if (Instruction.InsCode == InstructionOpCode::CALL)
	{
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Instruction.Operator[0].Extra.Call.ParamByteSize));
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << GetInstructionString(Instruction.InsCode) << " ";
	for (auto& it : Instruction.Operator)
	{
		WSS << it.Variable.Name << " " << (uint32)it.Variable.Type.PrimaryType << " " << (uint32)it.Scope << " " << it.Extra.Index << " ";
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
	uint32 Offset = 0;
	for (auto& Iter : Class.Vector_Member)
	{
		if (Iter.Name != MemberName)
		{
			Offset += GetSizeByType(Iter.Type, this);
		}
		else
		{
			return Offset;
		}
	}

	return (uint32)-1;
}
