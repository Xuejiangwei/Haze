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
#include "ObjectHash.h"
#include "ObjectBase.h"
#include "ObjectClosure.h"

#include <cstdarg>

extern Unique<HazeDebugger> g_Debugger;
extern Unique<HazeLibraryManager> g_HazeLibManager;
extern void* const GetOperatorAddress(HazeStack* stack, const InstructionData& insData);
extern void CallHazeFunction(HazeStack* stack, const FunctionData* funcData, va_list& args);

HazeVM::HazeVM(HazeRunType GenType) : GenType(GenType)
{
	m_Stack = MakeUnique<HazeStack>(this);
	m_Compiler = MakeUnique<Compiler>(this);
	m_TypeInfoMap = MakeUnique<HazeTypeInfoMap>(nullptr);
}

HazeVM::~HazeVM()
{
}

bool HazeVM::InitVM(V_Array<STDString> Vector_ModulePath)
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
	V_Array<STDString> baseModules = { HAZE_BASE_LIBRARY_STREAM_NAME, HAZE_BASE_LIBRARY_MEMORY_NAME, HAZE_BASE_LIBRARY_FILE_NAME };
	for (x_uint64 i = 0; i < baseModules.size(); i++)
	{
		if (!m_Compiler->ParseBaseModule(baseModules[i]))
		{
			return false;
		}
	}

	// 提前读取类型信息表
	m_Compiler->ParseTypeInfoFile();

	while (!m_Compiler->IsFinishStage())
	{
		HAZE_LOG_INFO_W("解析阶段<%d>\n", (int)m_Compiler->GetParseStage());
		for (auto& iter : Vector_ModulePath)
		{
			if (ParseFile(iter).empty())
			{
				return false;
			}
		}

		m_Compiler->NextStage();
		if (m_Compiler->IsCompileError())
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

void HazeVM::LoadStandardLibrary(V_Array<STDString> Vector_ModulePath)
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

STDString HazeVM::GetAdvanceFunctionName(x_uint16 index)
{
#define GET_ADVANCE_MAPPING(CLASS) for (x_uint64 i = 0; i < CLASS::GetAdvanceClassInfo()->Functions.size(); i++) \
		{ \
			if (func == &CLASS::GetAdvanceClassInfo()->Functions[i]) \
			{ \
				for (auto it1 : CLASS::GetAdvanceClassInfo()->FunctionMapping) \
				{ \
					if (it1.second == i) \
					{ \
						return it1.first; \
					} \
				} \
			} \
		}

	auto func = m_FunctionObjectTable[index];
	GET_ADVANCE_MAPPING(ObjectArray);
	GET_ADVANCE_MAPPING(ObjectString);
	GET_ADVANCE_MAPPING(ObjectClass);
	GET_ADVANCE_MAPPING(ObjectDynamicClass);
	GET_ADVANCE_MAPPING(ObjectHash);
	GET_ADVANCE_MAPPING(ObjectBase);
	GET_ADVANCE_MAPPING(ObjectClosure);
	
	return H_TEXT("None");
}

ObjectClass* HazeVM::CreateObjectClass(const x_HChar* className, ...)
{
	auto pair = HazeMemory::AllocaGCData(sizeof(ObjectClass), GC_ObjectType::Class);
	new(pair.first) ObjectClass(pair.second, this, m_TypeInfoMap->GetTypeIdByClassName(className));

	auto& constructorFunc = GetFunctionByName(className, className);

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

STDString HazeVM::ParseFile(const STDString& FilePath)
{
	bool PopCurrModule = false;
	std::filesystem::path path(FilePath);
	auto dir = path.parent_path().wstring();
	auto moduleName = path.filename().stem().wstring();

	if (m_Compiler->InitializeCompiler(moduleName, FilePath))
	{
		PopCurrModule = true;
		Parse parse(m_Compiler.get());
		parse.InitializeFile(FilePath);

		if (!parse.ParseContent())
		{
			return STDString();
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

const STDString* HazeVM::GetModuleNameByCurrFunction()
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

const STDString* HazeVM::GetFunctionNameByData(const FunctionData* data)
{
	for (x_uint32 i = 0; i < m_FunctionTable.size(); i++)
	{
		if (&m_FunctionTable[i] == data)
		{
			for (auto& iter : m_HashFunctionTable)
			{
				if (iter.second == i)
				{
					return &iter.first;
				}
			}
		}
	}

	return nullptr;
}

int HazeVM::GetFucntionIndexByName(const STDString& name)
{
	auto Iter = m_HashFunctionTable.find(name);
	if (Iter == m_HashFunctionTable.end())
	{
		return -1;
	}
	return Iter->second;
}

const FunctionData& HazeVM::GetFunctionByName(const STDString& name, const x_HChar* className)
{
	int index = GetFucntionIndexByName(className ? GetHazeClassFunctionName(className, Move(name)) : Move(name));
	return m_FunctionTable[index];
}

const FunctionData* HazeVM::GetFunctionDataByName(const STDString& name, const x_HChar* className)
{
	int index = GetFucntionIndexByName(className ? GetHazeClassFunctionName(className, name) : name);
	return index >= 0 ? &m_FunctionTable[index] : nullptr;
}

const ObjectString* HazeVM::GetConstantStringByIndex(int index) const
{
	return m_StringTable[index];
}

char* HazeVM::GetGlobalValueByIndex(x_uint32 index)
{
	if (index < m_GlobalData.size())
	{
		return (char*)(&m_GlobalData[index].GetValue().Value);
	}

	return nullptr;
}

ClassData* HazeVM::FindClass(x_uint32 typeId)
{
	auto info = m_TypeInfoMap->GetClassNameById(typeId);
	for (auto& Iter : m_ClassTable)
	{
		if (Iter.Name == *info)
		{
			return &Iter;
		}
	}

	return nullptr;
}

ClassData* HazeVM::FindClass(const STDString& name)
{
	for (auto& Iter : m_ClassTable)
	{
		if (Iter.Name == name)
		{
			return &Iter;
		}
	}

	return nullptr;
}

x_uint32 HazeVM::GetClassSize(x_uint32 typeId)
{
	auto Class = FindClass(typeId);
	return Class ? Class->Size : 0;
}

void HazeVM::JitFunction(const FunctionData* func)
{
	if (func)
	{

	}
	HAZE_TO_DO(JIT);
}

void HazeVM::InitRegisterObjectFunction()
{
#define REGISTER_OBJ_FUNCTION(TYPE, OBJ) m_Compiler->RegisterAdvanceClassInfo(HazeValueType::TYPE, { OBJ::GetAdvanceClassInfo(), (x_int16)m_FunctionObjectTable.size() }); \
	for (auto& it : OBJ::GetAdvanceClassInfo()->Functions) \
	{ \
		m_FunctionObjectTable.push_back(&it); \
	}

	REGISTER_OBJ_FUNCTION(Array, ObjectArray);
	REGISTER_OBJ_FUNCTION(String, ObjectString);
	REGISTER_OBJ_FUNCTION(Class, ObjectClass);
	REGISTER_OBJ_FUNCTION(DynamicClass, ObjectDynamicClass);
	REGISTER_OBJ_FUNCTION(Hash, ObjectHash);
	REGISTER_OBJ_FUNCTION(ObjectBase, ObjectBase);
	REGISTER_OBJ_FUNCTION(Closure, ObjectClosure);
}

void HazeVM::InitGlobalStringCount(x_uint64 count)
{
	m_StringTable.resize(count);
}

void HazeVM::SetGlobalString(x_uint64 index, const STDString& str)
{
	if (index < m_StringTable.size())
	{
		auto newStr = HazeStream::FormatConstantString(str);
		/*auto address = HazeMemory::AllocaGCData(sizeof(ObjectString), GC_ObjectType::String);
		new((char*)address.first) ObjectString(address.second, newStr.c_str(), true);*/
		m_StringTable[index] = ObjectString::Create(newStr.c_str(), true);
	}
	else
	{
		GLOBAL_INIT_ERR_W("设置第<%d>个字符<%s>超过字符表长度<%d>", index, str.c_str(), m_StringTable.size());
	}
}

const STDString* HazeVM::GetSymbolClassName(const STDString& name)
{
	auto iter = m_ClassSymbol.find(name);
	if (iter == m_ClassSymbol.end())
	{
		m_ClassSymbol[name] = (x_uint64)-1;

		return &m_ClassSymbol.find(name)->first;
	}

	return &iter->first;
}

void HazeVM::ResetSymbolClassIndex(const STDString& name, x_uint64 index)
{
	m_ClassSymbol[name] = index;
}

void HazeVM::LoadDLLModules()
{
	for (auto& m : m_ModuleData)
	{
		if (m.LibType == HazeLibraryType::DLL)
		{
			g_HazeLibManager->LoadDLLLibrary(STDString(m.Path.c_str()) + H_TEXT("\\") + m.Name.c_str() + HAZE_LOAD_DLL_SUFFIX,
				STDString(m.Path.c_str()) + H_TEXT("\\") + m.Name.c_str() + HAZE_FILE_SUFFIX);
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

Pair<HStringView, x_uint32> HazeVM::GetStepIn(x_uint32 CurrLine)
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
				if (oper[0].Variable.Type.BaseType == HazeValueType::Function)
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