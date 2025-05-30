#include "HazePch.h"
#include "HazeVM.h"
#include "HazeHeader.h"
#include "HazeLog.h"

#include "Parse.h"
#include "Compiler.h"
#include "HazeBaseLibraryDefine.h"
#include "BackendParse.h"
#include "HazeExecuteFile.h"
#include "HazeLibraryManager.h"
#include "HazeDebuggerServer.h"
#include "HazeDebugger.h"
#include "HazeFilePathHelper.h"

#include "HazeStack.h"
#include "HazeMemory.h"

#include "HazeStream.h"

#include "ObjectArray.h"
#include "ObjectString.h"
#include "ObjectClass.h"
#include "ObjectDynamicClass.h"

#include <cstdarg>
#include <filesystem>

extern Unique<HazeDebugger> g_Debugger;
extern Unique<HazeLibraryManager> g_HazeLibManager;
extern void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData);
extern void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args);

HazeVM::HazeVM(HazeRunType GenType) : GenType(GenType)
{
	m_Stack = MakeUnique<HazeStack>(this);
	m_Compiler = MakeUnique<Compiler>(this);
}

HazeVM::~HazeVM()
{
}

bool HazeVM::InitVM(V_Array<HString> Vector_ModulePath)
{
	// 提前注册基本类型
	InitRegisterObjectFunction();

	// 提前注册类
	/*ClassData data;
	data.Name = H_TEXT("UObject");
	data.Size = 8;
	data.Members.push_back({ { HazeValueType::Int32, H_TEXT("A")}, 0, 4, 0 });
	m_Compiler->PreRegisterClass(data);*/

	// 提前解析基础模块，若临时文件夹没有中间文件，生成临时文件
	V_Array<HString> baseModules = { HAZE_BASE_LIBRARY_STREAM_NAME, HAZE_BASE_LIBRARY_MEMORY_NAME, HAZE_BASE_LIBRARY_FILE_NAME };
	for (x_uint64 i = 0; i < baseModules.size(); i++)
	{
		if (!m_Compiler->ParseBaseModule(baseModules[i]))
		{
			return false;
		}
	}

	for (auto& iter : Vector_ModulePath)
	{
		if (ParseFile(iter).empty())
		{
			return false;
		}
	}

	m_Compiler->FinishParse();

#define HAZE_BACKEND_PARSE_ENABLE	1
	{
#if HAZE_BACKEND_PARSE_ENABLE

		if (m_Compiler->IsNewCode())
		{
			BackendParse BP(this);
			BP.Parse();
		}

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

	LoadDLLModules();

	if (IsDebug())
	{
		HazeDebuggerServer::InitDebuggerServer(this);
		//VMDebugger = MakeUnique<HazeDebugger>(this);
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

	return true;
}

void HazeVM::LoadStandardLibrary(V_Array<HString> Vector_ModulePath)
{
}

void HazeVM::CallFunction(const x_HChar* functionName, ...)
{
	auto& function = GetFunctionByName(functionName);
	va_list args;
	//va_start(args, (int)function.Params.size());
	va_start(args, functionName);
	CallHazeFunction(m_Stack.get(), &function, args);
	va_end(args);
}

void HazeVM::CallFunction(const FunctionData* functionData, ...)
{
	va_list args;
	//va_start(args, functionData->Params.size());
	va_start(args, functionData);
	CallHazeFunction(m_Stack.get(), functionData, args);
	va_end(args);
}

void HazeVM::CallFunction(const FunctionData* functionData, va_list& args)
{
	CallHazeFunction(m_Stack.get(), functionData, args);
}

AdvanceFunctionInfo* HazeVM::GetAdvanceFunction(x_uint16 index)
{
	return m_FunctionObjectTable[index];
}

ObjectClass* HazeVM::CreateObjectClass(const x_HChar* className, ...)
{
	auto pair = HazeMemory::AllocaGCData(sizeof(ObjectClass), GC_ObjectType::Class);
	new(pair.first) ObjectClass(pair.second, FindClass(className));

	auto& constructorFunc = GetFunctionByName(GetHazeClassFunctionName(className, className));

	va_list args;
	//va_start(args, constructorFunc.Params.size());
	va_start(args, className);

	auto data = ((ObjectClass*)(pair.first))->m_Data;
	auto dst = &va_arg(args, decltype(data));
	memcpy(dst, &pair.first, sizeof(data));

	CallHazeFunction(m_Stack.get(), &constructorFunc, (va_list&)dst);
	va_end(args);

	return (ObjectClass*)pair.first;
}

bool HazeVM::ParseString(const x_HChar* moduleName, const x_HChar* moduleCode)
{
	bool PopCurrModule = false;
	if (m_Compiler->InitializeCompiler(moduleName, H_TEXT("")))
	{
		PopCurrModule = true;
		Parse P(m_Compiler.get());
		P.InitializeString(moduleCode);

		if (!P.ParseContent())
		{
			return false;
		}
		m_Compiler->FinishModule();
	}

	HashSet_RefModule.insert(moduleName);

	if (PopCurrModule)
	{
		m_Compiler->PopCurrModule();
	}

	return true;
}

HString HazeVM::ParseFile(const HString& FilePath)
{
	bool PopCurrModule = false;
	std::filesystem::path path(FilePath);
	auto dir = path.parent_path().wstring();
	auto moduleName = path.filename().stem().wstring();

	if (m_Compiler->InitializeCompiler(moduleName, FilePath))
	{
		PopCurrModule = true;
		Parse P(m_Compiler.get());
		P.InitializeFile(FilePath);

		if (!P.ParseContent())
		{
			return HString();
		}
		m_Compiler->FinishModule();
	}

	HashSet_RefModule.insert(moduleName);

	if (PopCurrModule)
	{
		m_Compiler->PopCurrModule();
	}

	return moduleName;
}

const HString* HazeVM::GetModuleNameByCurrFunction()
{
	for (size_t i = 0; i < m_FunctionTable.size(); i++)
	{
		auto functionInfo = m_Stack->GetCurrFrame().FunctionInfo;
		if (&m_FunctionTable[i] == functionInfo)
		{
			for (auto& Iter : m_ModuleData)
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

int HazeVM::GetFucntionIndexByName(const HString& m_Name)
{
	auto Iter = m_HashFunctionTable.find(m_Name);
	if (Iter == m_HashFunctionTable.end())
	{
		return -1;
	}
	return Iter->second;
}

const FunctionData& HazeVM::GetFunctionByName(const HString& m_Name)
{
	int index = GetFucntionIndexByName(m_Name);
	return m_FunctionTable[index];
}

const FunctionData* HazeVM::GetFunctionDataByName(const HString& m_Name)
{
	int index = GetFucntionIndexByName(m_Name);
	return index >= 0 ? &m_FunctionTable[index] : nullptr;
}

const ObjectString* HazeVM::GetConstantStringByIndex(int index) const
{
	return m_StringTable[index];
}

char* HazeVM::GetGlobalValueByIndex(x_uint32 Index)
{
	if (Index < m_GlobalData.size())
	{
		return (char*)(&m_GlobalData[Index].Value);
	}

	return nullptr;
}

ClassData* HazeVM::FindClass(const HString& m_ClassName)
{
	for (auto& Iter : m_ClassTable)
	{
		if (Iter.Name == m_ClassName)
		{
			return &Iter;
		}
	}

	return nullptr;
}

x_uint32 HazeVM::GetClassSize(const HString& m_ClassName)
{
	auto Class = FindClass(m_ClassName);
	return Class ? Class->Size : 0;
}

void HazeVM::InitRegisterObjectFunction()
{
	m_Compiler->RegisterAdvanceClassInfo(HazeValueType::Array, { ObjectArray::GetAdvanceClassInfo(), (x_int16)m_FunctionObjectTable.size() });
	for (auto& it : ObjectArray::GetAdvanceClassInfo()->Functions)
	{
		m_FunctionObjectTable.push_back(&it);
	}

	m_Compiler->RegisterAdvanceClassInfo(HazeValueType::String, { ObjectString::GetAdvanceClassInfo(), (x_int16)m_FunctionObjectTable.size() });
	for (auto& it : ObjectString::GetAdvanceClassInfo()->Functions)
	{
		m_FunctionObjectTable.push_back(&it);
	}

	m_Compiler->RegisterAdvanceClassInfo(HazeValueType::Class, { ObjectClass::GetAdvanceClassInfo(), (x_int16)m_FunctionObjectTable.size() });
	for (auto& it : ObjectClass::GetAdvanceClassInfo()->Functions)
	{
		m_FunctionObjectTable.push_back(&it);
	}

	m_Compiler->RegisterAdvanceClassInfo(HazeValueType::DynamicClass, { ObjectDynamicClass::GetAdvanceClassInfo(), (x_int16)m_FunctionObjectTable.size() });
	for (auto& it : ObjectDynamicClass::GetAdvanceClassInfo()->Functions)
	{
		m_FunctionObjectTable.push_back(&it);
	}
}

void HazeVM::InitGlobalStringCount(x_uint64 count)
{
	m_StringTable.resize(count);
}

void HazeVM::SetGlobalString(x_uint64 index, const HString& str)
{
	if (index < m_StringTable.size())
	{
		auto newStr = HazeStream::FormatConstantString(str);
		auto address = HazeMemory::AllocaGCData(sizeof(ObjectString), GC_ObjectType::String);
		new((char*)address.first) ObjectString(address.second, newStr.c_str(), true);
		m_StringTable[index] = (ObjectString*)address.first;
	}
	else
	{
		GLOBAL_INIT_ERR_W("设置第<%d>个字符<%s>超过字符表长度<%d>", index, str.c_str(), m_StringTable.size());
	}
}

const HString* HazeVM::GetSymbolClassName(const HString& name)
{
	auto iter = m_ClassSymbol.find(name);
	if (iter == m_ClassSymbol.end())
	{
		m_ClassSymbol[name] = (x_uint64)-1;

		return &m_ClassSymbol.find(name)->first;
	}

	return &iter->first;
}

void HazeVM::ResetSymbolClassIndex(const HString& name, x_uint64 index)
{
	m_ClassSymbol[name] = index;
}

void HazeVM::LoadDLLModules()
{
	HString dllPath;
	for (auto& m : m_ModuleData)
	{
		if (m.LibType == HazeLibraryType::DLL)
		{
			g_HazeLibManager->LoadDLLLibrary(m.Path + H_TEXT("\\") + m.Name + HAZE_LOAD_DLL_SUFFIX, m.Path + H_TEXT("\\") + m.Name + HAZE_FILE_SUFFIX);
		}
	}
}

void HazeVM::DynamicInitializerForGlobalData()
{
	for (auto iter : m_GlobalInitFunction)
	{
		CallFunction(&m_FunctionTable[iter]);
	}
}

void HazeVM::OnExecLine(x_uint32 Line)
{
	if (g_Debugger)
	{
		g_Debugger->OnExecLine(Line);
	}
}

void HazeVM::InstructionExecPost()
{
}

x_uint32 HazeVM::GetNextLine(x_uint32 CurrLine)
{
	x_uint64 startAddress = m_Stack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	x_uint32 instructionNum = m_Stack->GetCurrFrame().FunctionInfo->InstructionNum;
	for (x_uint64 i = m_Stack->GetCurrPC(); i < startAddress + instructionNum; i++)
	{
		if (m_Instructions[i].InsCode == InstructionOpCode::LINE && m_Instructions[i].Operator[0].Extra.Line > CurrLine)
		{
			return m_Instructions[i].Operator[0].Extra.Line;
		}
	}

	return m_Stack->GetCurrFrame().FunctionInfo->FunctionDescData.EndLine;
}

x_uint32 HazeVM::GetNextInstructionLine(x_uint32 currLine)
{
	x_uint64 startAddress = m_Stack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	x_uint32 instructionNum = m_Stack->GetCurrFrame().FunctionInfo->InstructionNum;
	for (x_uint64 i = m_Stack->GetCurrPC(); i < startAddress + instructionNum; i++)
	{
		if (m_Instructions[i].InsCode == InstructionOpCode::LINE && m_Instructions[i].Operator[0].Extra.Line >= currLine)
		{
			return m_Instructions[i].Operator[0].Extra.Line;
		}
	}

	return m_Stack->GetCurrFrame().FunctionInfo->FunctionDescData.EndLine;
}

Pair<HString, x_uint32> HazeVM::GetStepIn(x_uint32 CurrLine)
{
	x_uint64 startAddress = m_Stack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	x_uint32 instructionNum = m_Stack->GetCurrFrame().FunctionInfo->InstructionNum;
	for (x_uint64 i = m_Stack->GetCurrPC(); i < startAddress + instructionNum; i++)
	{
		if (m_Instructions[i].InsCode == InstructionOpCode::LINE && m_Instructions[i].Operator[0].Extra.Line > CurrLine)
		{
			break;
		}

		if (m_Instructions[i].InsCode == InstructionOpCode::CALL)
		{
			const auto& oper = m_Instructions[i].Operator;
			if (oper.size() >= 1)
			{
				if (oper[0].Variable.Type.PrimaryType == HazeValueType::Function)
				{
					void* value = GetOperatorAddress(m_Stack.get(), oper[0]);
					x_uint64 functionAddress;
					memcpy(&functionAddress, value, sizeof(functionAddress));
					auto function = (FunctionData*)functionAddress;
					if (function->FunctionDescData.Type == InstructionFunctionType::HazeFunction)
					{
						for (size_t j = 0; j < m_FunctionTable.size(); j++)
						{
							if (&m_FunctionTable[j] == function)
							{
								for (auto& Iter : m_ModuleData)
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
						auto& function = m_FunctionTable[functionIndex];
						if (function.FunctionDescData.Type == InstructionFunctionType::HazeFunction)
						{
							return { oper[1].Variable.Name, function.FunctionDescData.StartLine };
						}
					}
				}
			}
		}
	}

	return { HString(), 0 };
}

x_uint32 HazeVM::AddGlobalValue(ObjectClass* value)
{
	m_ExtreGlobalData.push_back(value);
	return (x_uint32)m_ExtreGlobalData.size() - 1;
}

void HazeVM::RemoveGlobalValue(x_uint32 index)
{
	if (m_ExtreGlobalData.size() > index)
	{
		m_ExtreGlobalData[index] = nullptr;
	}
}

void HazeVM::ClearGlobalData()
{
	m_GlobalData.clear();
	m_GlobalData.shrink_to_fit();

	m_ExtreGlobalData.clear();
	m_ExtreGlobalData.shrink_to_fit();

	m_StringTable.clear();
	m_StringTable.shrink_to_fit();
}

x_uint32 HazeVM::GetCurrCallFunctionLine()
{
	auto startAddress = (x_int64)m_Stack->GetCurrFrame().FunctionInfo->FunctionDescData.InstructionStartAddress;
	for (x_int64 i = m_Stack->GetCurrPC(); i >= startAddress; i--)
	{
		if (m_Instructions[i].InsCode == InstructionOpCode::LINE)
		{
			return m_Instructions[i].Operator[0].Extra.Line;
		}
	}

	return m_Stack->GetCurrFrame().FunctionInfo->FunctionDescData.EndLine;
}

//void HazeVM::Hook(HazeVM* m_VM)
//{
//	HAZE_LOG_INFO(H_TEXT("已命中断点\n"));
//}