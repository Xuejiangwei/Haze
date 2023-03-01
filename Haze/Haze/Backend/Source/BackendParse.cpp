#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeVM.h"

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
	//Global data
	GetNextLexeme();
	if (CurrLexeme == GetGlobalDataHeaderString())
	{
		GetNextLexeme();
		unsigned int Num = StringToInt<unsigned int>(CurrLexeme);

		ModuleUnit::GlobalDataTable& Table = CurrParseModule->Table_GlobalData;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextLexeme();

			Table.Vector_Data[i].Name = CurrLexeme;

			GetNextLexeme();

			unsigned int Type = StringToInt<unsigned int>(CurrLexeme);

			Table.Vector_Data[i].Value.Type = (HazeValueType)Type;

			GetNextLexeme();

			StringToHazeValueNumber(CurrLexeme, Table.Vector_Data[i].Value);
		}
	}

	//String table
	GetNextLexeme();
	if (CurrLexeme == GetStringTableHeaderString())
	{
		GetNextLexeme();
		unsigned int Num = StringToInt<unsigned int>(CurrLexeme);

		ModuleUnit::StringTable& Table = CurrParseModule->Table_String;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextLexeme();

			Table.Vector_Data[i].Name = CurrLexeme;

			GetNextLexeme();

			Table.Vector_Data[i].String = CurrLexeme;
		}
	}

	//Function table
	GetNextLexeme();
	if (CurrLexeme == GetFucntionTableHeaderString())
	{
		GetNextLexeme();
		unsigned int Num = StringToInt<unsigned int>(CurrLexeme);

		ModuleUnit::FunctionTable& Table = CurrParseModule->Table_Function;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextLexeme();

			if (CurrLexeme == GetFunctionLabelHeader())
			{
				GetNextLexeme();
				Table.Vector_Data[i].Name = CurrLexeme;

				GetNextLexeme();
				Table.Vector_Data[i].Type = (HazeValueType)StringToInt<unsigned int>(CurrLexeme);

				GetNextLexeme();
				while (CurrLexeme == GetFunctionParamHeader())
				{
					GetNextLexeme();

					std::pair<HAZE_STRING, HazeValue> Param;
					Param.first = CurrLexeme;

					GetNextLexeme();
					Param.second.Type = (HazeValueType)StringToInt<unsigned int>(CurrLexeme);

					Table.Vector_Data[i].Vector_Param.push_back(std::move(Param));

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

						Table.Vector_Data[i].Vector_Instruction.push_back(std::move(Instruction));

						GetNextLexeme();
					}
				}
			}
		}
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
	case InstructionOpCode::EXP:
	{
		InstructionData OperatorOne;
		InstructionData OperatorTwo;

		GetNextLexeme();
		OperatorOne.Scope = (InstructionScopeType)StringToInt<unsigned int>(CurrLexeme);

		GetNextLexeme();
		OperatorOne.Name = CurrLexeme;

		GetNextLexeme();
		OperatorTwo.Scope = (InstructionScopeType)StringToInt<unsigned int>(CurrLexeme);

		GetNextLexeme();
		OperatorTwo.Name = CurrLexeme;

		Instruction.Operator = { OperatorOne, OperatorTwo };
	}
	break;
	case InstructionOpCode::NEG:
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

		GetNextLexeme();
		OperatorOne.Type = (HazeValueType)StringToInt<unsigned int>(CurrLexeme);

		GetNextLexeme();
		OperatorOne.Name = CurrLexeme;

		GetNextLexeme();
		OperatorOne.Scope = (InstructionScopeType)StringToInt<unsigned int>(CurrLexeme);

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::RET:
	{
		InstructionData OperatorOne;

		GetNextLexeme();
		OperatorOne.Scope = (InstructionScopeType)StringToInt<unsigned int>(CurrLexeme);

		GetNextLexeme();
		OperatorOne.Name = CurrLexeme;

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::POP:
		break;
	case InstructionOpCode::CALL:
	{
		InstructionData OperatorOne;

		GetNextLexeme();
		OperatorOne.Scope = InstructionScopeType::None;
		OperatorOne.Name = CurrLexeme;

		Instruction.Operator = { OperatorOne };
	}
	break;
	case InstructionOpCode::Concat:
		break;
	case InstructionOpCode::GetChar:
		break;
	case InstructionOpCode::SetChar:
		break;
	default:
		break;
	}
}

void BackendParse::GenOpCodeFile()
{
	ModuleUnit::GlobalDataTable NewGlobalDataTable;
	ModuleUnit::StringTable NewStringTable;
	ModuleUnit::FunctionTable NewFunctionTable;

	
	for (auto& iter : HashMap_Modules)
	{
		NewGlobalDataTable.Vector_Data.insert(NewGlobalDataTable.Vector_Data.end(), iter.second->Table_GlobalData.Vector_Data.begin(), iter.second->Table_GlobalData.Vector_Data.end());
		NewStringTable.Vector_Data.insert(NewStringTable.Vector_Data.end(), iter.second->Table_String.Vector_Data.begin(), iter.second->Table_String.Vector_Data.end());
		NewFunctionTable.Vector_Data.insert(NewFunctionTable.Vector_Data.end(), iter.second->Table_Function.Vector_Data.begin(), iter.second->Table_Function.Vector_Data.end());
	}
	
	//替换变量为索引
	for (auto& iter : NewFunctionTable.Vector_Data)
	{
		for (size_t i = 0; i < iter.Vector_Instruction.size(); ++i)
		{
			for (auto& it : iter.Vector_Instruction[i].Operator)
			{
				if (it.Scope == InstructionScopeType::Global)
				{
					it.Index = NewGlobalDataTable.GetIndex(it.Name);
				}
				else if (it.Scope == InstructionScopeType::Local)
				{
					for (unsigned int j = 0; j < i; j++)
					{
						if (iter.Vector_Instruction[j].InsCode == InstructionOpCode::PUSH && iter.Vector_Instruction[j].Operator[0].Name == it.Name)
						{
							it.Index = j;
							break;
						}
					}
				}
			}
		}
	}

	//将全局数据写入
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
		FS_OpCode.write(GetBinaryPointer(i.Value), GetSize(i.Value.Type));
	}

	//将字符串表写入
	Num = (unsigned int)NewStringTable.Vector_Data.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));

	for (auto& i : NewStringTable.Vector_Data)
	{	
		BinaryString = WString2String(i.Name);
		Num = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		FS_OpCode.write(BinaryString.data(), Num);

		BinaryString = WString2String(i.String);
		Num = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		FS_OpCode.write(BinaryString.data(), Num);
	}

	//将函数表写入
	Num = (unsigned int)NewFunctionTable.Vector_Data.size();
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));

	unsigned int InstructionStartAddr = 0;
	for (auto& i : NewFunctionTable.Vector_Data)
	{
		BinaryString = WString2String(i.Name);
		Num = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		FS_OpCode.write(BinaryString.data(), Num);

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(i.Type));

		Num = (unsigned int)i.Vector_Param.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
		for (auto& iter : i.Vector_Param)
		{
			BinaryString = WString2String(iter.first);
			Num = (unsigned int)BinaryString.size();
			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));
			FS_OpCode.write(BinaryString.c_str(), Num);

			FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(iter.second.Type));
			FS_OpCode.write(GetBinaryPointer(iter.second), GetSize(iter.second.Type));
		}

		//写入指令数与起始地址
		Num = (unsigned int)i.Vector_Instruction.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Num));

		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(InstructionStartAddr));
		InstructionStartAddr += Num;
	}

	//写入指令
	FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(InstructionStartAddr));  //指令总个数
	for (auto& i : NewFunctionTable.Vector_Data)
	{
		for (auto& iter : i.Vector_Instruction)
		{
			WriteInstruction(FS_OpCode, iter);
		}
	}
}

void BackendParse::WriteInstruction(HAZE_BINARY_OFSTREAM& B_OFS, ModuleUnit::FunctionInstruction& Instruction)
{
	unsigned int Int = 0;
	HAZE_BINARY_STRING BinaryString;

	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Instruction.InsCode));		//字节码
	Int = (unsigned int)Instruction.Operator.size();										
	B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Int));						//操作数个数

	//std::pair<InstructionDataType, std::pair<HAZE_STRING, unsigned int>>;
	for (auto& i : Instruction.Operator)
	{
		Int = (unsigned int)i.Scope;
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Int));					//操作数类型

		BinaryString = WString2String(i.Name);
		Int = (unsigned int)BinaryString.size();
		FS_OpCode.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Int));
		FS_OpCode.write(BinaryString.data(), Int);							//操作数名字

		Int = (unsigned int)i.Index;
		B_OFS.write(HAZE_BINARY_OP_WRITE_CODE_SIZE(Int));					//操作数索引
	}
	/*switch (Instruction.InsCode)
	{
	case InstructionOpCode::NONE:
		break;
	case InstructionOpCode::MOV:
	case InstructionOpCode::ADD:
	case InstructionOpCode::SUB:
	case InstructionOpCode::MUL:
	case InstructionOpCode::DIV:
	case InstructionOpCode::MOD:
	case InstructionOpCode::EXP:
	{
		
	}
		break;
	case InstructionOpCode::NEG:
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
	case InstructionOpCode::RET:
	{
	}
		break;
	case InstructionOpCode::POP:
		break;
	case InstructionOpCode::CALL:
	{

	}
		break;
	case InstructionOpCode::Concat:
		break;
	case InstructionOpCode::GetChar:
		break;
	case InstructionOpCode::SetChar:
		break;
	default:
		break;
	}*/
}
