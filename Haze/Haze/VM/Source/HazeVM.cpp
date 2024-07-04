#include "HazeVM.h"
#include "HazeHeader.h"
#include "HazeLog.h"

#include "Parse.h"
#include "HazeCompiler.h"
#include "HazeBaseLibraryDefine.h"
#include "BackendParse.h"
#include "HazeExecuteFile.h"

#include "HazeDebuggerServer.h"
#include "HazeDebugger.h"

#include "HazeStack.h"
#include "HazeMemory.h"
#include <cstdarg>
#include <filesystem>

extern std::unique_ptr<HazeDebugger> g_Debugger;
extern void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData);
extern void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args);

HazeVM::HazeVM(HazeRunType GenType) : GenType(GenType)
{
	VMStack = std::make_unique<HazeStack>(this);
	m_Compiler = std::make_unique<HazeCompiler>(this);
}

HazeVM::~HazeVM()
{
}

void HazeVM::InitVM(std::vector<HAZE_STRING> Vector_ModulePath)
{
	std::vector<HAZE_STRING> baseModules = HAZE_BASE_LIBS;
	for (uint64 i = 0; i + 1 < baseModules.size(); i+=2)
	{
		m_Compiler->ParseBaseModule(baseModules[i].c_str(), baseModules[i + 1].c_str());
	}

	for (auto& iter : Vector_ModulePath)
	{
		ParseFile(iter);
	}

	m_Compiler->FinishParse();

#define HAZE_BACKEND_PARSE_ENABLE	1
	{
#if HAZE_BACKEND_PARSE_ENABLE

		BackendParse BP(this);
		BP.Parse();

#endif
	}

#define HAZE_LOAD_OP_CODE_ENABLE	1

	{
#if HAZE_LOAD_OP_CODE_ENABLE

		HazeExecuteFile ExeFile(ExeFileType::In);
		ExeFile.ReadExecuteFile(this);

#endif

	}

	m_Compiler.release();
	HashSet_RefModule.clear();

	HazeMemory::GetMemory()->SetVM(this);
	
	if (IsDebug())
	{
		HazeDebuggerServer::InitDebuggerServer(this);
		//VMDebugger = std::make_unique<HazeDebugger>(this);
		//VMDebugger->SetHook(&HazeVM::Hook, HazeDebugger::DebuggerHookType::Instruction | HazeDebugger::DebuggerHookType::Line);
	
		while (!g_Debugger)
		{
		}

		DynamicInitializerForGlobalData();
	}
	else
	{
		DynamicInitializerForGlobalData();
	}
}

void HazeVM::LoadStandardLibrary(std::vector<HAZE_STRING> Vector_ModulePath)
{
}

void HazeVM::CallFunction(const HAZE_CHAR* functionName, ...)
{
	auto& function = GetFunctionByName(functionName);
	va_list args;
	va_start(args, function.Params.size());
	CallHazeFunction(VMStack.get(), &function, args);
	va_end(args);
}

void HazeVM::CallFunction(FunctionData* functionData, ...)
{
	va_list args;
	va_start(args, functionData->Params.size());
	CallHazeFunction(VMStack.get(), functionData, args);
	va_end(args);
}

void* HazeVM::CreateHazeClass(const HAZE_STRING& className, ...)
{
	auto hazeClass = FindClass(className);
	auto obj = malloc(hazeClass->Size);

	auto& constructorFunc = GetFunctionByName(GetHazeClassFunctionName(className, className));

	va_list args;
	va_start(args, constructorFunc.Params.size());

	auto dst = &va_arg(args, decltype(obj));
	memcpy(dst, &obj, sizeof(obj));

	CallHazeFunction(VMStack.get(), &constructorFunc, (va_list&)dst);
	va_end(args);

	return obj;
}

void HazeVM::ParseString(const HAZE_CHAR* moduleName, const HAZE_CHAR* moduleCode)
{
	bool PopCurrModule = false;
	if (m_Compiler->InitializeCompiler(moduleName, HAZE_TEXT("")))
	{
		PopCurrModule = true;
		Parse P(m_Compiler.get());
		P.InitializeString(moduleCode);
		P.ParseContent();
		m_Compiler->FinishModule();
	}

	HashSet_RefModule.insert(moduleName);

	if (PopCurrModule)
	{
		m_Compiler->PopCurrModule();
	}
}

void HazeVM::ParseFile(const HAZE_STRING& FilePath)
{
	bool PopCurrModule = false;
	std::filesystem::path path(FilePath); 
	auto dir = path.parent_path().wstring();
	auto moduleName = path.filename().stem().wstring();
	if (m_Compiler->InitializeCompiler(moduleName, dir))
	{
		PopCurrModule = true;
		Parse P(m_Compiler.get());
		P.InitializeFile(FilePath);
		P.ParseContent();
		m_Compiler->FinishModule();
	}

	HashSet_RefModule.insert(moduleName);

	if (PopCurrModule)
	{
		m_Compiler->PopCurrModule();
	}
}

const HAZE_STRING* HazeVM::GetModuleNameByCurrFunction()
{
	for (size_t i = 0; i < Vector_FunctionTable.size(); i++)
	{
		auto functionInfo = VMStack->GetCurrFrame().FunctionInfo;
		if (&Vector_FunctionTable[i] == functionInfo)
		{
			for (auto& Iter : Vector_ModuleData)
			{
				if (Iter.FunctionIndex.first <= i && i < Iter.FunctionIndex.second)
				{
					return &Iter.Name;
				}
			}
		}
	}

	return nullptr;
}

int HazeVM::GetFucntionIndexByName(const HAZE_STRING& m_Name)
{
	auto Iter = HashMap_FunctionTable.find(m_Name);
	if (Iter == HashMap_FunctionTable.end())
	{
		return -1;
	}
	return Iter->second;
}

const FunctionData& HazeVM::GetFunctionByName(const HAZE_STRING& m_Name)
{
	int Index = GetFucntionIndexByName(m_Name);
	return Vector_FunctionTable[Index];
}

const HAZE_CHAR* HazeVM::GetConstantStringByIndex(int Index) const
{
	return Vector_StringTable.at(Index).c_str();
}

char* HazeVM::GetGlobalValueByIndex(uint32 Index)
{
	if (Index < Vector_GlobalData.size())
	{
		if (Vector_GlobalData[Index].second)
		{
			if (IsClassType(Vector_GlobalData[Index].first.m_Type.PrimaryType))
			{
				return (char*)Vector_GlobalData[Index].first.Address;
			}
			else
			{
				return (char*)(&Vector_GlobalData[Index].first.Value);
			}
		}
		else
		{
			Vector_GlobalData[Index].second = true;
			VMStack->RunGlobalDataInit(m_GlobalDataInitAddress[Index].first, m_GlobalDataInitAddress[Index].second);
			if (IsClassType(Vector_GlobalData[Index].first.m_Type.PrimaryType))
			{
				return (char*)Vector_GlobalData[Index].first.Address;
			}
			else
			{
				return (char*)(&Vector_GlobalData[Index].first.Value);
			}
		}
	}
	
	return nullptr;
}

ClassData* HazeVM::FindClass(const HAZE_STRING& m_ClassName)
{
	for (auto& Iter : Vector_ClassTable)
	{
		if (Iter.Name == m_ClassName)
		{
			return &Iter;
		}
	}

	return nullptr;
}

uint32 HazeVM::GetClassSize(const HAZE_STRING& m_ClassName)
{
	auto Class = FindClass(m_ClassName);
	return Class ? Class->Size : 0;
}

void HazeVM::DynamicInitializerForGlobalData()
{
	for (int i = 0; i < Vector_GlobalData.size(); i++)
	{
		Vector_GlobalData[i].second = true;
		VMStack->RunGlobalDataInit(m_GlobalDataInitAddress[i].first, m_GlobalDataInitAddress[i].second);
	}
}

void HazeVM::OnExecLine(uint32 Line)
{
	if (g_Debugger)
	{
		g_Debugger->OnExecLine(Line);
	}
}

void HazeVM::InstructionExecPost()
{
}

uint32 HazeVM::GetNextLine(uint32 CurrLine)
{
	uint32 StartAddress = VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	uint32 InstructionNum = VMStack->GetCurrFrame().FunctionInfo->InstructionNum;
	for (size_t i = VMStack->GetCurrPC(); i < StartAddress + InstructionNum; i++)
	{
		if (Instructions[i].InsCode == InstructionOpCode::LINE && Instructions[i].Operator[0].Extra.Line > CurrLine)
		{
			return Instructions[i].Operator[0].Extra.Line;
		}
	}

	return VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.EndLine;
}

uint32 HazeVM::GetNextInstructionLine(uint32 currLine)
{
	uint32 StartAddress = VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	uint32 InstructionNum = VMStack->GetCurrFrame().FunctionInfo->InstructionNum;
	for (size_t i = VMStack->GetCurrPC(); i < StartAddress + InstructionNum; i++)
	{
		if (Instructions[i].InsCode == InstructionOpCode::LINE && Instructions[i].Operator[0].Extra.Line >= currLine)
		{
			return Instructions[i].Operator[0].Extra.Line;
		}
	}

	return VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.EndLine;
}

std::pair<HAZE_STRING, uint32> HazeVM::GetStepIn(uint32 CurrLine)
{
	uint32 StartAddress = VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	uint32 InstructionNum = VMStack->GetCurrFrame().FunctionInfo->InstructionNum;
	for (size_t i = VMStack->GetCurrPC(); i < StartAddress + InstructionNum; i++)
	{
		if (Instructions[i].InsCode == InstructionOpCode::LINE && Instructions[i].Operator[0].Extra.Line > CurrLine)
		{
			break;
		}

		if (Instructions[i].InsCode == InstructionOpCode::CALL)
		{
			const auto& oper = Instructions[i].Operator;
			if (oper.size() >= 1)
			{
				if (oper[0].Variable.Type.PrimaryType == HazeValueType::PointerFunction)
				{
					void* value = GetOperatorAddress(VMStack.get(), oper[0]);
					uint64 functionAddress;
					memcpy(&functionAddress, value, sizeof(functionAddress));
					auto function = (FunctionData*)functionAddress;
					if (function->FunctionDescData.Type == InstructionFunctionType::HazeFunction)
					{
						for (size_t j = 0; j < Vector_FunctionTable.size(); j++)
						{
							if (&Vector_FunctionTable[j] == function)
							{
								for (auto& Iter : Vector_ModuleData)
								{
									if (Iter.FunctionIndex.first <= i && i < Iter.FunctionIndex.second)
									{
										return { Iter.Name, function->FunctionDescData.StartLine };
									}
								}
							}
						}
					}
				}
				else
				{
					int functionIndex = GetFucntionIndexByName(oper[0].Variable.Name);
					if (functionIndex >= 0)
					{
						auto& function = Vector_FunctionTable[functionIndex];
						if (function.FunctionDescData.Type == InstructionFunctionType::HazeFunction)
						{
							return { oper[1].Variable.Name, function.FunctionDescData.StartLine };
						}
					}
				}
			}
		}
	}

	return { HAZE_STRING(), 0 };
}

uint32 HazeVM::GetCurrCallFunctionLine()
{
	uint32 StartAddress = VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	for (size_t i = VMStack->GetCurrPC(); i >= StartAddress; i--)
	{
		if (Instructions[i].InsCode == InstructionOpCode::LINE)
		{
			return Instructions[i].Operator[0].Extra.Line;
		}
	}

	return VMStack->GetCurrFrame().FunctionInfo->FunctionDescData.EndLine;
}

uint64 HazeVM::GetRegisterArrayLength(uint64 address)
{
	auto iter = Vector_ArrayCache.find(address);
	if (iter != Vector_ArrayCache.end())
	{
		return iter->second;
	}

	return 0;
}

//void HazeVM::Hook(HazeVM* m_VM)
//{
//	HAZE_LOG_INFO(HAZE_TEXT("ÒÑÃüÖÐ¶Ïµã\n"));
//}