#include <iostream>
#include <locale>

#include <filesystem>

#include "HazeVM.h"
#include "HazeStack.h"
#include "GarbageCollection.h"
#include "HazeCompiler.h"
#include "Parse.h"
#include "HazeFilePathHelper.h"

#include "HazeLog.h"

#include "BackendParse.h"

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> Hash_MapStdLib;

HazeVM::HazeVM()
{
	FunctionReturn.first.PrimaryType = HazeValueType::Void;
	Compiler = std::make_unique<HazeCompiler>();
	VMStack = std::make_unique<HazeStack>(this);
	GC = std::make_unique<GarbageCollection>(this);
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
#define HAZE_BACKEND_PARSE_ENABLE	1

#if HAZE_BACKEND_PARSE_ENABLE
		BackendParse BP(this);
		BP.Parse();
#endif // HAZE_BACKEND_PARSE_ENABLE

	}

#define HAZE_LOAD_OP_CODE_ENABLE	1

#if HAZE_LOAD_OP_CODE_ENABLE
	LoadOpCodeFile();
#endif // ENABLE_LOAD_OP_CODE

}

void HazeVM::LoadStandardLibrary(std::vector<ModulePair> Vector_ModulePath)
{
}

void HazeVM::StartMainFunction()
{
	auto Iter = HashMap_FunctionTable.find(HAZE_MAIN_FUNCTION_TEXT);
	if (Iter != HashMap_FunctionTable.end())
	{
		VMStack->Start(Vector_FunctionTable[Iter->second].Extra.FunctionDescData.InstructionStartAddress);
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
	bool PopCurrModule = false;
	if (Compiler->InitializeCompiler(ModuleName))
	{
		PopCurrModule = true;
		Parse P(this);
		P.InitializeFile(FilePath);
		P.ParseContent();
		Compiler->FinishModule();
	}

	HashSet_RefModule.insert(ModuleName);

	if (PopCurrModule)
	{
		Compiler->PopCurrModule();
	}
}

HazeCompilerModule* HazeVM::ParseModule(const HAZE_STRING& ModuleName)
{
	ParseFile(GetModuleFilePath(ModuleName), ModuleName);
	return Compiler->GetModule(ModuleName);
}

void HazeVM::LoadOpCodeFile()
{
	HAZE_BINARY_IFSTREAM FS(GetMainBinaryFilePath(), std::ios::in | std::ios::binary);
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
		Vector_GlobalData[i].Name = String2WString(B_String);

		FS.read((char*)&Vector_GlobalData[i].Type.PrimaryType, sizeof(HazeValueType));
		FS.read(GetBinaryPointer(Vector_GlobalData[i].Type.PrimaryType, Vector_GlobalData[i].Value), GetSizeByHazeType(Vector_GlobalData[i].Type.PrimaryType));
	}

	FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
	Vector_StringTable.resize(Num);

	for (size_t i = 0; i < Vector_StringTable.size(); i++)
	{
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		B_String.resize(Num);
		FS.read(B_String.data(), Num);
		Vector_StringTable[i].second = String2WString(B_String);
	}

	FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
	Vector_ClassTable.resize(Num);

	for (size_t i = 0; i < Vector_ClassTable.size(); i++)
	{
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		B_String.resize(Num);
		FS.read(B_String.data(), Num);
		Vector_ClassTable[i].Name = String2WString(B_String);

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_ClassTable[i].Size));

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		Vector_ClassTable[i].Vector_Member.resize(Num);

		for (size_t j = 0; j < Vector_ClassTable[i].Vector_Member.size(); j++)
		{
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
			B_String.resize(Num);
			FS.read(B_String.data(), Num);
			Vector_ClassTable[i].Vector_Member[j].MemberData.Name = String2WString(B_String);

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_ClassTable[i].Vector_Member[j].MemberData.Type.PrimaryType));
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));

			FS.read(B_String.data(), Num);
			if (Num > 0)
			{
				Vector_ClassTable[i].Vector_Member[j].MemberData.Type.CustomName = String2WString(B_String);
			}

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_ClassTable[i].Vector_Member[j].Offset));
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_ClassTable[i].Vector_Member[j].Size));
		}
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
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Extra.FunctionDescData.Type));

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		Vector_FunctionTable[i].Vector_Param.resize(Num);

		for (size_t j = 0; j < Vector_FunctionTable[i].Vector_Param.size(); j++)
		{
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
			B_String.resize(Num);
			FS.read(B_String.data(), Num);
			Vector_FunctionTable[i].Vector_Param[j].Name = String2WString(B_String);

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Vector_Param[j].Type.PrimaryType));

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
			B_String.resize(Num);
			FS.read(B_String.data(), Num);
			Vector_FunctionTable[i].Vector_Param[j].Type.CustomName = String2WString(B_String);
		}

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
		Vector_FunctionTable[i].Vector_Variable.resize(Num);

		for (size_t j = 0; j < Vector_FunctionTable[i].Vector_Variable.size(); j++)
		{
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
			B_String.resize(Num);
			FS.read(B_String.data(), Num);
			Vector_FunctionTable[i].Vector_Variable[j].Variable.Name = String2WString(B_String);

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Vector_Variable[j].Variable.Type.PrimaryType));

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
			B_String.resize(Num);
			FS.read(B_String.data(), Num);
			Vector_FunctionTable[i].Vector_Variable[j].Variable.Type.CustomName = String2WString(B_String);

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Vector_Variable[j].Offset));
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Vector_Variable[j].Size));
		}

		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].InstructionNum));
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_FunctionTable[i].Extra.FunctionDescData.InstructionStartAddress));
	}

	//读取指令
	FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));
	Vector_Instruction.resize(Num);
	for (size_t i = 0; i < Vector_Instruction.size(); i++)
	{
		FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_Instruction[i].InsCode));
		ReadInstruction(FS, Vector_Instruction[i]);
	}

	//重新指认std lib 函数指针
	for (auto& Iter : HashMap_FunctionTable)
	{
		auto& Function = Vector_FunctionTable[Iter.second];
		if (Function.Extra.FunctionDescData.Type == InstructionFunctionType::StdLibFunction)
		{
			for (auto& I : Hash_MapStdLib)
			{
				auto P = I.second->find(Iter.first);
				if (P != I.second->end())
				{
					Function.Extra.StdLibFunction = P->second;
				}
			}
		}
	}
}

void HazeVM::ReadInstruction(HAZE_BINARY_IFSTREAM& B_IFS, Instruction& Instruction)
{
	unsigned int UnsignedInt = 0;
	HAZE_BINARY_STRING BinaryString;
	B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(UnsignedInt));
	Instruction.Operator.resize(UnsignedInt);

	for (auto& Iter : Instruction.Operator)
	{
		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.Variable.Type.PrimaryType));

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(UnsignedInt));
		BinaryString.resize(UnsignedInt);
		B_IFS.read(BinaryString.data(), UnsignedInt);
		Iter.Variable.Name = String2WString(BinaryString);

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.Scope));
		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.Variable.Type.SecondaryType));

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(UnsignedInt));
		BinaryString.resize(UnsignedInt);
		B_IFS.read(BinaryString.data(), UnsignedInt);
		Iter.Variable.Type.CustomName = String2WString(BinaryString);

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.Extra.Index));
		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.AddressType));

		if (IsClassMember(Iter.Scope) || IsJmpOpCode(Instruction.InsCode) || Iter.Scope == HazeDataDesc::ArrayElement
			|| (Instruction.InsCode == InstructionOpCode::CALL && &Iter == &Instruction.Operator[0]))
		{
			B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.Extra.Address.Offset));	//操作数地址偏移, 指针指之属性应定义单独类型
		}
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << "Read: " << GetInstructionString(Instruction.InsCode);
	for (auto& it : Instruction.Operator)
	{
		WSS << " ----" << it.Variable.Name << " Type: " << (unsigned int)it.Variable.Type.PrimaryType << " Scope: " << (unsigned int)it.Scope << " Base: " << it.Extra.Index
			<< " Offset: " << it.Extra.Address.Offset;
	}
	WSS << std::endl;

	std::wcout << WSS.str();
#endif
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

const FunctionData& HazeVM::GetFunctionByName(const HAZE_STRING& Name)
{
	int Index = GetFucntionIndexByName(Name);
	return Vector_FunctionTable[Index];
}

HazeValue* HazeVM::GetGlobalValue(const HAZE_STRING& Name)
{
	for (auto& Iter : Vector_GlobalData)
	{
		if (Iter.Name == Name)
		{
			return &Iter.Value;
		}
	}

	return nullptr;
}

ClassData* HazeVM::FindClass(const HAZE_STRING& ClassName)
{
	for (auto& Iter : Vector_ClassTable)
	{
		if (Iter.Name == ClassName)
		{
			return &Iter;
		}
	}

	return nullptr;
}

unsigned int HazeVM::GetClassSize(const HAZE_STRING& ClassName)
{
	auto Class = FindClass(ClassName);
	return Class ? Class->Size : 0;
}

bool HazeVM::IsClass(const HAZE_STRING& Name)
{
	return Compiler->IsClass(Name);
}