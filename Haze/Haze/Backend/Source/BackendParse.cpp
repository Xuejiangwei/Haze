#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeVM.h"
#include "ModuleUnit.h"

BackendParse::BackendParse(HazeVM* VM) : VM(VM)
{
	CurrCode = nullptr;
	CurrInstruction = InstructionOpCode::NONE;
}

BackendParse::~BackendParse()
{
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

		std::wifstream FS(I_CodePath);
		FS.imbue(std::locale("chs"));

		HAZE_STRING Content(std::istreambuf_iterator<HAZE_CHAR>(FS), {});

		CodeText = std::move(Content);
		CurrCode = CodeText.c_str();
		
		ToParse();
		
		FS.close();
	}
}

void BackendParse::ToParse()
{
	ParseHeader();

	GetNextInstruction();

	ParseInstruction();
}

void BackendParse::ParseHeader()
{
	auto GetNextHeader = [](const HAZE_CHAR*& CurrCode, HAZE_STRING& CurrLexeme)
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
	};

	auto ParseInstruction = [&GetNextHeader](ModuleUnit::FunctionInstruction& Instruction, const HAZE_CHAR*& CurrCode, HAZE_STRING& CurrLexeme)
	{
		switch (Instruction.InsCode)
		{
		case InstructionOpCode::NONE:
			break;
		case InstructionOpCode::MOV:
		{
			std::pair<InstructionDataType, HAZE_STRING> OperatorOne;
			std::pair<InstructionDataType, HAZE_STRING> OperatorTwo;

			GetNextHeader(CurrCode, CurrLexeme);
			OperatorOne.first = (InstructionDataType)StringToInt<unsigned int>(CurrLexeme);

			GetNextHeader(CurrCode, CurrLexeme);
			OperatorOne.second = CurrLexeme;

			GetNextHeader(CurrCode, CurrLexeme);
			OperatorTwo.first = (InstructionDataType)StringToInt<unsigned int>(CurrLexeme);

			GetNextHeader(CurrCode, CurrLexeme);
			OperatorTwo.second = CurrLexeme;

			Instruction.Operator = { OperatorOne, OperatorTwo };
		}
			break;
		case InstructionOpCode::ADD:
			break;
		case InstructionOpCode::SUB:
			break;
		case InstructionOpCode::MUL:
			break;
		case InstructionOpCode::DIV:
			break;
		case InstructionOpCode::MOD:
			break;
		case InstructionOpCode::EXP:
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
			break;
		case InstructionOpCode::POP:
			break;
		case InstructionOpCode::CALL:
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

		
	};
	
	//Global data
	GetNextHeader(CurrCode, CurrLexeme);
	if (CurrLexeme == GetGlobalDataHeaderString())
	{
		GetNextHeader(CurrCode, CurrLexeme);
		unsigned int Num = StringToInt<unsigned int>(CurrLexeme);

		ModuleUnit::GlobalDataTable& Table = CurrParseModule->Table_GlobalData;
		Table.Num = Num;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextHeader(CurrCode, CurrLexeme);

			Table.Vector_Data[i].Name = CurrLexeme;

			GetNextHeader(CurrCode, CurrLexeme);

			unsigned int Type = StringToInt<unsigned int>(CurrLexeme);

			Table.Vector_Data[i].Value.Type = (HazeValueType)Type;

			GetNextHeader(CurrCode, CurrLexeme);

			StringToHazeValueNumber(CurrLexeme, Table.Vector_Data[i].Value);
		}
	}

	//String table
	GetNextHeader(CurrCode, CurrLexeme);
	if (CurrLexeme == GetStringTableHeaderString())
	{
		GetNextHeader(CurrCode, CurrLexeme);
		unsigned int Num = StringToInt<unsigned int>(CurrLexeme);

		ModuleUnit::StringTable& Table = CurrParseModule->Table_String;
		Table.Num = Num;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{
			GetNextHeader(CurrCode, CurrLexeme);

			Table.Vector_Data[i].Name = CurrLexeme;

			GetNextHeader(CurrCode, CurrLexeme);

			Table.Vector_Data[i].String = CurrLexeme;
		}
	}

	//Function table
	GetNextHeader(CurrCode, CurrLexeme);
	if (CurrLexeme == GetFucntionTableHeaderString())
	{
		GetNextHeader(CurrCode, CurrLexeme);
		unsigned int Num = StringToInt<unsigned int>(CurrLexeme);

		ModuleUnit::FunctionTable& Table = CurrParseModule->Table_Function;
		Table.Num = Num;
		Table.Vector_Data.resize(Num);

		for (size_t i = 0; i < Table.Vector_Data.size(); i++)
		{

			GetNextHeader(CurrCode, CurrLexeme);

			Table.Vector_Data[i].Name = CurrLexeme;

			GetNextHeader(CurrCode, CurrLexeme);
			while (CurrLexeme == GetFunctionParamHeader())
			{
				GetNextHeader(CurrCode, CurrLexeme);

				std::pair<HAZE_STRING, HazeValue> Param;
				Param.first = CurrLexeme;

				GetNextHeader(CurrCode, CurrLexeme);
				Param.second.Type = (HazeValueType)StringToInt<unsigned int>(CurrLexeme);

				Table.Vector_Data[i].Vector_Param.push_back(std::move(Param));

				GetNextHeader(CurrCode, CurrLexeme);
			}

			if (CurrLexeme == GetFunctionStartHeader())
			{
				GetNextHeader(CurrCode, CurrLexeme);
				while (CurrLexeme != GetFunctionEndHeader())
				{
					ModuleUnit::FunctionInstruction Instruction;
					Instruction.InsCode = GetInstructionByString(CurrLexeme);

					ParseInstruction(Instruction, CurrCode, CurrLexeme);

					Table.Vector_Data[i].Vector_Instruction.push_back(std::move(Instruction));
				}
			}
		}
	}
}

void BackendParse::ParseInstruction()
{
	while (CurrInstruction != InstructionOpCode::NONE)
	{
		switch (CurrInstruction)
		{
		case InstructionOpCode::NONE:
			break;
		case InstructionOpCode::MOV:
			break;
		case InstructionOpCode::ADD:
			break;
		case InstructionOpCode::SUB:
			break;
		case InstructionOpCode::MUL:
			break;
		case InstructionOpCode::DIV:
			break;
		case InstructionOpCode::MOD:
			break;
		case InstructionOpCode::EXP:
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
			break;
		case InstructionOpCode::POP:
			break;
		case InstructionOpCode::CALL:
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
}

InstructionOpCode BackendParse::GetNextInstruction()
{
	if (!*CurrCode)
	{
		CurrInstruction = InstructionOpCode::NONE;
		CurrLexeme = HAZE_TEXT("");
		return InstructionOpCode::NONE;
	}

	while (isspace(*CurrCode))
	{
		CurrCode++;
	}

	if (HAZE_STRING(CurrCode) == HAZE_TEXT(""))
	{
		return InstructionOpCode::NONE;
	}

	//Match Token
	CurrLexeme.clear();
	while (!isspace(*CurrCode))
	{
		/*if (IsHazeSignalToken(*CurrCode))
		{
			if (CurrLexeme.length() == 0)
			{
				CurrLexeme += *(CurrCode++);
			}
			break;
		}*/

		CurrLexeme += *(CurrCode++);
	}

	/*if (CurrLexeme == HAZE_STRING(HAZE_SINGLE_COMMENT))
	{
		HAZE_STRING WS;
		WS = *CurrCode;
		while (WS != HAZE_TEXT("\n"))
		{
			CurrCode++;
			WS = *CurrCode;
		}
		return GetNextToken();
	}*/


	InstructionOpCode Ins = GetInstructionByString(CurrLexeme);
	if (Ins != InstructionOpCode::NONE)
	{
		CurrInstruction = Ins;
	}
	/*else if (IsNumber(CurrLexeme))
	{
		CurrInstruction = HazeToken::Number;
	}
	else
	{
		CurrToken = HazeToken::Identifier;
	}*/

	return CurrInstruction;
}
