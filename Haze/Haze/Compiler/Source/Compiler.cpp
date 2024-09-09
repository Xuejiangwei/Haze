#include "HazePch.h"
#include "HazeVM.h"
#include "HazeLogDefine.h"
#include "HazeFilePathHelper.h"

#include "Compiler.h"
#include "CompilerBlock.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerValue.h"
#include "CompilerFunction.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "CompilerClass.h"
#include "CompilerModule.h"
#include "CompilerHelper.h"

//static Share<HazeCompilerFunction> s_GlobalVariebleDefine = MakeShare<HazeCompilerFunction>();

static HashMap<const HChar*, Share<CompilerValue>> g_GlobalRegisters = {
	{ RET_REGISTER, CreateVariable(nullptr, HazeValueType::Void, HazeVariableScope::Global, HazeDataDesc::RegisterRet, 0) },
	//{ NEW_REGISTER, nullptr },
	{ CMP_REGISTER, CreateVariable(nullptr, HazeValueType::Void, HazeVariableScope::Global, HazeDataDesc::RegisterCmp, 0) },
};

// 只用两个临时变量寄存器, 每个函数栈都要存储下这两个临时寄存器, GC时也要扫描
static HashMap<const HChar*, Share<CompilerValue>> g_GlobalTempRegisters = {
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

Compiler::Compiler(HazeVM* vm) : m_VM(vm)
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

	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<CompilerValue>>>>> classDatas;
	V_Array<Pair<HString, Share<CompilerValue>>> classData;
	for (int i = 0; i < data.Members.size(); i++)
	{
		classData.push_back({ data.Members[i].Variable.Name, CreateVariable(m.get(),
			data.Members[i].Variable.Type, HazeVariableScope::Local, HazeDataDesc::ClassFunction_Local_Public, 0) });
	}
	classDatas.push_back({ HazeDataDesc::ClassFunction_Local_Public, Move(classData) });

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

CompilerModule* Compiler::ParseBaseModule(const HChar* moduleName, const HChar* moduleCode)
{
	m_VM->ParseString(moduleName, moduleCode);
	m_CompilerBaseModules[moduleName] = GetModule(moduleName);

	return m_CompilerBaseModules[moduleName];
}

CompilerModule* Compiler::ParseModule(const HString& modulePath)
{
	auto pos = modulePath.find_last_of(HAZE_MODULE_PATH_CONBINE);
	HString moduleName;
	if (pos == std::string::npos)
	{
		moduleName = modulePath;
	}
	else
	{
		moduleName = modulePath.substr(pos + 1);
	}

	m_VM->ParseFile(GetModuleFilePath(moduleName, &GetCurrModule()->GetPath(), pos == std::string::npos ? nullptr : &modulePath));
	return GetModule(moduleName);
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
	V_Array<Pair<uint64, CompilerValue*>>* offsets)
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

void Compiler::InsertLineCount(int64 lineCount)
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

Pair<Share<CompilerFunction>, Share<CompilerValue>> Compiler::GetFunction(const HString& name)
{
	for (auto& iter : m_CompilerModules)
	{
		auto function = iter.second->GetFunction(name);
		if (function.first)
		{
			return function;
		}
	}

	return { nullptr, nullptr };
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

Share<CompilerValue> Compiler::GetTempRegister(const HazeDefineType& type, uint64 arrayDimension)
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
		}
	}
	else
	{
		HAZE_LOG_ERR_W("不能获得临时寄存器!\n");
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

HashMap<const HChar*, Share<CompilerValue>> Compiler::GetUseTempRegister()
{
	HashMap<const HChar*, Share<CompilerValue>> registers;
	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second.use_count() >= 2)
		{
			registers.insert({ iter.first, iter.second });
		}
	}

	return registers;
}

void Compiler::ClearTempRegister(const HashMap<const HChar*, Share<CompilerValue>>& useTempRegisters)
{
	for (auto & useRegi : useTempRegisters)
	{
		for (auto& regi : g_GlobalTempRegisters)
		{
			if (regi.first == useRegi.first)
			{
				regi.second = CreateVariable(nullptr, HazeValueType::Void, HazeVariableScope::Temp, HazeDataDesc::RegisterTemp, 0);
				break;
			}
		}
	}
}

void Compiler::ResetTempRegister(const HashMap<const HChar*, Share<CompilerValue>>& useTempRegisters)
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

Share<CompilerValue> Compiler::GetRegister(const HChar* name)
{
	auto iter = g_GlobalRegisters.find(name);
	if (iter != g_GlobalRegisters.end())
	{
		return iter->second;
	}

	return nullptr;
}

const HChar* Compiler::GetRegisterName(const Share<CompilerValue>& compilerRegister)
{
	for (auto& iter : g_GlobalRegisters)
	{
		if (iter.second == compilerRegister)
		{
			return iter.first;
		}
	}

	for (auto& iter : g_GlobalTempRegisters)
	{
		if (iter.second == compilerRegister)
		{
			return iter.first;
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
	static HazeDefineType s_DefineVariableType;
	static HazeValue  s_Value;

	ConvertBaseTypeValue(type, s_Value, varType ? *varType : type, var);

	s_DefineVariableType.PrimaryType = type;

	Share<CompilerValue> ret = nullptr;
	switch (type)
	{
	case HazeValueType::Bool:
	{
		auto it = m_BoolConstantValues.find(s_Value.Value.Bool);
		if (it != m_BoolConstantValues.end())
		{
			return it->second;
		}
		ret = CreateVariable(nullptr, s_DefineVariableType, HazeVariableScope::Global, HazeDataDesc::Constant, 0, nullptr, {});
		m_BoolConstantValues[s_Value.Value.Bool] = ret;
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
	}
		break;
	default:
		HAZE_LOG_ERR_W("未支持生成<%s>常量类型!\n", GetHazeValueTypeString(type));
		break;
	}

	memcpy(((void*)&ret->GetValue()), &s_Value, sizeof(s_Value));
	return ret;
}

Share<CompilerValue> Compiler::GenStringVariable(HString& str)
{
	return GetCurrModule()->GetOrCreateGlobalStringVariable(str);
}

Share<CompilerValue> Compiler::GetGlobalVariable(const HString& name)
{
	return CompilerModule::GetGlobalVariable(GetCurrModule().get(), name);
}

Share<CompilerValue> Compiler::GetLocalVariable(const HString& name)
{
	return  GetCurrModule()->GetCurrFunction()->GetLocalVariable(name);
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

Share<CompilerValue> Compiler::GetConstantValueUint64(uint64 v)
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

void Compiler::ClearBlockPoint()
{
	m_InsertBaseBlock = nullptr;
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

Share<CompilerValue> Compiler::CreateMov(Share<CompilerValue> allocaValue, Share<CompilerValue> value, bool storeValue)
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

		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOV);
	}

	return allocaValue;
}

Share<CompilerValue> Compiler::CreateMovToPV(Share<CompilerValue> allocaValue, Share<CompilerValue> value)
{
	GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOVTOPV);
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

		GetCurrModule()->GenIRCode_BinaryOperater(allocaValue, value, nullptr, InstructionOpCode::MOVPV);
		return allocaValue;
	}
	/*else
	{
		COMPILER_ERR_MODULE_W("生成<%s>操作错误", GetInstructionString(InstructionOpCode::MOVPV), GetCurrModuleName().c_str());
	}*/

	return allocaValue;
}


Share<CompilerValue> Compiler::CreateVariableBySection(HazeSectionSignal section, Unique<CompilerModule>& mod, Share<CompilerFunction> func,
	const HazeDefineVariable& var, int line, Share<CompilerValue> refValue, uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	switch (section)
	{
	case HazeSectionSignal::Global:
		return CreateGlobalVariable(mod, var, refValue, arrayDimension, params);
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
	Share<CompilerValue> refValue, uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	return function->CreateLocalVariable(variable, line, refValue, arrayDimension, params);
}

Share<CompilerValue> Compiler::CreateGlobalVariable(Unique<CompilerModule>& compilerModule, const HazeDefineVariable& var,
	Share<CompilerValue> refValue, uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	return compilerModule->CreateGlobalVariable(var, refValue, arrayDimension, params);
}

Share<CompilerValue> Compiler::CreateClassVariable(Unique<CompilerModule>& compilerModule, const HazeDefineVariable& var,
	Share<CompilerValue> refValue, uint64 arrayDimension, V_Array<HazeDefineType>* params)
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

Share<CompilerValue> Compiler::CreateBitNeg(Share<CompilerValue> assignTo, Share<CompilerValue> value)
{
	assignTo = assignTo ? assignTo : GetTempRegister(value->GetValueType());
	GetCurrModule()->CreateBitNeg(assignTo, value);
	return value;
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

Share<CompilerValue> Compiler::CreateNot(Share<CompilerValue> left, Share<CompilerValue> right)
{
	return GetCurrModule()->CreateNot(left, right);
}

Share<CompilerValue> Compiler::CreateFunctionCall(Share<CompilerFunction> function, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(function, param, thisPointerTo);
}

Share<CompilerValue> Compiler::CreateFunctionCall(Share<CompilerValue> pointerFunction, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo)
{
	return GetCurrModule()->CreateFunctionCall(pointerFunction, param, thisPointerTo);
}

Share<CompilerValue> Compiler::CreateAdvanceTypeFunctionCall(HazeValueType advanceType, const HString& functionName, V_Array<Share<CompilerValue>>& param, Share<CompilerValue> thisPointerTo)
{
	auto iter = m_AdvanceClassInfo.find(advanceType);
	if (iter != m_AdvanceClassInfo.end())
	{
		auto func = iter->second.Functions.find(functionName);
		if (func != iter->second.Functions.end())
		{
			return GetCurrModule()->CreateAdvanceTypeFunctionCall(func->second, param, thisPointerTo);
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

	return Compiler::GetRegister(RET_REGISTER);
}

Share<CompilerValue> Compiler::CreateGetArrayElement(Share<CompilerValue> value, Share<CompilerValue> index)
{
	auto arrayValue = DynamicCast<CompilerArrayValue>(value);

	V_Array<Share<CompilerValue>> params = { index };
	value = CreateAdvanceTypeFunctionCall(HazeValueType::Array, HAZE_ADVANCE_GET_FUNCTION, params, value);
	
	return CreateMov(GetTempRegister(arrayValue->GetArrayDimension() > 1 ? arrayValue->GetValueType() : arrayValue->GetValueType().GetArrayElement(),
		arrayValue->GetArrayDimension() -1), value);
}

Share<CompilerValue> Compiler::CreateSetArrayElement(Share<CompilerValue> value, Share<CompilerValue> index, Share<CompilerValue> assignValue)
{
	V_Array<Share<CompilerValue>> params = { assignValue, index };
	return CreateAdvanceTypeFunctionCall(HazeValueType::Array, HAZE_ADVANCE_SET_FUNCTION, params, value);
}

Share<CompilerValue> Compiler::CreateGetClassMember(Share<CompilerValue> value, const HString& memberName)
{
	auto classValue = DynamicCast<CompilerClassValue>(value);
	auto index = (uint64)classValue->GetOwnerClass()->GetMemberIndex(memberName);
	auto classMemberValue = classValue->GetOwnerClass()->GetMemberValue(index);

	V_Array<Share<CompilerValue>> params = { GetConstantValueUint64(index) };
	value = CreateAdvanceTypeFunctionCall(HazeValueType::Class, HAZE_ADVANCE_GET_FUNCTION, params, value);

	return CreateMov(GetTempRegister(classMemberValue), value);
}

Share<CompilerValue> Compiler::CreateSetClassMember(Share<CompilerValue> value, const HString& memberName, Share<CompilerValue> assignValue)
{
	auto classValue = DynamicCast<CompilerClassValue>(value);
	auto index = (uint64)classValue->GetOwnerClass()->GetMemberIndex(memberName);
	auto classMemberValue = classValue->GetOwnerClass()->GetMemberValue(index);

	V_Array<Share<CompilerValue>> params = { assignValue, GetConstantValueUint64(index) };
	return CreateAdvanceTypeFunctionCall(HazeValueType::Class, HAZE_ADVANCE_SET_FUNCTION, params, value);
}

//Share<CompilerValue> Compiler::CreateArrayElement(Share<CompilerValue> value, V_Array<Share<CompilerValue>> indices, Share<CompilerValue> assignTo)
//{
//	auto arr = DynamicCast<CompilerArrayValue>(value);
//
//	V_Array<Share<CompilerValue>> params;
//	for (uint64 i = 0; i < indices.size() - (assignValue ? 1 : 0); i++)
//	{
//		params.clear();
//		params.push_back(indices[i]);
//		value = CreateAdvanceTypeFunctionCall(HazeValueType::Array, HAZE_ADVANCE_GET_FUNCTION, params, value);
//		value = CreateMov(GetTempRegister(i + 1 < indices.size() ? arr->GetValueType() : arr->GetValueType().SecondaryType), value);
//	}
//
//	if (assignTo)
//	{
//		CreateMov(assignTo, value);
//	}
//
//	return nullptr;
//}

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
	}
}

//Share<HazeCompilerValue> HazeCompiler::CreatePointerToArray(Share<HazeCompilerValue> arrValue, Share<HazeCompilerValue> index)
//{
//	auto pointer = GetTempRegister();
//
//	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
//	type.PointerTo(arrValue->GetValueType());
//
//	auto ret = CreateLea(pointer, arrValue);
//	if (index)
//	{
//		CreateAdd(ret, index);
//	}
//
//	return ret;
//}

//Share<HazeCompilerValue> HazeCompiler::CreatePointerToArrayElement(Share<HazeCompilerValue> elementValue)
//{
//	auto arrayElementValue = DynamicCast<HazeCompilerArrayElementValue>(elementValue);
//	auto arrayValue = DynamicCast<HazeCompilerArrayValue>(arrayElementValue->GetArray()->GetShared());
//
//	if (arrayValue)
//	{
//		auto tempRegister = GetTempRegister();
//		Share<HazeCompilerValue> arrayPointer = CreatePointerToArray(arrayValue);
//
//		for (uint64 i = 0; i < arrayElementValue->GetIndex().size(); i++)
//		{
//			uint64 size = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetValue().Value.UnsignedLong
//				: arrayValue->GetSizeByLevel((uint64)i);
//			Share<HazeCompilerValue> sizeValue = nullptr;
//
//			if (arrayElementValue->GetIndex()[i]->IsConstant())
//			{
//				sizeValue = GetConstantValueUint64(size);
//			}
//			else
//			{
//				sizeValue = i == arrayElementValue->GetIndex().size() - 1 ? arrayElementValue->GetIndex()[i]->GetShared() : GetConstantValueUint64(size);
//			}
//
//			CreateMov(tempRegister, sizeValue);
//			if (i != arrayElementValue->GetIndex().size() - 1)
//			{
//				CreateMul(tempRegister, arrayElementValue->GetIndex()[i]->GetShared());
//			}
//			CreateAdd(arrayPointer, tempRegister);
//		}
//
//		return arrayPointer;
//	}
//
//	return nullptr;
//}

//Share<HazeCompilerValue> HazeCompiler::CreatePointerToPointerArray(Share<HazeCompilerValue> pointerArray,
//	Share<HazeCompilerValue> index)
//{
//	auto pointer = GetTempRegister();
//
//	auto& type = const_cast<HazeDefineType&>(pointer->GetValueType());
//	type.PrimaryType = HazeValueType::PointerBase;
//
//	auto ret = CreateMov(pointer, pointerArray);
//	if (index)
//	{
//		CreateAdd(ret, index);
//	}
//
//	return ret;
//}

Share<CompilerValue> Compiler::CreatePointerToFunction(Share<CompilerFunction> function, Share<CompilerValue> pointer)
{
	auto tempPointer = GetTempRegister(HazeDefineType(HazeValueType::Function, &function->GetName()));
	tempPointer->SetDataDesc(HazeDataDesc::FunctionAddress);

	return CreateLea(pointer, tempPointer);
}

Share<CompilerValue> Compiler::CreateNew(Share<CompilerFunction> function, const HazeDefineType& data, V_Array<Share<CompilerValue>>* countValue)
{
	return function->CreateNew(data, countValue);
}

Share<CompilerValue> Compiler::CreateCast(const HazeDefineType& type, Share<CompilerValue> value)
{
	auto reg = GetTempRegister(type);
	GetCurrModule()->GenIRCode_BinaryOperater(reg, reg, value, InstructionOpCode::CVT);

	return reg;
}

Share<CompilerValue> Compiler::CreateCVT(Share<CompilerValue> left, Share<CompilerValue> right)
{
	auto tempReg = GetTempRegister(left->GetValueType());
	GetCurrModule()->GenIRCode_BinaryOperater(tempReg, tempReg, right, InstructionOpCode::CVT);

	return tempReg;
}

Share<CompilerValue> Compiler::CreateNeg(Share<CompilerValue> assignTo, Share<CompilerValue> value)
{
	GetCurrModule()->CreateNeg(assignTo, value);
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

void Compiler::ReplaceConstantValueByStrongerType(Share<CompilerValue>& left, Share<CompilerValue>& right)
{
	if ((IsArrayType(left->GetValueType().PrimaryType) && IsNumberType(right->GetValueType().PrimaryType)))
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
				right = CreateMov(tempRegister, right, false);
			}
		}
		else
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
				left = CreateMov(tempRegister, left, false);
			}
		}
	}
	
}

void Compiler::CreateCompareJmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock)
{
	GetCurrModule()->GenIRCode_Cmp(cmpType, ifJmpBlock, elseJmpBlock);
}