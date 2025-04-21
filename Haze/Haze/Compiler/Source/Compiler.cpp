#include "HazePch.h"
#include "HazeVM.h"
#include "HazeLogDefine.h"
#include "HazeFilePathHelper.h"

#include "Compiler.h"
#include "CompilerBlock.h"
#include "CompilerStringValue.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerValue.h"
#include "CompilerFunction.h"
#include "CompilerElementValue.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "CompilerClass.h"
#include "CompilerModule.h"
#include "CompilerHelper.h"

//static Share<HazeCompilerFunction> s_GlobalVariebleDefine = MakeShare<HazeCompilerFunction>();

static HashMap<HString, Share<CompilerValue>> g_GlobalRegisters = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeValueType::Void, HazeVariableScope::Global, HazeDataDesc::RegisterRet, 0) },
	//{ NEW_REGISTER, nullptr },
	{ CMP_REGISTER, CreateVariable(nullptr, HazeValueType::Void, HazeVariableScope::Global, HazeDataDesc::RegisterCmp, 0) },
};

// 只用两个临时变量寄存器, 每个函数栈都要存储下这两个临时寄存器, GC时也要扫描
static HashMap<HString, Share<CompilerValue>> g_GlobalTempRegisters = {
	/*{ TEMP_REGISTER_A, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_B, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_2, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_3, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_4, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_5, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_6, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_7, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_8, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },
	{ TEMP_REGISTER_9, CreateVariable(nullptr, HazeDefineVariable(HazeDefineType(HazeValueType::Void),
		H_TEXT("")), HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0) },*/
};

Compiler::Compiler(HazeVM* vm) : m_VM(vm), m_MarkError(false)
{
	m_AdvanceClassInfo.clear();
}

Compiler::~Compiler()
{
}

void Compiler::RegisterAdvanceClassInfo(HazeValueType type, AdvanceClassInfo& info)
{
	if (IsAdvanceType(type))
	{
		m_AdvanceClassInfo[type] = info;
	}
	else
	{
		HAZE_LOG_ERR_W("添加类型信息错误!\n");
	}
}

void Compiler::PreRegisterClass(const ClassData& data)
{
	auto m = MakeUnique<CompilerModule>(this, H_TEXT("PreRegisterModule"), H_TEXT(""));

	m_CompilerBaseModules[H_TEXT("PreRegisterModule")] = m.get();

	V_Array<Pair<HString, Share<CompilerValue>>> classDatas;
	for (int i = 0; i < data.Members.size(); i++)
	{
		classDatas.push_back({ data.Members[i].Variable.Name, CreateVariable(m.get(),
			data.Members[i].Variable.Type, HazeVariableScope::Local, HazeDataDesc::ClassFunction_Local_Public, 0) });
	}

	V_Array<CompilerClass*> parentClass;
	m->CreateClass(data.Name, parentClass, classDatas);
	
	m_CompilerModules[H_TEXT("PreRegisterModule")] = Move(m);
}

void Compiler::PreRegisterVariable()
{
}

void Compiler::PreRegisterFunction()
{
}

bool Compiler::InitializeCompiler(const HString& moduleName, const HString& path)
{
	//生成模块
	auto It = m_CompilerModules.find(moduleName);
	if (It != m_CompilerModules.end())
	{
		return false;
	}

	m_ModuleNameStack.push_back(moduleName);
	m_CompilerModules[moduleName] = MakeUnique<CompilerModule>(this, moduleName, path);
	return true;
}

void Compiler::FinishParse()
{
	HAZE_OFSTREAM ofstream;
	ofstream.imbue(std::locale("chs"));
	ofstream.open(GetIntermediateModuleFile(HAZE_INTER_SYMBOL_TABLE));

	HAZE_STRING_STREAM hss;

	hss << GetSymbolBeginHeader() << std::endl;
	for (auto& m : m_CompilerModules)
	{
		for (auto& className : m.second->m_HashMap_Classes)
		{
			hss << className.first << std::endl;
		}
	}
	hss << GetSymbolEndHeader() << std::endl;

	ofstream << hss.str();
	ofstream.close();


	for (auto& m : m_CompilerModules)
	{
		m.second->GenCodeFile();
	}
}

CompilerModule* Compiler::ParseBaseModule(const x_HChar* moduleName, const x_HChar* moduleCode)
{
	m_VM->ParseString(moduleName, moduleCode);
	m_CompilerBaseModules[moduleName] = GetModule(moduleName);

	return m_CompilerBaseModules[moduleName];
}

CompilerModule* Compiler::ParseModule(const HString& modulePath)
{
	auto moduleName = m_VM->ParseFile(GetModuleFilePath(modulePath, &GetCurrModule()->GetPath()));
	if (!moduleName.empty())
	{
		return GetModule(moduleName);
	}

	return nullptr;
}

void Compiler::FinishModule()
{
	m_CompilerModules[GetCurrModuleName()]->FinishModule();
	ClearBlockPoint();
}

CompilerModule* Compiler::GetModule(const HString& name)
{
	auto Iter = m_CompilerModules.find(name);
	if (Iter != m_CompilerModules.end())
	{
		return Iter->second.get();
	}

	return nullptr;
}

const HString* Compiler::GetModuleName(const CompilerModule* compilerModule) const
{
	for (auto& Iter : m_CompilerModules)
	{
		if (Iter.second.get() == compilerModule)
		{
			return &(Iter.first);
		}
	}

	return nullptr;
}

const HString* Compiler::GetModuleTableClassName(const HString& name)
{
	auto cla = GetCurrModule()->GetClass(name);
	if (cla)
	{
		cla->GetName();
	}
	return nullptr;
}

const HString* Compiler::GetModuleTableEnumName(const HString& name)
{
	auto e = CompilerModule::GetEnum(GetCurrModule().get(), name);
	if (e)
	{
		e->GetName();
	}
	return nullptr;
}

void Compiler::RegisterClassToSymbolTable(const HString& className)
{
	for (auto iter = m_CacheSymbols.begin(); iter != m_CacheSymbols.end(); iter++)
	{
		if (*iter == className)
		{
			return;
		}
	}

	m_CacheSymbols.push_back({ className });
	m_SymbolTable[{ &m_CacheSymbols.back() }] = nullptr;
}

void Compiler::OnCreateClass(Share<CompilerClass> compClass)
{
	auto iter = m_SymbolTable.find({ &compClass->GetName() });
	if (iter->second == nullptr)
	{
		iter->second = compClass;
		return;
	}
	else
	{
		COMPILER_ERR_MODULE_W("重复创建类<%s>", compClass->GetName().c_str(), GetCurrModuleName().c_str());
		return;
	}
}

const HString* Compiler::GetSymbolTableNameAddress(const HString& className)
{
	auto iter = m_SymbolTable.find({ &className });
	if (iter != m_SymbolTable.end())
	{
		return iter->first.Str;
	}
	return nullptr;
}

Share<CompilerEnum> Compiler::GetBaseModuleEnum(const HString& name)
{
	for (auto& it : m_CompilerBaseModules)
	{
		auto ret = it.second->GetEnum_Internal(name);
		if (ret)
		{
			return ret;
		}
	}
	
	return nullptr;
}

Share<CompilerValue> Compiler::GetBaseModuleGlobalVariable(const HString& name)
{
	for (auto& it : m_CompilerBaseModules)
	{
		auto ret = it.second->GetGlobalVariable_Internal(name);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

Share<CompilerClass> Compiler::GetBaseModuleClass(const HString& className)
{
	for (auto& it : m_CompilerBaseModules)
	{
		auto ret = it.second->GetClass(className);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

bool Compiler::GetBaseModuleGlobalVariableName(const Share<CompilerValue>& value, HString& outName, bool getOffset, 
	V_Array<Pair<x_uint64, CompilerValue*>>* offsets)
{
	for (auto& it : m_CompilerBaseModules)
	{
		if (it.second->GetGlobalVariableName_Internal(value, outName, getOffset, offsets))
		{
			return true;
		}
	}

	return false;
}

void Compiler::GetRealTemplateTypes(const TemplateDefineTypes& types, V_Array<HazeDefineType>& defineTypes)
{
	for (auto& type : types.Types)
	{
		if (type.IsDefines)
		{
			HazeDefineType t;
			t.PrimaryType = HazeValueType::Function;

			HString className;
			GetTemplateClassName(className, type.Defines->Types);
		}
		else
		{
			defineTypes.push_back(*type.Type);
		}
	}
}

void Compiler::InsertLineCount(x_int64 lineCount)
{
	if (m_VM->IsDebug() && m_InsertBaseBlock)
	{
		HAZE_LOG_INFO(H_TEXT("行 %d\n"), lineCount);
		m_InsertBaseBlock->PushIRCode(GenIRCode(InstructionOpCode::LINE, lineCount));
	}
}

bool Compiler::IsDebug() const
{
	return m_VM->IsDebug();
}

Unique<CompilerModule>& Compiler::GetCurrModule()
{
	return m_CompilerModules[GetCurrModuleName()];
}

bool Compiler::CurrModuleIsStdLib()
{
	return GetCurrModule()->GetModuleLibraryType() == HazeLibraryType::Static;
}

Share<CompilerFunction> Compiler::GetFunction(const HString& name)
{
	for (auto& iter : m_CompilerModules)
	{
		auto function = iter.second->GetFunction(name);
		if (function)
		{
			return function;
		}
	}

	return nullptr;
}

//Share<CompilerValue> Compiler::GetNewRegister(CompilerModule* compilerModule, const HazeDefineType& data)
//{
//	auto iter = g_GlobalRegisters.find(NEW_REGISTER);
//	if (iter != g_GlobalRegisters.end())
//	{
//		HazeDefineType type(data);
//		iter->second = CreateVariable(compilerModule, HazeDefineVariable(type, NEW_REGISTER), HazeVariableScope::Global, HazeDataDesc::RegisterNew, 0);
//
//		return iter->second;
//	}
//
//	return nullptr;
//}

Share<CompilerValue> Compiler::GetTempRegister(Share<CompilerValue> v)
{
	return GetTempRegister(v.get());
}

Share<CompilerValue> Compiler::GetTempRegister(const CompilerValue* v)
{
	if (v->IsElement())
	{
		v = dynamic_cast<const CompilerElementValue*>(v)->GetElement().get();
	}
	
	if (v->IsArray())
	{
		auto arrayValue = dynamic_cast<const CompilerArrayValue*>(v);
		return GetTempRegister(v->GetValueType(), arrayValue->GetArrayDimension());
	}
	else
	{
		return GetTempRegister(v->GetValueType());
	}
}

Share<CompilerValue> Compiler::GetTempRegister(const HazeDefineType& type, x_uint64 arrayDimension)
{
	auto& currModule = GetCurrModule();
	if (currModule)
	{
		auto currFunc = currModule->GetCurrFunction();
		if (currFunc)
		{
			//局部临时寄存器
			return currFunc->CreateTempRegister(type, arrayDimension);
		}
		else
		{
			//全局临时寄存器
			return currModule->GetGlobalDataFunction()->CreateTempRegister(type, arrayDimension);
		}
	}
	/*for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second.use_count() == 1)
		{
			iter.second->SetDataDesc(HazeDataDesc::RegisterTemp);
			return iter.second;
		}
	}*/

	HAZE_LOG_ERR_W("不能获得临时寄存器!\n");
	return nullptr;
}

HashMap<const x_HChar*, Share<CompilerValue>> Compiler::GetUseTempRegister()
{
	HashMap<const x_HChar*, Share<CompilerValue>> registers;
	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second.use_count() >= 2)
		{
			registers.insert({ iter.first.c_str(), iter.second});
		}
	}

	return registers;
}

void Compiler::ClearTempRegister(const HashMap<const x_HChar*, Share<CompilerValue>>& useTempRegisters)
{
	for (auto & useRegi : useTempRegisters)
	{
		for (auto& regi : g_GlobalTempRegisters)
		{
			if (regi.first == useRegi.first)
			{
				regi.second = CreateVariable(nullptr, HazeValueType::Void, HazeVariableScope::Global, HazeDataDesc::RegisterTemp, 0);
				break;
			}
		}
	}
}

void Compiler::ResetTempRegister(const HashMap<const x_HChar*, Share<CompilerValue>>& useTempRegisters)
{
	for (auto& useRegi : useTempRegisters)
	{
		for (auto& regi : g_GlobalTempRegisters)
		{
			if (regi.first == useRegi.first)
			{
				regi.second = useRegi.second;
				break;
			}
		}
	}
}

Share<CompilerValue> Compiler::GetRegister(const x_HChar* name)
{
	auto iter = g_GlobalRegisters.find(name);
	if (iter != g_GlobalRegisters.end())
	{
		return iter->second;
	}

	return nullptr;
}

const x_HChar* Compiler::GetRegisterName(const Share<CompilerValue>& compilerRegister)
{
	for (auto& iter : g_GlobalRegisters)
	{
		if (iter.second == compilerRegister)
		{
			return iter.first.c_str();
		}
	}

	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second == compilerRegister)
		{
			return iter.first.c_str();
		}
	}

	//HAZE_LOG_ERR_W("查找寄存器错误,未能找到!\n");
	return H_TEXT("");
}

bool Compiler::IsClass(const HString& name)
{
	auto& currModule = GetCurrModule();
	if (currModule->GetClass(name))
	{
		return true;
	}
	else
	{
		for (auto& iter : currModule->m_ImportModules)
		{
			if (iter->GetClass(name))
			{
				return true;
			}
		}

		for (auto& iter : m_CompilerBaseModules)
		{
			if (iter.second->GetClass(name))
			{
				return true;
			}
		}
	}

	return false;
}

bool Compiler::IsEnum(const HString& name)
{
	auto& currModule = GetCurrModule();
	if (currModule.get()->GetEnum(currModule.get(), name))
	{
		return true;
	}

	return false;
}

bool Compiler::IsTemplateClass(const HString& name)
{
	auto& currModule = GetCurrModule();
	if (currModule->IsTemplateClass(name))
	{
		return true;
	}
	else
	{
		for (auto& iter : currModule->m_ImportModules)
		{
			if (iter->IsTemplateClass(name))
			{
				return true;
			}
		}

		for (auto& iter : m_CompilerBaseModules)
		{
			if (iter.second->IsTemplateClass(name))
			{
				return true;
			}
		}
	}

	return false;
}

void Compiler::MarkParseTemplate(bool begin, const HString* moduleName)
{
	static HString cacheFunctionName;
	static Share<CompilerBlock> cacheInsertBlock;

	if (begin)
	{
		cacheFunctionName = GetCurrModule()->m_CurrFunction;
		cacheInsertBlock = m_InsertBaseBlock;
		m_ModuleNameStack.push_back(*moduleName);
		GetCurrModule()->RestartTemplateModule(*moduleName);
	}
	else
	{
		m_ModuleNameStack.pop_back();
		GetCurrModule()->m_CurrFunction = cacheFunctionName;
		m_InsertBaseBlock = cacheInsertBlock;
		cacheFunctionName.clear();
		cacheInsertBlock = nullptr;
	}
}

Share<CompilerValue> Compiler::GenConstantValue(HazeValueType type, const HazeValue& var, HazeValueType* varType)
{
#define SET_VAR_VALUE(TYPE) memset(((void*)&ret->GetValue()), 0, sizeof(ret->GetValue())); \
	memcpy(((void*)&ret->GetValue().Value.##TYPE), &s_Value.Value.##TYPE, sizeof(s_Value.Value.##TYPE))

	static HazeDefineType s_DefineVariableType;
	static HazeValue  s_Value;

	ConvertBaseTypeValue(type, s_Value, varType ? *varType : type, var);

	s_DefineVariableType.PrimaryType = type;

	Share<CompilerValue> ret = nullptr;
	switch (type)
	{
	case HazeValueType::Bool:
	{
		s_Value.Value.Bool = var.Value.Bool == true;
		auto it = m_BoolConstantValues.find(s_Value.Value.Bool);
		if (it != m_BoolConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_BoolConstantValues[s_Value.Value.Bool] = ret;
		SET_VAR_VALUE(Bool);
	}
		break;
	case HazeValueType::Int8:
	{
		auto it = m_Int8_ConstantValues.find(s_Value.Value.Int8);
		if (it != m_Int8_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int8_ConstantValues[s_Value.Value.Int8] = ret;
		SET_VAR_VALUE(Int8);
	}
		break;
	case HazeValueType::UInt8:
	{
		auto it = m_UInt8_ConstantValues.find(s_Value.Value.UInt8);
		if (it != m_UInt8_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt8_ConstantValues[s_Value.Value.UInt8] = ret;
		SET_VAR_VALUE(UInt8);
	}
		break;
	case HazeValueType::Int16:
	{
		auto it = m_Int16_ConstantValues.find(s_Value.Value.Int16);
		if (it != m_Int16_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int16_ConstantValues[s_Value.Value.Int16] = ret;
		SET_VAR_VALUE(Int16);
	}
		break;
	case HazeValueType::UInt16:
	{
		auto it = m_UInt16_ConstantValues.find(s_Value.Value.UInt16);
		if (it != m_UInt16_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt16_ConstantValues[s_Value.Value.UInt16] = ret;
		SET_VAR_VALUE(UInt16);
	}
		break;
	case HazeValueType::Int32:
	{
		auto it = m_Int32_ConstantValues.find(s_Value.Value.Int32);
		if (it != m_Int32_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int32_ConstantValues[s_Value.Value.Int32] = ret;
		SET_VAR_VALUE(Int32);
	}
		break;
	case HazeValueType::UInt32:
	{
		auto it = m_UInt32_ConstantValues.find(s_Value.Value.UInt32);
		if (it != m_UInt32_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt32_ConstantValues[s_Value.Value.UInt32] = ret;
		SET_VAR_VALUE(UInt32);
	}
		break;
	case HazeValueType::Int64:
	{
		auto it = m_Int64_ConstantValues.find(s_Value.Value.Int64);
		if (it != m_Int64_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Int64_ConstantValues[s_Value.Value.Int64] = ret;
		SET_VAR_VALUE(Int64);
	}
		break;
	case HazeValueType::UInt64:
	{
		auto it = m_UInt64_ConstantValues.find(s_Value.Value.UInt64);
		if (it != m_UInt64_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_UInt64_ConstantValues[s_Value.Value.UInt64] = ret;
		SET_VAR_VALUE(UInt64);
	}
		break;
	case HazeValueType::Float32:
	{
		auto it = m_Float32_ConstantValues.find(s_Value.Value.Float32);
		if (it != m_Float32_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Float32_ConstantValues[s_Value.Value.Float32] = ret;
		SET_VAR_VALUE(Float32);
	}
		break;
	case HazeValueType::Float64:
	{
		auto it = m_Float64_ConstantValues.find(s_Value.Value.Float64);
		if (it != m_Float64_ConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_Float64_ConstantValues[s_Value.Value.Float64] = ret;
		SET_VAR_VALUE(Float64);
	}
		break;
	default:
		HAZE_LOG_ERR_W("未支持生成<%s>常量类型!\n", GetHazeValueTypeString(type));
		break;
	}
	return ret;
}

Share<CompilerValue> Compiler::GenStringVariable(const HString& str)
{
	return GetCurrModule()->GetOrCreateGlobalStringVariable(str);
}

Share<CompilerValue> Compiler::GetGlobalVariable(const HString& name)
{
	return CompilerModule::GetGlobalVariable(GetCurrModule().get(), name);
}

Share<CompilerValue> Compiler::GetLocalVariable(const HString& name, HString* nameSpace)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(name, nameSpace);
}

Share<CompilerValue> Compiler::GetEnumVariable(const HString& enumName, const HString& name)
{
	return  GetCurrModule()->GetEnum(GetCurrModule().get(), enumName)->GetEnumValue(name);
}

Share<CompilerValue> Compiler::GetConstantValueInt(int v)
{
	HazeValue Value;
	Value.Value.Int32 = v;

	return GenConstantValue(HazeValueType::Int32, Value);
}

Share<CompilerValue> Compiler::GetConstantValueUint64(x_uint64 v)
{
	HazeValue Value;
	Value.Value.UInt64 = v;

	return GenConstantValue(HazeValueType::UInt64, Value);
}

Share<CompilerValue> Compiler::GenConstantValueBool(bool isTrue)
{
	HazeValue Value;
	Value.Value.Bool = isTrue;

	return GenConstantValue(HazeValueType::Bool, Value);
}

Share<CompilerValue> Compiler::GetNullPtr(const HazeDefineType& type)
{
	static HashMap<HazeDefineType, Share<CompilerValue>, HazeDefineTypeHashFunction> s_NullPtrs;

	auto Iter = s_NullPtrs.find(type);
	if (Iter != s_NullPtrs.end())
	{
		return Iter->second;
	}

	s_NullPtrs[type] = CreateVariable(nullptr, type, HazeVariableScope::Global, HazeDataDesc::NullPtr, 0);
	return s_NullPtrs[type];
}

bool Compiler::IsConstantValueBoolTrue(Share<CompilerValue> v)
{
	return v == GenConstantValueBool(true);
}

bool Compiler::IsConstantValueBoolFalse(Share<CompilerValue> v)
{
	return v == GenConstantValueBool(false);
}

void Compiler::SetInsertBlock(Share<CompilerBlock> block)
{
	m_InsertBaseBlock = block;
}

Share<CompilerBlock> Compiler::GetInsertBlock()
{
	if (m_InsertBaseBlock)
	{
		return m_InsertBaseBlock;
	}
	return nullptr;
}

void Compiler::ClearBlockPoint()
{
	m_InsertBaseBlock = nullptr;
}

bool Compiler::IsCompileError() const
{
	return m_MarkError;
}

void Compiler::MarkCompilerError()
{
	m_MarkError = true;
}

void Compiler::AddImportModuleToCurrModule(CompilerModule* compilerModule)
{
	for (auto& iter : GetCurrModule()->m_ImportModules)
	{
		if (iter == compilerModule)
		{
			return;
		}
	}

	GetCurrModule()->m_ImportModules.push_back(compilerModule);
}

Share<CompilerValue> Compiler::CreateLea(Share<CompilerValue> allocaValue, Share<CompilerValue> value)
{
	/*if (value->IsArrayElement())
	{
		return GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, CreatePointerToArrayElement(value), InstructionOpCode::MOV);
	}
	else*/ if (value->IsArray())
	{
		return CreateMov(allocaValue, value);
	}
	else
	{
		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::LEA);
		return allocaValue;
	}
}

Share<CompilerValue> Compiler::CreateMov(Share<CompilerValue> allocaValue, Share<CompilerValue> value)
{
	/*if (allocaValue->IsRefrence() && !value->IsRefrence())
	{
		if (value->IsConstant())
		{
			CreateMovToPV(allocaValue, value);
		}
		else 
		{
			CreateLea(allocaValue, value);
		}
	}
	else*/
	{
		if (CanCVT(allocaValue->GetValueType().PrimaryType, value->GetValueType().PrimaryType))
		{
			value = CreateCVT(allocaValue, value);
		}

		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, 
			value->IsDynamicClassUnknow() || allocaValue->IsDynamicClassUnknow() ? InstructionOpCode::MOV_DCU : InstructionOpCode::MOV);
	}

	return allocaValue;
}

Share<CompilerValue> Compiler::CreateMovToPV(Share<CompilerValue> allocaValue, Share<CompilerValue> value)
{
	GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOVTOPV ,false);
	return allocaValue;
}

Share<CompilerValue> Compiler::CreateMovPV(Share<CompilerValue> allocaValue, Share<CompilerValue> value)
{
	//if (value->IsRefrence())
	{
		HazeDefineType& allocaValueType = const_cast<HazeDefineType&>(allocaValue->GetValueType());
		
		{
			allocaValueType.PrimaryType = HazeValueType::UInt64;//ValueValueType.SecondaryType;
			allocaValueType.CustomName = nullptr;//ValueValueType.CustomName;

			allocaValueType.SecondaryType = HazeValueType::None;
		}

		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOVPV, false);
		return allocaValue;
	}
	/*else
	{
		COMPILER_ERR_MODULE_W("生成<%s>操作错误", GetInstructionString(InstructionOpCode::MOVPV), GetCurrModuleName().c_str());
	}*/
}


Share<CompilerValue> Compiler::CreateVariableBySection(HazeSectionSignal section, Unique<CompilerModule>& mod, Share<CompilerFunction> func,
	const HazeDefineVariable& var, int line, Share<CompilerValue> refValue, x_uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	switch (section)
	{
	case HazeSectionSignal::Global:
		return CreateGlobalVariable(mod, var, line, refValue, arrayDimension, params);
	case HazeSectionSignal::Local:
		return CreateLocalVariable(func, var, line, refValue, arrayDimension, params);
	case HazeSectionSignal::Static:
		break;
	case HazeSectionSignal::Class:
		return CreateClassVariable(mod, var, refValue, arrayDimension, params);
	case HazeSectionSignal::Enum:
		break;
	default:
		break;
	}

	return nullptr;
}

Share<CompilerValue> Compiler::CreateLocalVariable(Share<CompilerFunction> function, const HazeDefineVariable& variable, int line,
	Share<CompilerValue> refValue, x_uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	return function->CreateLocalVariable(variable, line, refValue, arrayDimension, params);
}

Share<CompilerValue> Compiler::CreateGlobalVariable(Unique<CompilerModule>& compilerModule, const HazeDefineVariable& var, int line,
	Share<CompilerValue> refValue, x_uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	return compilerModule->CreateGlobalVariable(var, line, refValue, arrayDimension, params);
}

Share<CompilerValue> Compiler::CreateClassVariable(Unique<CompilerModule>& compilerModule, const HazeDefineVariable& var,
	Share<CompilerValue> refValue, x_uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	return CreateVariable(compilerModule.get(), var.Type, HazeVariableScope::None, HazeDataDesc::Class, 0, refValue, arrayDimension, params);
}

Share<CompilerValue> Compiler::CreateRet(Share<CompilerValue> value)
{
	/*if (value->IsArrayElement())
	{
		value = GetArrayElementToValue(GetCurrModule().get(), value);
	}*/
	GetCurrModule()->GenIRCode_Ret(value);
	return value;
}

Share<CompilerValue> Compiler::CreateAdd(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateAdd(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateSub(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateSub(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateMul(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateMul(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateDiv(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	ReplaceConstantValueByStrongerType(oper1, oper2);
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateDiv(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateMod(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateMod(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateBitAnd(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateBitAnd(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateBitOr(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateBitOr(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateBitXor(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateBitXor(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateShl(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateShl(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateShr(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateShr(assignTo, oper1, oper2);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateFunctionCall(Share<CompilerFunction> function, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo,
	const HString* nameSpace)
{
	return GetCurrModule()->CreateFunctionCall(function, param, thisPointerTo, nameSpace);
}

Share<CompilerValue> Compiler::CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(pointerFunction, param, thisPointerTo);
}

Share<CompilerValue> Compiler::CreateAdvanceTypeFunctionCall(HazeValueType advanceType, const HString& functionName,
	V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo)
{
	auto iter = m_AdvanceClassInfo.find(advanceType);
	if (iter != m_AdvanceClassInfo.end())
	{
		auto func = iter->second.Functions.find(functionName);
		if (func != iter->second.Functions.end())
		{
			if (thisPointerTo->IsElement())
			{
				thisPointerTo = DynamicCast<CompilerElementValue>(thisPointerTo)->CreateGetFunctionCall();
			}

			return GetCurrModule()->CreateAdvanceTypeFunctionCall(func->second, param, thisPointerTo);
		}
		else if (thisPointerTo->IsElement())
		{
			auto classValue = DynamicCast<CompilerClassValue>(DynamicCast<CompilerElementValue>(thisPointerTo)->CreateGetFunctionCall());
			if (classValue)
			{
				return CreateFunctionCall(classValue->GetOwnerClass()->FindFunction(functionName,nullptr), param, classValue);
			}
			else
			{
				COMPILER_ERR_MODULE_W("类型<%s>中的变量不是类, 没有找到<%s>方法", GetHazeValueTypeString(advanceType), functionName.c_str(),
					GetCurrModuleName().c_str());
			}
		}
		else if (thisPointerTo->IsDynamicClass())
		{
			return CreateDynamicClassFunctionCall(thisPointerTo, functionName, param);
		}
		else
		{
			COMPILER_ERR_MODULE_W("类型<%s>没有找到<%s>方法", GetHazeValueTypeString(advanceType), functionName.c_str(),
				GetCurrModuleName().c_str());
		}
	}
	else
	{
		COMPILER_ERR_MODULE_W("类型<%s>没有找到方法信息", GetHazeValueTypeString(advanceType), GetCurrModuleName().c_str());
	}

	return nullptr;
}

//Share<CompilerValue> Compiler::CreateSetElement(Share<CompilerValue> arrayValue, Share<CompilerValue> index, Share<CompilerValue> assignValue)
//{
//	if (true)
//	{
//
//	}
//}

Share<CompilerValue> Compiler::CreateGetArrayElement(Share<CompilerValue> arrayValue, Share<CompilerValue> index)
{
	auto value = DynamicCast<CompilerArrayValue>(arrayValue);
	V_Array<Share<CompilerValue>> params = { index };
	return CreateMov(GetTempRegister(value->GetArrayDimension() > 1 ? value->GetValueType() : value->GetValueType().GetArrayElement(),
		value->GetArrayDimension() -1), CreateAdvanceTypeFunctionCall(HazeValueType::Array, HAZE_ADVANCE_GET_FUNCTION, params, value));
}

Share<CompilerValue> Compiler::CreateSetArrayElement(Share<CompilerValue> arrayValue, Share<CompilerValue> index, Share<CompilerValue> assignValue)
{
	V_Array<Share<CompilerValue>> params = { assignValue, index };
	return CreateAdvanceTypeFunctionCall(HazeValueType::Array, HAZE_ADVANCE_SET_FUNCTION, params, arrayValue);
}

Share<CompilerValue> Compiler::CreateGetClassMember(Share<CompilerValue> classValue, const HString& memberName)
{
	auto v = DynamicCast<CompilerClassValue>(classValue);
	auto index = v->GetMemberIndex(memberName);
	auto classMemberValue = v->GetMember(memberName);

	V_Array<Share<CompilerValue>> params = { GetConstantValueUint64(index) };
	return CreateMov(GetTempRegister(classMemberValue), 
		CreateAdvanceTypeFunctionCall(HazeValueType::Class, HAZE_ADVANCE_GET_FUNCTION, params, classValue));
}

//Share<CompilerValue> Compiler::CreateSetClassMember(Share<CompilerValue> classValue, const HString& memberName, Share<CompilerValue> assignValue)
//{
//	auto v = DynamicCast<CompilerClassValue>(classValue);
//	auto index = (uint64)v->GetOwnerClass()->GetMemberIndex(memberName);
//
//	V_Array<Share<CompilerValue>> params = { assignValue, GetConstantValueUint64(index) };
//	return CreateAdvanceTypeFunctionCall(HazeValueType::Class, HAZE_ADVANCE_SET_FUNCTION, params, classValue);
//}

Share<CompilerValue> Compiler::CreateGetClassMember(Share<CompilerValue> classValue, Share<CompilerValue> member)
{
	auto v = DynamicCast<CompilerClassValue>(classValue);
	auto index = v->GetMemberIndex(member.get());

	V_Array<Share<CompilerValue>> params = { GetConstantValueUint64(index) };
	return CreateMov(GetTempRegister(member),
		CreateAdvanceTypeFunctionCall(HazeValueType::Class, HAZE_ADVANCE_GET_FUNCTION, params, classValue));
}

Share<CompilerValue> Compiler::CreateSetClassMember(Share<CompilerValue> classValue, Share<CompilerValue> member, Share<CompilerValue> assignValue)
{
	auto v = DynamicCast<CompilerClassValue>(classValue);
	auto index = v->GetMemberIndex(member.get());

	V_Array<Share<CompilerValue>> params = { assignValue, GetConstantValueUint64(index) };
	return CreateAdvanceTypeFunctionCall(HazeValueType::Class, HAZE_ADVANCE_SET_FUNCTION, params, classValue);
}

Share<CompilerValue> Compiler::CreateGetDynamicClassMember(Share<CompilerValue> dynamicClassValue, const HString& memberName)
{
	auto prueStr = MakeShare<CompilerStringValue>(GetCurrModule().get(), HazeValueType::PureString, HazeVariableScope::Local, HazeDataDesc::None, 0);
	prueStr->SetPureString(&memberName);
	V_Array<Share<CompilerValue>> params = { prueStr };

	auto ret = CreateAdvanceTypeFunctionCall(HazeValueType::DynamicClass, HAZE_CUSTOM_GET_MEMBER, params, dynamicClassValue);
	auto& type = const_cast<HazeDefineType&>(ret->GetValueType());
	type.DynamicClassUnknow();

	return CreateMov(GetTempRegister(HazeValueType::DynamicClassUnknow), ret);
}

Share<CompilerValue> Compiler::CreateSetDynamicClassMember(Share<CompilerValue> classValue, const HString& memberName, Share<CompilerValue> assignValue)
{
	auto prueStr = MakeShare<CompilerStringValue>(GetCurrModule().get(), HazeValueType::PureString, HazeVariableScope::Local, HazeDataDesc::None, 0);
	prueStr->SetPureString(&memberName);
	V_Array<Share<CompilerValue>> params = { assignValue, prueStr };

	auto ret = CreateAdvanceTypeFunctionCall(HazeValueType::DynamicClass, HAZE_CUSTOM_SET_MEMBER, params, classValue);
	auto& type = const_cast<HazeDefineType&>(ret->GetValueType());
	type.DynamicClassUnknow();

	return CreateMov(GetTempRegister(HazeValueType::DynamicClassUnknow), ret);
}

Share<CompilerValue> Compiler::CreateDynamicClassFunctionCall(Share<CompilerValue> classValue, const HString& functionName, V_Array<Share<CompilerValue>>& params)
{
	auto prueStr = MakeShare<CompilerStringValue>(GetCurrModule().get(), HazeValueType::PureString, HazeVariableScope::Local, HazeDataDesc::None, 0);
	prueStr->SetPureString(&functionName);
	params.push_back(prueStr);

	auto ret = CreateAdvanceTypeFunctionCall(HazeValueType::DynamicClass, HAZE_CUSTOM_CALL_FUNCTION, params, classValue);
	auto& type = const_cast<HazeDefineType&>(ret->GetValueType());
	type.DynamicClassUnknow();

	return CreateMov(GetTempRegister(HazeValueType::DynamicClassUnknow), ret);
}

Share<CompilerValue> Compiler::CreateElementValue(Share<CompilerValue> parentValue, Share<CompilerValue> elementValue)
{
	return MakeShare<CompilerElementValue>(GetCurrModule().get(), parentValue, elementValue);
}

Share<CompilerValue> Compiler::CreateElementValue(Share<CompilerValue> parentValue, const HString& memberName)
{
	if (DynamicCast<CompilerClassValue>(parentValue))
	{
		return MakeShare<CompilerElementValue>(GetCurrModule().get(), parentValue,
			DynamicCast<CompilerClassValue>(parentValue)->GetMember(memberName));
	}
	else if (parentValue->IsDynamicClass())
	{
		return MakeShare<CompilerElementValue>(GetCurrModule().get(), parentValue, memberName);
	}
	else
	{
		COMPILER_ERR_MODULE_W("类型<%s>生成成员<%s>错误", GetHazeValueTypeString(parentValue->GetValueType().PrimaryType),
			memberName.c_str(), GetCurrModuleName().c_str());
		return nullptr;
	}
}

Share<CompilerValue> Compiler::CreatePointerToValue(Share<CompilerValue> value)
{
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		auto pointer = GetTempRegister(value->GetValueType());
		auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
		type.Pointer();
		return CreateLea(pointer, value);
	}
	else
	{
		COMPILER_ERR_MODULE_W("不能对非基本类取地址", GetCurrModuleName().c_str());
		return nullptr;
	}
}

Share<CompilerValue> Compiler::CreatePointerToFunction(Share<CompilerFunction> function, Share<CompilerValue> pointer)
{
	auto tempPointer = GetTempRegister(HazeDefineType(HazeValueType::Function, &function->GetName()));
	tempPointer->SetDataDesc(HazeDataDesc::FunctionAddress);

	return CreateLea(pointer, tempPointer);
}

Share<CompilerValue> Compiler::CreateNew(Share<CompilerFunction> function, const HazeDefineType& data, V_Array<Share<CompilerValue>>* countValue)
{
	return GetCurrModule()->CreateNew(function, data, countValue);
}

Share<CompilerValue> Compiler::CreateCast(const HazeDefineType& type, Share<CompilerValue> value)
{
	auto reg = GetTempRegister(type);
	GetCurrModule()->GenIRCode_BinaryOperater(reg, value, nullptr, InstructionOpCode::CVT);
	return reg;
}

Share<CompilerValue> Compiler::CreateCVT(Share<CompilerValue> left, Share<CompilerValue> right)
{
	auto tempReg = left->IsTempVariable() ? left : GetTempRegister(left->GetValueType());
	GetCurrModule()->GenIRCode_BinaryOperater(tempReg, right, nullptr, InstructionOpCode::CVT);

	return tempReg;
}

Share<CompilerValue> Compiler::CreateBitNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateBitNeg(assignTo, oper1);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateNeg(assignTo, oper1);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateNot(Share<CompilerValue> assignTo, Share<CompilerValue> oper1)
{
	assignTo = assignTo ? assignTo : GetTempRegister(oper1->GetValueType());
	GetCurrModule()->CreateNot(assignTo, oper1);
	return assignTo;
}

Share<CompilerValue> Compiler::CreateInc(Share<CompilerValue> value, bool isPreInc)
{
	return GetCurrModule()->CreateInc(value, isPreInc);
}

Share<CompilerValue> Compiler::CreateDec(Share<CompilerValue> value, bool isPreDec)
{
	return GetCurrModule()->CreateDec(value, isPreDec);
}

void Compiler::CreateJmpToBlock(Share<CompilerBlock> block)
{
	GetCurrModule()->GenIRCode_JmpTo(block);
}

Share<CompilerValue> Compiler::CreateIntCmp(Share<CompilerValue> left, Share<CompilerValue> right)
{
	/*if (left->IsArrayElement())
	{
		left = GetArrayElementToValue(GetCurrModule().get(), left);
	}

	if (right->IsArrayElement())
	{
		right = GetArrayElementToValue(GetCurrModule().get(), right);
	}*/

	GetCurrModule()->GenIRCode_BinaryOperater(nullptr, left, right, InstructionOpCode::CMP);
	return GetRegister(CMP_REGISTER);
}

Share<CompilerValue> Compiler::CreateBoolCmp(Share<CompilerValue> value)
{
	if (IsConstantValueBoolTrue(value))
	{
		auto v = GenConstantValueBool(true);
		auto temp = CreateMov(GetTempRegister(v->GetValueType()), v);
		CreateIntCmp(CreateBitXor(temp, temp,
			GenConstantValueBool(false)), GenConstantValueBool(true));
		return GenConstantValueBool(true);
	}
	else if (IsConstantValueBoolFalse(value))
	{
		auto v = GenConstantValueBool(true);
		auto temp = CreateMov(GetTempRegister(v->GetValueType()), v);
		CreateIntCmp(CreateBitXor(temp, temp,
			GenConstantValueBool(false)), GenConstantValueBool(false));
		return GenConstantValueBool(false);
	}
	else
	{
		return CreateIntCmp(value, GenConstantValueBool(true));
	}
}

Share<CompilerValue> Compiler::CreateFunctionRet(const HazeDefineType& type)
{
	auto retRegister = GetRegister(RET_REGISTER);
	auto& retRegisterType = const_cast<HazeDefineType&>(retRegister->GetValueType());
	retRegisterType = type;
	return CreateMov(GetTempRegister(type), retRegister);
}

void Compiler::ReplaceConstantValueByStrongerType(Share<CompilerValue>& left, Share<CompilerValue>& right)
{
	if (left->IsArray() && IsNumberType(right->GetValueType().PrimaryType))
	{
		return;
	}

	if (left->IsDynamicClassUnknow() || right->IsDynamicClassUnknow())
	{
		return;
	}

	if (left->GetValueType().PrimaryType != right->GetValueType().PrimaryType)
	{
		auto strongerType = GetStrongerType(left->GetValueType().PrimaryType, right->GetValueType().PrimaryType);
		if (left->GetValueType().PrimaryType == strongerType)
		{
			if (right->IsConstant())
			{
				HazeValue v;
				ConvertBaseTypeValue(strongerType, v, right->GetValueType().PrimaryType, right->GetValue());
				right = GenConstantValue(strongerType, v);
			}
			else
			{
				auto tempRegister = GetTempRegister(right->GetValueType());
				right = CreateMov(tempRegister, right);
			}
		}
		else if(right->GetValueType().PrimaryType == strongerType)
		{
			if (left->IsConstant())
			{
				HazeValue v;
				ConvertBaseTypeValue(strongerType, v, left->GetValueType().PrimaryType, left->GetValue());
				left = GenConstantValue(strongerType, v);
			}
			else
			{
				auto tempRegister = GetTempRegister(right->GetValueType());
				left = CreateMov(tempRegister, left);
			}
		}
	}
	
}

void Compiler::CreateCompareJmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock)
{
	GetCurrModule()->GenIRCode_Cmp(cmpType, ifJmpBlock, elseJmpBlock);
}