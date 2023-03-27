#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeVM.h"

#include "HazeLog.h"

BackendParse::BackendParse(HazeVM* VM) : VM(VM)
{
	CurrCode = nullptr;

	HAZE_STRING Path = std::filesystem::current_path();
	HAZE_STRING OpCodePath = Path + HAZE_TEXT("\\HazeOpCode\\");
	OpCodePath += HAZE_TEXT("Main.Hzb");

	FS_OpCode.open(OpCodePath);
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
	while (isspace(*CurrCode))
	{
		CurrCode++;
	}

	CurrLexeme.clear();
	while (!isspace(*CurrCode))
	{
		CurrLexeme += *(CurrCode++);
	}
}

void BackendParse::Parse_I_Code()
{
	//Standard lib
	GetNextLexeme();
	CurrParseModule->IsStdLib = StringToStandardType<unsigned int>(CurrLexeme);

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
		unsigned int Num = StringToStandardType<unsigned int>(CurrLexeme);

		ModuleUnit::GlobalDataTable& Table = CurrParseModule->Table_GlobalData;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextLexmeAssign_HazeString(Table.Vector_Data[i].Name);


			GetNextLexmeAssign_CustomType<unsigned int>(Table.Vector_Data[i].Value.Type);

			GetNextLexeme();
			StringToHazeValueNumber(CurrLexeme, Table.Vector_Data[i].Value);
		}
	}
}

void BackendParse::Parse_I_Code_StringTable()
{
	if (CurrLexeme == GetStringTableHeaderString())
	{
		unsigned int Num;
		GetNextLexmeAssign_StandardType(Num);

		ModuleUnit::StringTable& Table = CurrParseModule->Table_String;
		Table.Vector_String.resize(Num);

		for (size_t i = 0; i < Table.Vector_String.size(); i++)
		{
			GetNextLexmeAssign_HazeString(Table.Vector_String[i].String);
		}
	}
}

void BackendParse::Parse_I_Code_ClassTable()
{
	if (CurrLexeme == GetClassTableHeaderString())
	{
		unsigned int Num;
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

					GetNextLexmeAssign_CustomType<unsigned int>(Table.Vector_Class[i].Vector_Member[j].Type.Type);

					GetNextLexmeAssign_StandardType(Num);
					if (Num > 0)
					{
						GetNextLexmeAssign_HazeString(Table.Vector_Class[i].Vector_Member[j].Type.CustomName);
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
		unsigned int Num;
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

				GetNextLexmeAssign_CustomType<unsigned int>(Table.Vector_Function[i].Type);

				GetNextLexeme();
				while (CurrLexeme == GetFunctionParamHeader())
				{
					HazeDefineVariable Param;
					GetNextLexmeAssign_HazeString(Param.Name);

					GetNextLexmeAssign_CustomType<unsigned int>(Param.Type.Type);

					if (Param.Type.Type == HazeValueType::Pointer || Param.Type.Type == HazeValueType::Class)
					{
						GetNextLexeme();
						Param.Type.CustomName = CurrLexeme;
					}

					Table.Vector_Function[i].Vector_Param.push_back(std::move(Param));

					GetNextLexeme();
				}

				if (CurrLexeme == GetFunctionStartHeader())
				{
					GetNextLexeme();
					while (CurrLexeme != GetFunctionEndHeader())
					{
						ModuleUnit::FunctionInstruction Instruction;
						Instruction.InsCode = GetInstructionByString(CurrLexeme);

						ParseInstruction(Instruction);

						Table.Vector_Function[i].Vector_Instruction.push_back(std::move(Instruction));

						GetNextLexeme();
					}
				}
			}
		}
	}
}

void BackendParse::ParseInstructionData(InstructionData& Data)
{
	GetNextLexmeAssign_CustomType<unsigned int>(Data.Type);
	GetNextLexmeAssign_HazeString(Data.Name);
	GetNextLexmeAssign_CustomType<unsigned int>(Data.Scope);
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

		GetNextLexeme();
		OperatorOne.Type = (HazeValueType)StringToStandardType<unsigned int>(CurrLexeme);

		if (OperatorOne.Type == HazeValueType::Void)
		{
			OperatorOne.Name = HAZE_TEXT("Void");
			OperatorOne.Scope = InstructionScopeType::None;
		}
		else
		{
			GetNextLexeme();
			OperatorOne.Name = CurrLexeme;

			GetNextLexeme();
			OperatorOne.Scope = (InstructionScopeType)StringToStandardType<unsigned int>(CurrLexeme);
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

		GetNextLexeme();
		OperatorOne.Scope = InstructionScopeType::None;
		OperatorOne.Name = CurrLexeme;

		GetNextLexeme();
		OperatorOne.Extra.FunctionCallParamNum = StringToStandardType<int>(CurrLexeme);

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::NEW:
	{
		InstructionData OperatorOne;

		GetNextLexeme();
		OperatorOne.Scope = InstructionScopeType::None;
		OperatorOne.Type = (HazeValueType)StringToStandardType<int>(CurrLexeme); 

		GetNextLexeme();
		OperatorOne.Name = CurrLexeme;

		Instruction.Operator = { OperatorOne };
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
			for (auto& it : CurrFunction.Vector_Instruction[i].Operator)
			{
				if (it.Scope == InstructionScopeType::Global)
				{
					it.Extra.Index = NewGlobalDataTable.GetIndex(it.Name);
					it.Extra.AddressOffset = 0;
				}
				else if (it.Scope == InstructionScopeType::ClassMember)
				{
					int AddressOffset = 0 - HAZE_ADDRESS_SIZE;	 	//��ջ�ķ��ص�ַ����ռ�ÿռ�Ϊ4���ֽ�
					
					HAZE_STRING ObjName;
					HAZE_STRING MemberName;
					size_t Pos =  it.Name.find(HAZE_CLASS_POINTER_ATTR);
					bool IsPointer = true;
					if (Pos != HAZE_STRING::npos)
					{
						ObjName = it.Name.substr(0, Pos);
						MemberName = it.Name.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());
					}
					else
					{
						Pos = it.Name.find(HAZE_CLASS_ATTR);
						if (Pos != HAZE_STRING::npos)
						{
							ObjName = it.Name.substr(0, Pos);
							MemberName = it.Name.substr(Pos + HAZE_STRING(HAZE_CLASS_ATTR).size());
							IsPointer = false;
						}
					}

					for (int j = (int)CurrFunction.Vector_Param.size() - 1; j >= 0; --j)
					{
						const ModuleUnit::ClassTableData* Class = nullptr;
						if (CurrFunction.Vector_Param[j].Type.CustomName.empty())
						{
							AddressOffset -= GetSizeByType(CurrFunction.Vector_Param[j].Type.Type);
						}
						else
						{
							Class = GetClass(CurrFunction.Vector_Param[j].Type.CustomName);
							AddressOffset -= Class->Size;
						}
						  
						if (CurrFunction.Vector_Param[j].Name == ObjName)
						{
							if (IsPointer)
							{
								//��Ҫ�洢һ��ָ��ָ���ַ�����ƫ��
								AddressOffset -= GetSizeByType(CurrFunction.Vector_Param[j].Type.Type);
								it.Extra.Address = AddressOffset;
								it.Extra.AddressOffset = GetMemberOffset(*Class, MemberName);
								break;
							}
							else
							{
								AddressOffset -= GetMemberOffset(*Class, MemberName);
								it.Extra.Address = AddressOffset - GetMemberOffset(*Class, MemberName);
								it.Extra.AddressOffset = 0;
								break;
							}
						}
					}
				}
				else if (it.Scope == InstructionScopeType::Local || it.Scope == InstructionScopeType::Temp)
				{
					//��Ҫ���ж��ǲ��Ǻ�������������Ϊ��������
					int AddressOffset = 0 - HAZE_ADDRESS_SIZE;	 	//��ջ�ķ��ص�ַ����ռ�ÿռ�Ϊ4���ֽ�
					bool Find = false;
					for (int j = (int)NewFunctionTable.Vector_Function[k].Vector_Param.size() - 1; j >= 0 ; --j)
					{
						AddressOffset -= GetSizeByType(NewFunctionTable.Vector_Function[k].Vector_Param[j].Type.Type);
						if (NewFunctionTable.Vector_Function[k].Vector_Param[j].Name == it.Name)
						{
							it.Extra.Address = AddressOffset;
							it.Extra.AddressOffset = 0;
							Find = true;
							break;
						}
					}

					if (Find)
					{
						continue;
					}


					int Offset = 0;
					for (unsigned int j = 0; j < i; j++)
					{
						if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::PUSH)
						{
							if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Name == it.Name)
							{
								it.Extra.Address = Offset;
								it.Extra.AddressOffset = 0;
								break;
							}
							else
							{
								Offset+=GetSizeByType(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Type);
							}
						}
						else if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::POP)
						{
							Offset -= GetSizeByType(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Type);;
						}
						else if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::CALL)
						{
							static int AddressOffset = 1;

							for (auto& Param : NewFunctionTable.Vector_Function[HashMap_FunctionIndexAndAddress[NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Name]].Vector_Param)
							{
								Offset -= GetSizeByType(Param.Type.Type);

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
				WSS << it.Name << " " << it.Extra.Address << " ";
			}
			WSS << std::endl;

			std::wcout << WSS.str();
#endif // ENABLE_BACKEND_INSTRUCTION_LOG
		}
	}

	//��ȫ������д��
	unsigned int Num = (unsigned int)NewGlobalDataTable.Vector_Data.size();
	HAZE_BINARY_STRING BinaryString;
	
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
	for (auto& i : NewGlobalDataTable.Vector_Data)
	{
		BinaryString = WString2String(i.Name);
		Num = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		
		FS_OpCode.write(BinaryString.c_str(), Num);
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Value.Type));
		FS_OpCode.write(GetBinaryPointer(i.Value), GetSizeByType(i.Value.Type));
	}

	//���ַ�����д��
	Num = (unsigned int)NewStringTable.Vector_String.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));

	for (auto& i : NewStringTable.Vector_String)
	{	
		/*BinaryString = WString2String(i.Name);
		Num = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		FS_OpCode.write(BinaryString.data(), Num);*/

		BinaryString = WString2String(i.String);
		Num = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		FS_OpCode.write(BinaryString.data(), Num);
	}

	//��������д��
	Num = (unsigned int)NewFunctionTable.Vector_Function.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));

	unsigned int InstructionStartAddr = 0;
	for (auto& i : NewFunctionTable.Vector_Function)
	{
		BinaryString = WString2String(i.Name);
		Num = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		FS_OpCode.write(BinaryString.data(), Num);

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Type));
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.DescType));

		Num = (unsigned int)i.Vector_Param.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		for (auto& iter : i.Vector_Param)
		{
			BinaryString = WString2String(iter.Name);
			Num = (unsigned int)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
			FS_OpCode.write(BinaryString.c_str(), Num);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(iter.Type.Type));
			BinaryString = WString2String(iter.Type.CustomName);
			Num = (unsigned int)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
			FS_OpCode.write(BinaryString.c_str(), Num);
		}

		//д��ָ��������ʼ��ַ
		Num = (unsigned int)i.Vector_Instruction.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(InstructionStartAddr));
		InstructionStartAddr += Num;
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

void BackendParse::WriteInstruction(HAZE_BINARY_OFSTREAM& B_OFS, ModuleUnit::FunctionInstruction& Instruction)
{
	/*if (!(Instruction.InsCode == InstructionOpCode::RET || Instruction.InsCode == InstructionOpCode::CALL))
	{
		return;
	}*/

	unsigned int UnsignedInt = 0;
	HAZE_BINARY_STRING BinaryString;

	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Instruction.InsCode));				//�ֽ���

	UnsignedInt = (unsigned int)Instruction.Operator.size();										
	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));						//����������

	//std::pair<InstructionDataType, std::pair<HAZE_STRING, unsigned int>>;
	for (auto& i : Instruction.Operator)
	{
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Type));					//����������

		BinaryString = WString2String(i.Name);
		UnsignedInt = (unsigned int)BinaryString.size();
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		B_OFS.write(BinaryString.data(), UnsignedInt);						//����������

		if (i.Scope == InstructionScopeType::Address)
		{
			UnsignedInt = (unsigned int)10;
			B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		}
		else
		{
			B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Scope));				//������������
		}

		//B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Extra.Index));					//����������

		//Int = (int)i.Extra.AddressOffset;
		//B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Int));					//��������ַƫ��
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << GetInstructionString(Instruction.InsCode) << " ";
	for (auto& it : Instruction.Operator)
	{
		WSS << it.Name << " " << (unsigned int)it.Type << " " << (unsigned int)it.Scope << " " << it.Extra.Index << " ";
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

unsigned int const BackendParse::GetClassSize(const HAZE_STRING& ClassName)
{
	auto Class = GetClass(ClassName);
	if (Class)
	{
		return Class->Size;

	}
	return 0;
}

unsigned int BackendParse::GetMemberOffset(const ModuleUnit::ClassTableData& Class, const HAZE_STRING& MemberName)
{
	unsigned int Offset = 0;
	for (auto& Iter : Class.Vector_Member)
	{
		if (Iter.Name != MemberName)
		{
			Offset += Iter.Type.CustomName.empty() ? GetSizeByType(Iter.Type.Type) : GetClassSize(Iter.Type.CustomName);
		}
		else
		{
			return Offset;
		}
	}

	return -1;
}
