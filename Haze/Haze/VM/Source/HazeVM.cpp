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
	VMStack = std::make_unique<HazeStack>(this);
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

	LoadOpCodeFile();
}

void HazeVM::LoadStandardLibrary(std::vector<ModulePair> Vector_ModulePath)
{
}

void HazeVM::StartMainFunction()
{
	auto Iter = HashMap_FunctionTable.find(HAZE_MAIN_FUNCTION_TEXT);
	if (Iter != HashMap_FunctionTable.end())
	{
		VMStack->Start(Vector_FunctionTable[Iter->second].InstructionStartAddress);
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
		Compiler->FinishModule();
	}

	if (!Compiler->CurrModuleIsStdLib())
	{
		auto Module = UnorderedMap_Module.find(ModuleName);
		if (Module == UnorderedMap_Module.end())
		{
			UnorderedMap_Module[ModuleName] = std::make_unique<HazeModule>(Compiler->GetCurrModuleOpFile());
		}
	}
	
	Compiler->PopCurrModule();
}

void HazeVM::ParseModule(const HAZE_STRING& ModuleName)
{
	HAZE_STRING FilePath = std::filesystem::current_path();
	FilePath += (HAZE_TEXT("\\Other\\") + ModuleName + HAZE_TEXT(".hz"));

	ParseFile(FilePath, ModuleName);
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
		HashMap_FunctionTable[String2WString(B_String)] = (unsigned int)i;

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Type));

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		Vector_FunctionTable[i].Vector_Param.resize(Num);

		for (size_t j = 0; j < Vector_FunctionTable[i].Vector_Param.size(); j++)
		{
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
			B_String.resize(Num);
			FS.read(B_String.data(), Num);
			Vector_FunctionTable[i].Vector_Param[j].first = String2WString(B_String);

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Vector_Param[j].second.Type));
			FS.read(GetBinaryPointer(Vector_FunctionTable[i].Vector_Param[j].second), GetSize(Vector_FunctionTable[i].Vector_Param[j].second.Type));
		}

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].InstructionNum));
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].InstructionStartAddress));
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

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(i.Scope));

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(i.Extra.Index));
	}
}

int HazeVM::GetFucntionIndexByName(const HAZE_STRING& Name)
{
	auto Iter = HashMap_FunctionTable.find(Name);
	if (Iter == HashMap_FunctionTable.end())
	{
		return -1;
	}
	return Iter->second;
}

HazeValue* HazeVM::GetGlobalValue(const HAZE_STRING& Name)
{
	for (auto& Iter : Vector_GlobalData)
	{
		if (Iter.first == Name)
		{
			return &Iter.second;
		}
	}

	return nullptr;
}
