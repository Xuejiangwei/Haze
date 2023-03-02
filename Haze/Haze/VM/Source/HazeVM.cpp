#include <iostream>
#include <locale>

#include <filesystem>

#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeCompiler.h"
#include "Parse.h"
#include "HazeLog.h"

#include "BackendParse.h"

HazeVM::HazeVM()
{
	std::wcout.imbue(std::locale("chs"));

	FunctionReturn.Type = HazeValueType::Null;
	Compiler = std::make_unique<HazeCompiler>();
	VMStack = std::make_unique<HazeStack>();
}

HazeVM::~HazeVM()
{
}

void HazeVM::InitVM(std::vector<ModulePair> Vector_ModulePath)
{
	for (auto& iter : Vector_ModulePath)
	{
		ParseFile(iter.first, iter.second);
	}

	{
		BackendParse BP(this);
		BP.Parse();
	}

	//LoadOpCodeFile();
}

void HazeVM::StartMainFunction()
{
	int StartAddress = -1;
	for (auto& i : Vector_FunctionTable)
	{
		if (i.first == HAZE_MAIN_FUNCTION_TEXT)
		{
			StartAddress = i.second.InstructionStartAddress;
			break;
		}
	}

	if (StartAddress >= 0)
	{
		RunInstruction(Vector_Instruction[StartAddress]);
	}
}

void HazeVM::ParseString(const HAZE_STRING& String)
{
	Parse P(this);
	P.InitializeString(String);
	P.ParseContent();
}

void HazeVM::ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName)
{
	if (Compiler->InitializeCompiler(ModuleName))
	{
		Parse P(this);
		P.InitializeFile(FilePath);
		P.ParseContent();
	}
	Compiler->FinishModule();

	auto Module = UnorderedMap_Module.find(ModuleName);
	if (Module == UnorderedMap_Module.end())
	{
		UnorderedMap_Module[ModuleName] = std::make_unique<HazeModule>(Compiler->GetCurrModuleOpFile());
	}
}

HazeValue* HazeVM::GetVirtualRegister(const HAZE_CHAR* Name)
{
	return VMStack->GetVirtualRegister(Name);
}

void HazeVM::LoadOpCodeFile()
{
	HAZE_STRING Path = std::filesystem::current_path();
	HAZE_STRING OpCodePath = Path + HAZE_TEXT("\\HazeOpCode\\");
	OpCodePath += HAZE_TEXT("Main.Hzb");

	HAZE_BINARY_IFSTREAM FS(OpCodePath, std::ios::binary);
	FS.imbue(std::locale("chs"));

	//HAZE_BINARY_STRING Content(std::istreambuf_iterator<HAZE_BINARY_CHAR>(FS), {});

	//const HAZE_BINARY_CHAR* T = Content.c_str();
	//CodeText = std::move(Content);

	unsigned int Num = 0;
	HAZE_BINARY_STRING B_String;

	FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
	Vector_GlobalData.resize(Num);

	for (size_t i = 0; i < Vector_GlobalData.size(); i++)
	{
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));

		B_String.resize(Num);
		FS.read(B_String.data(), Num);
		Vector_GlobalData[i].first = String2WString(B_String);

		FS.read((char*)&Vector_GlobalData[i].second.Type, sizeof(HazeValue::Type));
		FS.read(GetBinaryPointer(Vector_GlobalData[i].second), GetSize(Vector_GlobalData[i].second.Type));
	}

	FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
	Vector_StringTable.resize(Num);

	for (size_t i = 0; i < Vector_StringTable.size(); i++)
	{
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		B_String.resize(Num);
		FS.read(B_String.data(), Num);
		Vector_StringTable[i].first = String2WString(B_String);

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		B_String.resize(Num);
		FS.read(B_String.data(), Num);
		Vector_StringTable[i].second = String2WString(B_String);
	}

	FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
	Vector_FunctionTable.resize(Num);

	for (size_t i = 0; i < Vector_FunctionTable.size(); i++)
	{
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		B_String.resize(Num);
		FS.read(B_String.data(), Num);
		Vector_FunctionTable[i].first = String2WString(B_String);

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].second.Type));

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		Vector_FunctionTable[i].second.Vector_Param.resize(Num);

		for (size_t j = 0; j < Vector_FunctionTable[i].second.Vector_Param.size(); j++)
		{
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
			B_String.resize(Num);
			FS.read(B_String.data(), Num);
			Vector_FunctionTable[i].second.Vector_Param[j].first = String2WString(B_String);

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].second.Vector_Param[j].second.Type));
			FS.read(GetBinaryPointer(Vector_FunctionTable[i].second.Vector_Param[j].second), GetSize(Vector_FunctionTable[i].second.Vector_Param[j].second.Type));
		}

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].second.InstructionNum));
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].second.InstructionStartAddress));
	}

	//¶ÁÈ¡Ö¸Áî
	FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
	Vector_Instruction.resize(Num);
	for (size_t i = 0; i < Vector_Instruction.size(); i++)
	{
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_Instruction[i].InsCode));
		ReadInstruction(FS, Vector_Instruction[i]);
	}
}

void HazeVM::ReadInstruction(HAZE_BINARY_IFSTREAM& B_IFS, Instruction& Instruction)
{
	unsigned int Int = 0;
	HAZE_BINARY_STRING BinaryString;
	B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Int));
	Instruction.Operator.resize(Int);

	for (auto& i : Instruction.Operator)
	{
		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(i.Type));

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Int));
		BinaryString.resize(Int);
		B_IFS.read(BinaryString.data(), Int);
		i.Name = String2WString(BinaryString);

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(i.Index));
	}
}

void HazeVM::RunInstruction(const Instruction& Ins)
{
	switch (Ins.InsCode)
	{
	case InstructionOpCode::NONE:
		break;
	case InstructionOpCode::MOV: 
	{

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
	{
		VMStack->Push(Ins.Operator);
	}
		break;
	case InstructionOpCode::POP:
		break;
	case InstructionOpCode::CALL:
		break;
	case InstructionOpCode::RET:
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
