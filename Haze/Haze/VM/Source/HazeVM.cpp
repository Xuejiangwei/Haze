#include <iostream>
#include <locale>

#include <filesystem>

#include "HazeVM.h"
#include "MemoryPool.h"
#include "HazeStack.h"
#include "HazeCompiler.h"
#include "Parse.h"
#include "HazeLog.h"

#include "BackendParse.h"

extern std::unordered_map<HAZE_STRING, std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)>*> Hash_MapStdLib;

HazeVM::HazeVM()
{
	FunctionReturn.Type = HazeValueType::Void;
	Compiler = std::make_unique<HazeCompiler>();
	VMStack = std::make_unique<HazeStack>(this);

	Vector_MemoryPool.clear();
	Vector_MemoryPool.push_back(std::make_unique<MemoryPool>(this));
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
	if (Compiler->InitializeCompiler(ModuleName))
	{
		Parse P(this);
		P.InitializeFile(FilePath);
		P.ParseContent();
		Compiler->FinishModule();
	}

	auto Module = HashMap_Module.find(ModuleName);
	if (Module == HashMap_Module.end())
	{
		HashMap_Module[ModuleName] = std::make_unique<HazeModule>(Compiler->GetCurrModuleOpFile());
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

	HAZE_BINARY_IFSTREAM FS(OpCodePath, std::ios::in | std::ios::binary);
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
		FS.read(GetBinaryPointer(Vector_GlobalData[i].second), GetSizeByHazeType(Vector_GlobalData[i].second.Type));
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
			Vector_ClassTable[i].Vector_Member[j].Name = String2WString(B_String);

			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Vector_ClassTable[i].Vector_Member[j].Type.PrimaryType));
			FS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Num));

			FS.read(B_String.data(), Num);
			if (Num > 0)
			{
				Vector_ClassTable[i].Vector_Member[j].Type.CustomName = String2WString(B_String);
			}
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

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(UnsignedInt));
		BinaryString.resize(UnsignedInt);
		B_IFS.read(BinaryString.data(), UnsignedInt);
		Iter.Variable.Type.CustomName = String2WString(BinaryString);

		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.Extra.Index));
		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.AddressType));

		if (Iter.Scope == HazeDataDesc::ClassMember_Local_Public)
		{
			B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Iter.Extra.Address.Offset));	//操作数地址偏移, 指针指之属性应定义单独类型
		}
	}

	if (Instruction.InsCode == InstructionOpCode::CALL)
	{
		B_IFS.read(HAZE_BINARY_OP_READ_CODE_SIZE(Instruction.Operator[0].Extra.Call.ParamByteSize));
	}

#if HAZE_INS_LOG
	HAZE_STRING_STREAM WSS;
	WSS << GetInstructionString(Instruction.InsCode) << " ";
	for (auto& it : Instruction.Operator)
	{
		WSS << it.Variable.Name << " " << (unsigned int)it.Variable.Type.PrimaryType << " " << (unsigned int)it.Scope << " " << it.Extra.Index << " ";
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

void* HazeVM::Alloca(HazeValueType Type, unsigned int Size)
{
	void* Ret = nullptr;
	for (auto& Iter : Vector_MemoryPool)
	{
		Ret = Iter->Alloca(Type, Size);
		if (Ret)
		{
			return Ret;
		}
	}
	Vector_MemoryPool.push_back(std::make_unique<MemoryPool>(this));
	Ret = Vector_MemoryPool.back()->Alloca(Type, Size);

	return Ret;
}
