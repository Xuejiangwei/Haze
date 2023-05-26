#include "HazeVM.h"
#include "HazeLog.h"

#include "Parse.h"
#include "HazeCompiler.h"
#include "BackendParse.h"
#include "HazeExecuteFile.h"

#include "HazeStack.h"
#include "GarbageCollection.h"

HazeVM::HazeVM()
{
	FunctionReturn.first.PrimaryType = HazeValueType::Void;
	VMStack = std::make_unique<HazeStack>(this);
	GC = std::make_unique<GarbageCollection>(this);
	
	Compiler = std::make_unique<HazeCompiler>(this);
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

	{
#if HAZE_LOAD_OP_CODE_ENABLE

		HazeExecuteFile ExeFile(ExeFileType::In);
		ExeFile.ReadExecuteFile(this);

#endif // ENABLE_LOAD_OP_CODE
	}

	Compiler.release();
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

//void HazeVM::ParseString(const HAZE_STRING& String)
//{
//	Parse P(Compiler.get());
//	P.InitializeString(String);
//	P.ParseContent();
//}

void HazeVM::ParseFile(const HAZE_STRING& FilePath, const HAZE_STRING& ModuleName)
{
	bool PopCurrModule = false;
	if (Compiler->InitializeCompiler(ModuleName))
	{
		PopCurrModule = true;
		Parse P(Compiler.get());
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

void HazeVM::SetHook()
{

}
