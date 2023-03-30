#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeVM.h"

#include "HazeLog.h"

static std::pair<bool, int> ParseStringCount = { false, 0 };

BackendParse::BackendParse(HazeVM* VM) : VM(VM)
{
	CurrCode = nullptr;

	HAZE_STRING Path = std::filesystem::current_path();
	HAZE_STRING OpCodePath = Path + HAZE_TEXT("\\HazeOpCode\\");
	OpCodePath += HAZE_TEXT("Main.Hzb");

	FS_OpCode.open(OpCodePath, std::ios::out | std::ios::binary); //不用二进制的话，写入10，会当成换行特殊处理，写入两个字符 0x0d 0x0a，即回车换行符
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
	while (!isspace(*CurrCode) || (ParseStringCount.first && ParseStringCount.second > 0))
	{
		CurrLexeme += *(CurrCode++);
		ParseStringCount.second--;
	}
}

void BackendParse::Parse_I_Code()
{
	//Standard lib
	GetNextLexeme();
	CurrParseModule->IsStdLib = StringToStandardType<uint>(CurrLexeme);

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
		uint Num = StringToStandardType<uint>(CurrLexeme);

		ModuleUnit::GlobalDataTable& Table = CurrParseModule->Table_GlobalData;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextLexmeAssign_HazeString(Table.Vector_Data[i].Name);


			GetNextLexmeAssign_CustomType<uint>(Table.Vector_Data[i].Value.Type);

			GetNextLexeme();
			StringToHazeValueNumber(CurrLexeme, Table.Vector_Data[i].Value);
		}
	}
}

void BackendParse::Parse_I_Code_StringTable()
{
	if (CurrLexeme == GetStringTableHeaderString())
	{
		uint Num;
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
		uint Num;
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

					GetNextLexmeAssign_CustomType<uint>(Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType);

					if (Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType == HazeValueType::PointerClass 
						|| Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType == HazeValueType::Class)
					{
						GetNextLexmeAssign_HazeString(Table.Vector_Class[i].Vector_Member[j].Type.CustomName);
					}
					else if (Table.Vector_Class[i].Vector_Member[j].Type.PrimaryType == HazeValueType::PointerBase)
					{
						GetNextLexmeAssign_CustomType<uint>(Table.Vector_Class[i].Vector_Member[j].Type.PointerToType);
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
		uint Num;
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

				GetNextLexmeAssign_CustomType<uint>(Table.Vector_Function[i].Type);

				GetNextLexeme();
				while (CurrLexeme == GetFunctionParamHeader())
				{
					HazeDefineVariable Param;
					GetNextLexmeAssign_HazeString(Param.Name);

					GetNextLexmeAssign_CustomType<uint>(Param.Type.PrimaryType);

					if (Param.Type.PrimaryType == HazeValueType::PointerClass || Param.Type.PrimaryType == HazeValueType::Class)
					{
						GetNextLexmeAssign_HazeString(Param.Type.CustomName);

					}
					else if (Param.Type.PrimaryType == HazeValueType::PointerBase)
					{
						GetNextLexmeAssign_CustomType<uint>(Param.Type.PointerToType);
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
	GetNextLexmeAssign_CustomType<uint>(Data.Variable.Type.PrimaryType);
	
	GetNextLexmeAssign_HazeString(Data.Variable.Name);

	GetNextLexmeAssign_CustomType<uint>(Data.Scope);

	if (Data.Variable.Type.PrimaryType == HazeValueType::PointerBase)
	{
		GetNextLexmeAssign_CustomType<uint>(Data.Variable.Type.PointerToType);

		if (Data.Scope == InstructionScopeType::ConstantString)
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
		OperatorOne.Variable.Type.PrimaryType = (HazeValueType)StringToStandardType<uint>(CurrLexeme);

		if (OperatorOne.Variable.Type.PrimaryType == HazeValueType::Void)
		{
			OperatorOne.Variable.Name = HAZE_TEXT("Void");
			OperatorOne.Scope = InstructionScopeType::None;
		}
		else
		{
			GetNextLexeme();
			OperatorOne.Variable.Name = CurrLexeme;

			GetNextLexeme();
			OperatorOne.Scope = (InstructionScopeType)StringToStandardType<uint>(CurrLexeme);
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
		OperatorOne.Variable.Name = CurrLexeme;

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
		OperatorOne.Variable.Type.PrimaryType = (HazeValueType)StringToStandardType<int>(CurrLexeme);

		GetNextLexeme();
		OperatorOne.Variable.Name = CurrLexeme;

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
	
	ReplaceOffset(NewGlobalDataTable, NewStringTable, NewClassTable, NewFunctionTable);

	//将全局数据写入
	uint UnsignedInt = (uint)NewGlobalDataTable.Vector_Data.size();
	HAZE_BINARY_STRING BinaryString;
	
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
	for (auto& i : NewGlobalDataTable.Vector_Data)
	{
		BinaryString = WString2String(i.Name);
		UnsignedInt = (uint)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		
		FS_OpCode.write(BinaryString.c_str(), UnsignedInt);
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Value.Type));
		FS_OpCode.write(GetBinaryPointer(i.Value), GetSizeByHazeType(i.Value.Type));
	}

	//将字符串表写入
	UnsignedInt = (uint)NewStringTable.Vector_String.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

	for (auto& i : NewStringTable.Vector_String)
	{
		BinaryString = WString2String(i.String);
		UnsignedInt = (uint)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		FS_OpCode.write(BinaryString.data(), UnsignedInt);
	}

	//将类表写入
	UnsignedInt = (uint)NewClassTable.Vector_Class.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

	for (auto& Iter : NewClassTable.Vector_Class)
	{
		BinaryString = WString2String(Iter.Name);
		UnsignedInt = (uint)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		FS_OpCode.write(BinaryString.data(), UnsignedInt);

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Size));

		UnsignedInt = (uint)Iter.Vector_Member.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

		for (size_t i = 0; i < Iter.Vector_Member.size(); i++)
		{
			BinaryString = WString2String(Iter.Vector_Member[i].Name);
			UnsignedInt = (uint)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.data(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Vector_Member[i].Type.PrimaryType));

			BinaryString = WString2String(HAZE_STRING(Iter.Vector_Member[i].Type.CustomName));
			UnsignedInt = (uint)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.data(), UnsignedInt);
		}
	}

	//将函数表写入
	UnsignedInt = (uint)NewFunctionTable.Vector_Function.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

	uint InstructionStartAddr = 0;
	for (auto& i : NewFunctionTable.Vector_Function)
	{
		BinaryString = WString2String(i.Name);
		UnsignedInt = (uint)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		FS_OpCode.write(BinaryString.data(), UnsignedInt);

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Type));
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.DescType));

		UnsignedInt = (uint)i.Vector_Param.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		for (auto& iter : i.Vector_Param)
		{
			BinaryString = WString2String(iter.Name);
			UnsignedInt = (uint)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.c_str(), UnsignedInt);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(iter.Type.PrimaryType));
			BinaryString = WString2String(iter.Type.CustomName);
			UnsignedInt = (uint)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
			FS_OpCode.write(BinaryString.c_str(), UnsignedInt);
		}

		//写入指令数与起始地址
		UnsignedInt = (uint)i.Vector_Instruction.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(InstructionStartAddr));
		InstructionStartAddr += UnsignedInt;
	}

	//写入指令
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(InstructionStartAddr));  //指令总个数
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

	//替换变量为索引或相对函数起始偏移
	for (size_t k = 0; k < NewFunctionTable.Vector_Function.size(); ++k)
	{
		auto& CurrFunction = NewFunctionTable.Vector_Function[k];
		for (size_t i = 0; i < CurrFunction.Vector_Instruction.size(); ++i)
		{
			for (auto& it : CurrFunction.Vector_Instruction[i].Operator)
			{
				auto& VariableName = it.Variable.Name;
				if (it.Scope == InstructionScopeType::Global)
				{
					it.Extra.Index = NewGlobalDataTable.GetIndex(VariableName);
				}
				else if (it.Scope == InstructionScopeType::ClassMember_Local)
				{
					int AddressOffset = 0 - HAZE_ADDRESS_SIZE;	 	//入栈的返回地址数据占用空间为4个字节

					HAZE_STRING ObjName;
					HAZE_STRING MemberName;
					size_t Pos = VariableName.find(HAZE_CLASS_POINTER_ATTR);
					bool IsPointer = true;
					if (Pos != HAZE_STRING::npos)
					{
						ObjName = VariableName.substr(0, Pos);
						MemberName = VariableName.substr(Pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());
					}
					else
					{
						Pos = VariableName.find(HAZE_CLASS_ATTR);
						if (Pos != HAZE_STRING::npos)
						{
							ObjName = VariableName.substr(0, Pos);
							MemberName = VariableName.substr(Pos + HAZE_STRING(HAZE_CLASS_ATTR).size());
							IsPointer = false;
						}
					}

					bool Find = false;
					for (int j = (int)CurrFunction.Vector_Param.size() - 1; j >= 0; --j)
					{
						AddressOffset -= GetSizeByType(CurrFunction.Vector_Param[j].Type, this);
						if (CurrFunction.Vector_Param[j].Name == ObjName)
						{
							auto Class = GetClass(CurrFunction.Vector_Param[j].Type.CustomName);
							if (IsPointer)
							{
								//需要存储一个指针指向地址的相对偏移
								AddressOffset -= GetSizeByHazeType(CurrFunction.Vector_Param[j].Type.PrimaryType);
								it.Extra.Address = AddressOffset + GetMemberOffset(*Class, MemberName); //错误，应该是加上偏移位置，需要存储两个值
								Find = true;
								break;
							}
							else
							{
								AddressOffset -= GetMemberOffset(*Class, MemberName);
								it.Extra.Address = AddressOffset - GetMemberOffset(*Class, MemberName);
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
					for (uint j = 0; j < i; j++)
					{
						if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::PUSH)
						{
							if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name == ObjName)
							{
								auto Class = GetClass(NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Type.CustomName);
								it.Extra.Address = Offset + GetMemberOffset(*Class, MemberName);
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
							for (auto& Param : NewFunctionTable.Vector_Function[HashMap_FunctionIndexAndAddress[NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name]].Vector_Param)
							{
								Offset -= GetSizeByType(Param.Type, this);
							}
						}
					}
				}
				else if (it.Scope == InstructionScopeType::Local || it.Scope == InstructionScopeType::Temp)
				{
					//需要先判断是不是函数参数，设置为负的索引
					int AddressOffset = 0 - HAZE_ADDRESS_SIZE;	 	//入栈的返回地址数据占用空间为4个字节
					bool Find = false;
					for (int j = (int)NewFunctionTable.Vector_Function[k].Vector_Param.size() - 1; j >= 0; --j)
					{
						AddressOffset -= GetSizeByHazeType(NewFunctionTable.Vector_Function[k].Vector_Param[j].Type.PrimaryType);
						if (NewFunctionTable.Vector_Function[k].Vector_Param[j].Name == VariableName)
						{
							it.Extra.Address = AddressOffset;
							Find = true;
							break;
						}
					}

					if (Find)
					{
						continue;
					}


					int Offset = 0;
					for (uint j = 0; j < i; j++)
					{
						if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].InsCode == InstructionOpCode::PUSH)
						{
							if (NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name == VariableName)
							{
								it.Extra.Address = Offset;
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
							for (auto& Param : NewFunctionTable.Vector_Function[HashMap_FunctionIndexAndAddress[NewFunctionTable.Vector_Function[k].Vector_Instruction[j].Operator[0].Variable.Name]].Vector_Param)
							{
								Offset -= GetSizeByType(Param.Type, this);
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
				WSS << it.Variable.Name << " " << it.Extra.Address << " ";
			}
			WSS << std::endl;

			std::wcout << WSS.str();
#endif // ENABLE_BACKEND_INSTRUCTION_LOG
		}
	}
}

void BackendParse::WriteInstruction(HAZE_BINARY_OFSTREAM& B_OFS, ModuleUnit::FunctionInstruction& Instruction)
{
	uint UnsignedInt = 0;
	HAZE_BINARY_STRING BinaryString;

	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Instruction.InsCode));				//字节码

	UnsignedInt = (uint)Instruction.Operator.size();										
	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));						//操作数个数

	//std::pair<InstructionDataType, std::pair<HAZE_STRING, uint>>;
	for (auto& Iter : Instruction.Operator)
	{
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Variable.Type.PrimaryType));		//操作数类型

		BinaryString = WString2String(Iter.Variable.Name);
		UnsignedInt = (uint)BinaryString.size();
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		B_OFS.write(BinaryString.data(), UnsignedInt);								//操作数名字

		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Scope));						//操作数作用域

		BinaryString = WString2String(Iter.Variable.Type.CustomName);
		UnsignedInt = (uint)BinaryString.size();
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(UnsignedInt));
		B_OFS.write(BinaryString.data(), UnsignedInt);								//操作数类型2

		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Extra.Index));								//操作数索引
		//B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Iter.Extra.AddressOffset));						//操作数地址偏移
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << GetInstructionString(Instruction.InsCode) << " ";
	for (auto& it : Instruction.Operator)
	{
		WSS << it.Variable.Name << " " << (uint)it.Variable.Type.PrimaryType << " " << (uint)it.Scope << " " << it.Extra.Index << " ";
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

uint const BackendParse::GetClassSize(const HAZE_STRING& ClassName)
{
	auto Class = GetClass(ClassName);
	if (Class)
	{
		return Class->Size;

	}
	return 0;
}

uint BackendParse::GetMemberOffset(const ModuleUnit::ClassTableData& Class, const HAZE_STRING& MemberName)
{
	uint Offset = 0;
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

	return (uint)-1;
}
