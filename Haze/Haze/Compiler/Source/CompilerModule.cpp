#include "HazePch.h"

#include "Parse.h"
#include "HazeTokenText.h"
#include "HazeDebugDefine.h"
#include "HazeLogDefine.h"
#include "HazeFilePathHelper.h"

#include "Compiler.h"
#include "CompilerHelper.h"
#include "CompilerModule.h"
#include "CompilerValue.h"
#include "CompilerPointerFunction.h"
#include "CompilerClosureFunction.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerFunction.h"
#include "CompilerBlock.h"
#include "CompilerClass.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "CompilerElementValue.h"
#include "CompilerSymbol.h"

struct PushTempRegister
{
	PushTempRegister(HAZE_STRING_STREAM& hss, Compiler* compiler, CompilerModule* compilerModule, const HazeVariableType* defineType, Share<CompilerValue>* retValue)
		: Size(0), Hss(hss), Compiler(compiler), Module(compilerModule), DefineType(defineType), RetValue(retValue)
	{
		UseTempRegisters = Compiler->GetUseTempRegister();
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::PUSH) << " ";
			GenVariableHzic(Module, Hss, regi.second);
			Hss << HAZE_ENDL;
			Size += regi.second->GetSize();
		}
	}

	~PushTempRegister()
	{
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::POP) << " ";
			GenVariableHzic(Module, Hss, regi.second);
			Hss << HAZE_ENDL;
		}
		
		Compiler->GetInsertBlock()->PushIRCode(Hss.str());
		Compiler->ResetTempRegister(UseTempRegisters);

		if (DefineType && !IsVoidType(DefineType->BaseType) && RetValue)
		{
			*RetValue = Compiler->CreateFunctionRet(*DefineType);
		}
	}

	int GetSize() const { return Size; }
private:
	int Size;
	HAZE_STRING_STREAM& Hss;
	Compiler* Compiler;
	CompilerModule* Module;
	HashMap<const x_HChar*, Share<CompilerValue>> UseTempRegisters;
	const HazeVariableType* DefineType;
	Share<CompilerValue>* RetValue;
};


CompilerModule::CompilerModule(Compiler* compiler, const HString& moduleName, const HString& modulePath)
	: m_Compiler(compiler), m_ModuleLibraryType(HazeLibraryType::Normal), m_BeginCreateFunctionParamVariableIndex(-1),
	 m_IsGenTemplateCode(false), m_Path(modulePath)
{
#if HAZE_I_CODE_ENABLE

	auto functionName = GetHazeModuleGlobalDataInitFunctionName(moduleName);
	HazeVariableType functionType(HazeValueType::Void);
	V_Array<HazeDefineVariable> params;
	m_GlobalDataFunction = CreateFunction(functionName, functionType, params);
	m_GlobalDataFunction->m_DescType = InstructionFunctionType::HazeFunction;

	auto intermediateFilePath = GetIntermediateModuleFile(moduleName);
	bool newFS = true;

#if COMPILER_PARSE_INTER
	if (FileExist(intermediateFilePath))
	{
		HAZE_IFSTREAM fs(intermediateFilePath);
		fs.imbue(std::locale("chs"));
		x_uint64 lastTime = 1;
		fs >> lastTime;

		newFS = modulePath.empty() ? false : !(lastTime == GetFileLastTime(GetPath()));

		if (!newFS)
		{
			newFS = !ParseIntermediateFile(fs/*, moduleName*/);
		}
	}
#endif

	m_FS_I_Code = nullptr;
	if (newFS)
	{
		m_Compiler->GetCompilerSymbol()->GetTypeInfoMap()->RemoveModuleRefTypes(moduleName);

		m_FS_I_Code = new HAZE_OFSTREAM();
		m_FS_I_Code->imbue(std::locale("chs"));
		m_FS_I_Code->open(GetIntermediateModuleFile(moduleName));
		m_Compiler->MarkNewCode();
	}
	
#endif
}

CompilerModule::~CompilerModule()
{
	if (m_FS_I_Code && m_FS_I_Code->is_open())
	{
		m_FS_I_Code->close();
		delete m_FS_I_Code;
	}
}

bool CompilerModule::ParseIntermediateFile(HAZE_IFSTREAM& stream/*, const HString& moduleName*/)
{
	HString str;
	x_uint64 ui64;

	stream >> str;

	if (str != HAZE_VERSION)
	{
		return false;
	}

	stream >> str;
	if (str != GetPath())
	{
		return false;
	}
	
	stream >> *(x_uint32*)(&m_ModuleLibraryType);
	stream >> str;
	if (str != GetImportHeaderString())
	{
		HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到引用模块表的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
		return false;
	}
	stream >> ui64;
	for (int i = 0; i < ui64; i++)
	{
		stream >> str;
		CompilerModule* m;
		if (str == GetImportHeaderModuleString())
		{
			stream >> str;
			m = m_Compiler->GetModuleAndTryParseIntermediateFile(str);
			if (!m)
			{
				HAZE_LOG_ERR_W("<%s>解析临时文件失败，引入模块<%s>未能找到!\n", m_Path.c_str(), str.c_str());
				return false;
			}
			else if (m->NeedParse())
			{
				return false;
			}
		}
		else
		{
			HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到引用模块的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
			return false;
		}

		m_ImportModules.push_back(m);
	}

	stream >> str;
	if (str != GetGlobalDataHeaderString())
	{
		HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到全局数据表的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
		return false;
	}

	stream >> ui64;
	{
		if (ui64 > 0)
		{
			for (int i = 0; i < ui64; i++)
			{
				stream >> str;
				HazeVariableType::StringStreamFrom(stream);
			}
		}
	} 

	stream >> str;
	if (str != GetStringTableHeaderString())
	{
		HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到常量字符串表的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
		return false;
	}

	stream >> ui64;
	{
		if (ui64 > 0)
		{
			x_uint32 ui32;
			for (int i = 0; i < ui64; i++)
			{
				stream >> ui32;
				str.resize(ui32 + 1);
				stream >> str;
				stream.getline(str.data(), ui32 + 1);
			}
		}
	}

	stream >> str;
	if (str != GetEnumTableLabelHeader())
	{
		HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到枚举表的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
		return false;
	}

	stream >> ui64;
	{
		if (ui64 > 0)
		{
			HString enumValueStr;
			for (int i = 0; i < ui64; i++)
			{
				stream >> str;
				if (str == GetEnumStartHeader())
				{
					stream >> str;
					auto enumValue = CreateEnum(str);

					stream >> str;
					while (str != GetEnumEndHeader())
					{
						stream >> enumValueStr;
						HazeValue hazeValue;
						StringToHazeValueNumber(enumValueStr, ENUM_INT_TYPE, hazeValue);
						enumValue->AddEnumValue(str, m_Compiler->GenConstantValue(ENUM_INT_TYPE, hazeValue));

						stream >> str;
					}

					FinishCreateEnum();
				}
			}
		}
	}

	stream >> str;
	if (str != GetClassTableHeaderString())
	{
		HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到类表的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
		return false;
	}
	x_uint32 classCount = 0;
	stream >> classCount;
	for (x_uint32 i = 0; i < classCount; i++)
	{
		stream >> str;
		if (str != GetClassLabelHeader())
		{
			HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到类的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
			return false;
		}

		HString className;
		stream >> className;

		x_uint32 dataSize;
		stream >> dataSize;
		stream >> ui64;
		stream >> ui64;

		V_Array<HString> parentName(ui64);
		for (int j = 0; j < ui64; j++)
		{
			stream >> parentName[j];
		}

		x_uint32 memberCount;
		stream >> memberCount;

		V_Array<CompilerClass*> parentClass;
		for (int j = 0; j < parentName.size(); j++)
		{
			parentClass.push_back(GetClass(parentName[j]).get());
			memberCount -= parentClass.back()->GetMemberCount();
		}

		CompilerClass::ParseIntermediateClass(stream, this, parentClass);

		V_Array<Pair<HString, Share<CompilerValue>>> classData(memberCount);
		for (x_uint32 j = 0; j < memberCount; ++j)
		{
			HazeDataDesc desc;
			stream >> classData[j].first >> *((x_uint32*)(&desc));
			auto memberType = HazeVariableType::StringStreamFrom(stream);

			x_uint32 ui32;
			stream >> ui32 >> ui64;

			classData[j].second = m_Compiler->CreateClassVariable(this, memberType, nullptr, nullptr);
			classData[j].second->SetDataDesc(desc);
		}

		Share<CompilerClass> compilerClass = CreateClass(className, parentClass, classData);

		 //= CompilerClass::ParseIntermediateClass(stream, this, parentName);
		if (compilerClass)
		{
			m_HashMap_Classes[compilerClass->GetName()] = compilerClass;
		}
		else
		{
			HAZE_LOG_ERR_W("<%s>解析临时文件失败，未能找到第<%d>个类!\n", GetName().c_str(), i);
			return false;
		}
	}

	stream >> str;
	if (str != GetFucntionTableHeaderString())
	{
		HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到函数表的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
		return false;
	}
	stream >> ui64;// 包括类函数和普通函数
	stream >> ui64;// 闭包个数

	//全局数据初始化函数先不解析
	//m_GlobalDataFunction->GenI_Code(hss);


	V_Array<HazeDefineVariable> params;
	stream >> str;
	while (str == GetClassFunctionLabelHeader() || str == GetFunctionLabelHeader())
	{
		bool isNormalFunc = str == GetFunctionLabelHeader();
		HString name;
		int descType;
		stream >> descType;

		if (isNormalFunc)
		{
			stream >> name;
			auto type = HazeVariableType::StringStreamFrom(stream);

			params.clear();

			stream >> str;
			while (str == GetFunctionParamHeader())
			{
				params.resize(params.size() + 1);
				stream >> params.back().Name;
				params.back().Type = HazeVariableType::StringStreamFrom(stream);
				stream >> str;
			}

			while (str != GetFunctionEndHeader())
			{
				stream >> str;
			}

			stream >> ui64;

			if (name == m_GlobalDataFunction->GetName())
			{
				
			}
			else
			{
				CreateFunction(name, type, params);
			}
		}
		else
		{
			stream >> str;
			auto compilerClass = GetClass(str);

			x_uint32 functionAttrType;
			stream >> functionAttrType;

			stream >> name;
			auto type = HazeVariableType::StringStreamFrom(stream);

			params.clear();

			stream >> str;
			while (str == GetFunctionParamHeader())
			{
				params.resize(params.size() + 1);
				stream >> params.back().Name;
				params.back().Type = HazeVariableType::StringStreamFrom(stream);
				stream >> str;
			}

			while (str != GetFunctionEndHeader())
			{
				stream >> str;
			}

			stream >> ui64;
			CreateFunction(compilerClass, (HazeFunctionDesc)functionAttrType, NativeClassFunctionName(compilerClass->GetName(), name), type, params);
		}

		stream >> str;
		if (str == GetBlockFlowHeader())
		{
			x_uint32 flowCount;
			stream >> flowCount;
			for (x_uint32 i = 0; i < flowCount; i++)
			{
				stream >> ui64;

				stream >> ui64;
				x_uint32 number;
				for (x_uint64 j = 0; j < ui64; j++)
				{
					stream >> number;
				}

				stream >> ui64;
				for (x_uint64 j = 0; j < ui64; j++)
				{
					stream >> number;
				}
			}
		}
		else
		{
			HAZE_LOG_ERR_W("<%s>解析临时文件失败, 未能找到函数块控制流的头文本, <%s>!\n", m_Path.c_str(), str.c_str());
			return false;
		}

		stream >> str;
	}

	return true;
}

const HString& CompilerModule::GetName() const
{
	return *m_Compiler->GetModuleName(this);
}

void CompilerModule::MarkLibraryType(HazeLibraryType type)
{
	m_ModuleLibraryType = type;
}

void CompilerModule::RestartTemplateModule(const HString& moduleName)
{
#if HAZE_I_CODE_ENABLE
	
	if (m_FS_I_Code && !m_IsGenTemplateCode)
	{
		m_FS_I_Code->imbue(std::locale("chs"));
		m_FS_I_Code->open(GetIntermediateModuleFile(moduleName));
		m_IsGenTemplateCode = true;
	}

#endif
}

void CompilerModule::FinishModule()
{
	m_CurrFunction.clear();
}

void CompilerModule::GenCodeFile()
{
#if HAZE_I_CODE_ENABLE

	//生成中间代码先不需要计算symbol table表中的偏移，等统一生成字节码时在进行替换，模板会重新打开文件流。
	if (m_FS_I_Code && m_FS_I_Code->is_open())
	{
		GenICode();
		m_FS_I_Code->close();
	}

#endif
}

Share<CompilerClass> CompilerModule::CreateClass(const HString& name, V_Array<CompilerClass*>& parentClass,
	V_Array<Pair<HString, Share<CompilerValue>>>& classData)
{
	auto compilerClass = GetClass(name);
	if (!compilerClass)
	{
		auto typeId = m_Compiler->GetCompilerSymbol()->GetSymbolTypeId(name);

		// 检验是否存在循环继承
		HazeVariableType tempType(HazeValueType::Class, typeId);
		for (auto& parent : parentClass)
		{
			if (parent->IsParentClass(tempType))
			{
				COMPILER_ERR_MODULE_W("类<%s>与类<%s>存在循环继承", m_Compiler, GetName().c_str(), name.c_str(), parent->GetName().c_str());
				return nullptr;
			}
		}

		compilerClass = MakeShare<CompilerClass>(this, name, parentClass, classData, typeId);
		m_HashMap_Classes[name] = compilerClass;

		//m_Compiler->GetCompilerSymbol()->ResolveSymbol_Class(name, compilerClass.get());

		//m_Compiler->OnCreateClass(compilerClass);
	}

	m_CurrClass = name;
	return compilerClass;
}

Share<CompilerEnum> CompilerModule::CreateEnum(const HString& name)
{
	Share<CompilerEnum> compilerEnum = GetEnum(this, name);
	if (!compilerEnum)
	{
		compilerEnum = MakeShare<CompilerEnum>(this, name, m_Compiler->GetCompilerSymbol()->GetSymbolTypeId(name));
		m_HashMap_Enums[name] = compilerEnum;
	
		m_CurrEnum = name;

		//m_Compiler->GetCompilerSymbol()->ResolveSymbol_Enum(name, compilerEnum.get());
	}

	return compilerEnum;
}

Share<CompilerEnum> CompilerModule::GetCurrEnum()
{
	auto iter = m_HashMap_Enums.find(m_CurrEnum);
	if (iter != m_HashMap_Enums.end())
	{
		return iter->second;
	}

	return nullptr;
}

void CompilerModule::FinishCreateEnum()
{
	m_CurrEnum.clear();
}

void CompilerModule::FinishCreateClass()
{
	m_CurrClass.clear();
}

Share<CompilerFunction> CompilerModule::GetCurrFunction()
{
	if (m_CurrFunction.empty())
	{
		return nullptr;
	}

	auto iter = m_HashMap_Functions.find(m_CurrFunction);
	if (iter == m_HashMap_Functions.end())
	{
		if (m_CurrClass.empty())
		{
			return nullptr;
		}
		else
		{
			return GetClass(m_CurrClass)->FindFunction(m_CurrFunction, nullptr);
		}
	}

	return iter->second;
}

Share<CompilerFunction> CompilerModule::GetCurrClosure()
{
	return m_ClosureStack.back();
}

Share<CompilerFunction> CompilerModule::GetCurrClosureOrFunction()
{
	return m_ClosureStack.size() > 0 ? GetCurrClosure() : GetCurrFunction();
}

Share<CompilerFunction> CompilerModule::GetUpOneLevelClosureOrFunction()
{
	return m_ClosureStack.size() > 1 ? m_ClosureStack[m_ClosureStack.size() - 2] : GetCurrFunction();
}

Share<CompilerFunction> CompilerModule::GetFunction(const HString& name)
{
	SearchContext context;
	return GetFunction_Internal(name, context);
}

bool CompilerModule::IsImportModule(CompilerModule* m) const
{
	for (auto iter : m_ImportModules)
	{
		if (iter == m)
		{
			return true;
		}
	}
	return false;
}

CompilerModule* CompilerModule::ExistGlobalValue(const HString& name)
{
	SearchContext context;
	return ExistGlobalValue_Internal(name, context);
}

Share<CompilerEnum> CompilerModule::GetEnum(CompilerModule* m, const HString& name)
{
	SearchContext context;
	auto ret = m->GetEnum_Internal(name, context);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleEnum(name);
}

Share<CompilerEnum> CompilerModule::GetEnum(CompilerModule* m, x_uint32 typeId)
{
	return m->GetEnum(m, *m->m_Compiler->GetCompilerSymbol()->GetSymbolByTypeId(typeId));
}

Share<CompilerFunction> CompilerModule::CreateFunction(const HString& name, const HazeVariableType& type, V_Array<HazeDefineVariable>& params)
{
	Share<CompilerFunction> function = nullptr;
	auto it = m_HashMap_Functions.find(name);
	if (it == m_HashMap_Functions.end())
	{
		m_HashMap_Functions[name] = MakeShare<CompilerFunction>(this, name, type, params, HazeFunctionDesc::Normal);
		//m_HashMap_Functions[name]->InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, m_HashMap_Functions[name], nullptr));

		function = m_HashMap_Functions[name];
	}
	else
	{
		function = it->second;
	}

	m_CurrFunction = name;
	return function;
}

Share<CompilerFunction> CompilerModule::CreateFunction(Share<CompilerClass> compilerClass, HazeFunctionDesc desc,
	const HString& name, const HazeVariableType& type, V_Array<HazeDefineVariable>& params)
{
	Share<CompilerFunction> function = compilerClass->FindFunction(name, &compilerClass->GetName());
	if (!function)
	{
		function = MakeShare<CompilerFunction>(this, name, type, params, desc, compilerClass.get());
		compilerClass->AddFunction(function);

		//function->InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, function, nullptr));
	}

	m_CurrFunction = name;
	return function;
}

Share<CompilerClosureFunction> CompilerModule::CreateClosureFunction(HazeVariableType& type, V_Array<HazeDefineVariable>& params)
{
	HString name = CLOSURE_NAME_PREFIX + GetName() + ToHazeString(m_ClosureStack.size());
	auto closure = MakeShare<CompilerClosureFunction>(this, name, type, params);
	//closure->InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, closure, nullptr));

	m_Closures.push_back(closure);
	m_ClosureStack.push_back(closure);

	closure->SetUpLevelEntry(m_Compiler->GetInsertBlock());
	return closure;
}

void CompilerModule::BeginGlobalDataDefine()
{
	m_CurrFunction = m_GlobalDataFunction->GetName();
	m_Compiler->SetInsertBlock(m_GlobalDataFunction->GetEntryBlock());
}

void CompilerModule::EndGlobalDataDefine()
{
	FinishFunction();
}

void CompilerModule::FinishFunction()
{
	GetCurrFunction()->FunctionFinish();
	m_CurrFunction.clear();
	m_Compiler->ClearBlockPoint();
}

void CompilerModule::FinishClosure()
{
	m_ClosureStack.back()->FunctionFinish();
	m_Compiler->SetInsertBlock(m_ClosureStack.back()->GetUpLevelBlock());
	m_ClosureStack.pop_back();
}

void CompilerModule::CreateAdd(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::ADD);
}

void CompilerModule::CreateSub(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::SUB);
}

void CompilerModule::CreateMul(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::MUL);
}

void CompilerModule::CreateDiv(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::DIV);
}

void CompilerModule::CreateMod(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::MOD);
}

void CompilerModule::CreateBitAnd(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::BIT_AND);
}

void CompilerModule::CreateBitOr(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::BIT_OR);
}

void CompilerModule::CreateBitXor(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::BIT_XOR);
}

void CompilerModule::CreateShl(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::SHL);
}

void CompilerModule::CreateShr(Share<CompilerValue> assignTo, Share<CompilerValue> left, Share<CompilerValue> right)
{
	GenIRCode_BinaryOperater(assignTo, left, right, InstructionOpCode::SHR);
}

void CompilerModule::CreateBitNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1)
{
	GenIRCode_UnaryOperator(assignTo, oper1, InstructionOpCode::BIT_NEG);
}

void CompilerModule::CreateNeg(Share<CompilerValue> assignTo, Share<CompilerValue> oper1)
{
	GenIRCode_UnaryOperator(assignTo, oper1, InstructionOpCode::NEG);
}

void CompilerModule::CreateNot(Share<CompilerValue> assignTo, Share<CompilerValue> oper1)
{
	GenIRCode_UnaryOperator(assignTo, oper1, InstructionOpCode::NOT);
}

Share<CompilerValue> CompilerModule::CreateInc(Share<CompilerValue> value, bool isPreInc)
{
	Share<CompilerValue> retValue = value;
	if (IsHazeBaseType(value->GetVariableType().BaseType))
	{
		if (!isPreInc)
		{
			retValue = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetVariableType()), value);
		}
		//Compiler->CreateMov(Value, CreateAdd(Value, Compiler->GetConstantValueInt(1)));
		m_Compiler->CreateAdd(value, value, m_Compiler->GetConstantValueInt(1));
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Inc操作\n", GetHazeValueTypeString(value->GetVariableType().BaseType));
	}
	return retValue;
}

Share<CompilerValue> CompilerModule::CreateDec(Share<CompilerValue> value, bool isPreDec)
{
	Share<CompilerValue> retValue = value;
	if (IsHazeBaseType(value->GetVariableType().BaseType))
	{
		if (!isPreDec)
		{
			retValue = m_Compiler->GetTempRegister(value->GetVariableType());
			m_Compiler->CreateMov(retValue, value);
		}
		//Compiler->CreateMov(Value, CreateSub(Value, Compiler->GetConstantValueInt(1)));
		m_Compiler->CreateSub(value, value, m_Compiler->GetConstantValueInt(1));
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Dec操作\n", GetHazeValueTypeString(value->GetVariableType().BaseType));
	}
	return retValue;
}

Share<CompilerValue> CompilerModule::CreateNew(const HazeVariableType& data, V_Array<Share<CompilerValue>>* countValue, Share<CompilerFunction> closure)
{
	HAZE_STRING_STREAM hss;

	//if (defineTypes)
	//{
	//	GenIRCode_NewSign(hss, defineTypes);
	//}

	if (countValue)
	{
		for (x_uint64 i = 0; i < countValue->size(); i++)
		{
			GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, countValue->at(i));
		}
	}

	auto tempRegister = m_Compiler->GetTempRegister(data);
	
	if (closure)
	{
		m_Compiler->CreatePointerToFunction(closure, tempRegister);
	}

	GenIRCode(hss, this, InstructionOpCode::NEW, tempRegister, m_Compiler->GetConstantValueUint64(countValue ? countValue->size() : 0), nullptr, &data);

	if (countValue)
	{
		for (x_uint64 i = 0; i < countValue->size(); i++)
		{
			GenIRCode(hss, this, InstructionOpCode::POP, nullptr, countValue->at(i));
		}
	}

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());

	return tempRegister;
}

void CompilerModule::GenIRCode_BinaryOperater(Share<CompilerValue> assignTo, Share<CompilerValue> oper1, Share<CompilerValue> oper2,
	InstructionOpCode opCode, bool check)
{
	//Share<HazeCompilerValue> retValue = left;

	//优化相关
	//if (oper1->IsConstant() && oper2->IsConstant())
	//{
	//	if (IsNumberType(oper1->GetVariableType().PrimaryType))
	//	{
	//		auto& leftValue = const_cast<HazeValue&>(oper1->GetValue());
	//		HazeValue tempValue = leftValue;
	//		auto& rightValue = const_cast<HazeValue&>(oper2->GetValue());
	//		CalculateValueByType(oper1->GetVariableType().PrimaryType, opCode, &rightValue, &leftValue);

	//		auto retValue = m_Compiler->GenConstantValue(oper1->GetVariableType().PrimaryType, leftValue);
	//		leftValue = tempValue;
	//	}
	//	else
	//	{
	//		COMPILER_ERR_MODULE_W("生成<%s>操作错误", GetInstructionString(opCode), GetName().c_str());
	//	}
	//	return;// retValue;
	//}

	HAZE_STRING_STREAM ss;
	
	GenIRCode(ss, this, opCode, assignTo, oper1, oper2, nullptr, check);
	m_Compiler->GetInsertBlock()->PushIRCode(ss.str());
}

void CompilerModule::GenIRCode_UnaryOperator(Share<CompilerValue> assignTo, Share<CompilerValue> value, InstructionOpCode opCode)
{
	auto function = GetCurrFunction();
	HAZE_STRING_STREAM hss;

	if (m_CurrFunction.empty())
	{
		COMPILER_ERR_MODULE_W("未能找到有效函数生成字节码", m_Compiler);
	}
	else
	{
		GenIRCode(hss, this, opCode, assignTo, value);
	}

	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_Ret(Share<CompilerValue> value)
{
	auto function = GetCurrFunction();
	HAZE_STRING_STREAM hss;

	GenIRCode(hss, this, InstructionOpCode::RET, nullptr, value);
	hss << HAZE_ENDL;
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_Cmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock)
{
	HAZE_STRING_STREAM hss;

	if (cmpType == HazeCmpType::None)
	{
		HAZE_LOG_ERR_W("比较失败,比较类型为空,当前函数<%s>!\n", GetCurrFunction()->GetName().c_str());
	}

	m_Compiler->GetInsertBlock()->AddSuccessor(ifJmpBlock, elseJmpBlock);

	GenIRCode(hss, this, GetInstructionOpCodeByCmpType(cmpType), ifJmpBlock, elseJmpBlock);
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_JmpTo(Share<CompilerBlock> block)
{
	HAZE_STRING_STREAM hss;
	GenIRCode(hss, this, InstructionOpCode::JMP, block);

	m_Compiler->GetInsertBlock()->AddSuccessor(block);
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

Share<CompilerValue> CompilerModule::CreateGlobalVariable(const HazeDefineVariable& var, int line, Share<CompilerValue> refValue)
{
	return m_GlobalDataFunction->CreateGlobalVariable(var, line, refValue);
}

Share<CompilerValue> CompilerModule::GetClosureVariable(const HString& name, bool addRef)
{
	auto block = m_Compiler->GetInsertBlock();
	for (int i = (int)m_ClosureStack.size() - 1; i >= 0; i--)
	{
		if (i < (int)m_ClosureStack.size() - 1)
		{
			block = m_ClosureStack[i + 1]->GetUpLevelBlock();
		}

		auto var = m_ClosureStack[i]->GetLocalVariable(name, block);
		if (var)
		{
			if (i < m_ClosureStack.size() - 1 && addRef)
			{
				int refIndex = m_ClosureStack[i]->FindLocalVariableIndex(var);
				for (int j = i + 1; j < m_ClosureStack.size() - 1; j++)
				{
					if (refIndex >= 0)
					{
						refIndex = m_ClosureStack[j]->AddRefValue(refIndex, var, name);
					}
				}
			}

			return var;
		}
	}

	return nullptr;
}

void CompilerModule::ClosureAddLocalRef(Share<CompilerValue> value, const HString& name)
{
	auto index = GetCurrFunction()->FindLocalVariableIndex(value);
	for (x_uint64 i = 0; i < m_ClosureStack.size(); i++)
	{
		m_ClosureStack[i]->AddRefValue(index, value, name);
	}
}

void CompilerModule::FunctionCall(HAZE_STRING_STREAM& hss, Share<CompilerFunction> callFunction, Share<CompilerValue> pointerFunction,
	AdvanceFunctionInfo* advancFunctionInfo, x_uint32& size, const V_Array<Share<CompilerValue>>& params,
	Share<CompilerValue> thisPointerTo)
{
	static HazeVariableType s_UInt64 = HazeVariableType(HazeValueType::UInt64);
	static HazeVariableType s_Float64 = HazeVariableType(HazeValueType::Float64);

	Share<CompilerBlock> insertBlock = m_Compiler->GetInsertBlock();
	HString strName;

	auto pointerFunc = DynamicCast<CompilerPointerFunction>(pointerFunction);
	x_uint64 pushDefaultParamCount = 0;

	V_Array<HazeVariableType> funcTypes(params.size());
	if (!callFunction && !pointerFunc && !advancFunctionInfo)
	{
		COMPILER_ERR_MODULE_W("生成函数调用错误, <%s>为空", m_Compiler, GetName().c_str(), callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"));
	}
	else
	{
		x_uint64 paramSize = callFunction ? callFunction->GetParamCount() : pointerFunc ? pointerFunc->GetParamCount() : advancFunctionInfo->Params.size();

		for (x_int64 i = params.size() - 1; i >= 0; i--)
		{
			auto variable = params[i];
			auto index = params.size() - 1 - i;
			auto type = callFunction ? callFunction->GetParamTypeLeftToRightByIndex(index) :
				pointerFunc ? pointerFunc->GetParamTypeLeftToRightByIndex(index) :
				advancFunctionInfo->Params.size() > index ? advancFunctionInfo->Params.at(index) : advancFunctionInfo->Params.at(advancFunctionInfo->Params.size() - 1);


			if (type != variable->GetVariableType() && !variable->GetVariableType().IsStrongerType(type))
			{
				if (i == (x_int64)params.size() - 1 && !IsMultiVariableType(type.BaseType) && paramSize != params.size())
				{
					COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 应填入<%d>个参数，实际填入了<%d>个", m_Compiler, GetName().c_str(),
						callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"), paramSize, params.size());
				}
				else if (IsMultiVariableType(type.BaseType))
				{
					if (params.size() - i < paramSize)
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误,  第<%d>个参数枚举类型不匹配", m_Compiler, GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"), params.size() - 1 - i);
					}
				}
				else if (variable->IsEnum())
				{
					auto enumValue = DynamicCast<CompilerEnumValue>(variable);
					if (enumValue && enumValue->GetEnum() && enumValue->GetEnum()->GetTypeId() == type.TypeId) {}
					else
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数枚举类型不匹配", m_Compiler, GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"), params.size() - 1 - i);
					}
				}
				else if (type.IsStrongerType(variable->GetVariableType())) {}
				else if (IsRefrenceType(type.BaseType) && variable->GetVariableType().TypeId == type.TypeId) {}
				else if (variable->IsClass() && IsClassType(type.BaseType))
				{
					auto hazeClass = variable->IsElement() ? DynamicCast<CompilerElementValue>(variable)->GetRealClass() : DynamicCast<CompilerClassValue>(variable)->GetOwnerClass();
					if (hazeClass && hazeClass->IsParentClass(type)) {}
					else
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数类不匹配, 参数应为<%s>类, 实际为<%s>", m_Compiler, GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"), params.size() - 1 - i, m_Compiler->GetCompilerSymbol()->GetSymbolByTypeId(type.TypeId)->c_str(),
							hazeClass->GetName().c_str());
					}
				}
				else if (!IsMultiVariableType(type.BaseType))
				{
					COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数类型不匹配", m_Compiler, GetName().c_str(),
						callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"),
						params.size() - 1 - i);
				}
			}

			if (IsMultiVariableType(type.BaseType))
			{
				if (variable->IsRefrence())
				{
					if (IsIntegerType(HAZE_ID_2_TYPE(variable->GetVariableType().TypeId)))
					{
						funcTypes[i] = s_UInt64;
					}
					else if (IsFloatingType(HAZE_ID_2_TYPE(variable->GetVariableType().TypeId)))
					{
						funcTypes[i] = s_Float64;
					}
					else
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数引用类型不匹配", m_Compiler, GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"),
							params.size() - 1 - i);
					}
				}
				else
				{
					funcTypes[i] = variable->GetVariableType();
				}
			}
			else
			{
				funcTypes[i] = type;
			}
		}

		if (paramSize > 0)
		{
			auto& lastParam = callFunction ? callFunction->GetParamTypeByIndex(0) :
				pointerFunc ? pointerFunc->GetParamTypeByIndex(0) : advancFunctionInfo->Params.at(advancFunctionInfo->Params.size() - 1);
			if ((IsMultiVariableType(lastParam.BaseType) && params.size() + 1 >= paramSize)) {}
			else if (!IsMultiVariableType(lastParam.BaseType) && (params.size() + (callFunction ? callFunction->GetDefaultParamCount() : 0) >= paramSize))
			{
				if (params.size() != paramSize && callFunction)
				{
					pushDefaultParamCount = paramSize - params.size();
				}
			}
			else
			{
				COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 函数个数不匹配", m_Compiler, GetName().c_str(),
					callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"));
			}
		}
	}

	if (pushDefaultParamCount > 0)
	{
		callFunction->PushDefaultParam(pushDefaultParamCount);
	}

	for (x_uint64 i = 0; i < params.size(); i++)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, params[i], nullptr, &funcTypes[i]);
		insertBlock->PushIRCode(hss.str());

		size += funcTypes[i].GetTypeSize(); //funcTypes[i] ? funcTypes[i]->GetCompilerTypeSize() : GetSizeByCompilerValue(params[i]);
		hss.str(H_TEXT(""));
		strName.clear();
	}

	if (thisPointerTo)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, thisPointerTo);
		insertBlock->PushIRCode(hss.str());

		hss.str(H_TEXT(""));

		size += thisPointerTo->GetVariableType().GetTypeSize();
	}

	HazeVariableType retPcType(HAZE_CALL_PUSH_ADDRESS_TYPE);
	hss << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CALL_PUSH_ADDRESS_NAME << " " << INT_VAR_SCOPE(HazeVariableScope::None)
		<< " " << (x_uint32)HazeDataDesc::Address << " ";
	retPcType.StringStreamTo(hss);
	hss << HAZE_ENDL;
	
	insertBlock->PushIRCode(hss.str());

	hss.str(H_TEXT(""));
}

Share<CompilerValue> CompilerModule::CreateFunctionCall(Share<CompilerFunction> callFunction, 
	const V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo, const HString* nameSpace)
{
	HAZE_STRING_STREAM hss;
	x_uint32 size = 0;

	Share<CompilerValue> ret = nullptr;
	{
		PushTempRegister pushTempRegister(hss, m_Compiler, this, &callFunction->GetFunctionType(), &ret);
		FunctionCall(hss, callFunction, nullptr, nullptr, size, params, thisPointerTo);
		GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, callFunction, nullptr, nullptr, -1, nameSpace);
	}

	return ret;
}

Share<CompilerValue> CompilerModule::CreateFunctionCall(Share<CompilerValue> pointerFunction, 
	V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo)
{
	HAZE_STRING_STREAM hss;
	x_uint32 size = 0;

	HString varName;
	GetGlobalVariableName(this, pointerFunction, varName);
	if (varName.empty())
	{
		GetCurrFunction()->FindLocalVariableName(pointerFunction, varName);
		if (varName.empty())
		{
			HAZE_LOG_ERR_W("函数指针调用失败!\n");
			return nullptr;
		}
	}

	Share<CompilerValue> ret = nullptr;
	{
		PushTempRegister pushTempRegister(hss, m_Compiler, this, &DynamicCast<CompilerPointerFunction>(pointerFunction)->GetFunctionType(), &ret);
		FunctionCall(hss, nullptr, pointerFunction, nullptr, size, params, thisPointerTo);
		GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, nullptr, pointerFunction);
	}

	return ret;
}

Share<CompilerValue> CompilerModule::CreateAdvanceTypeFunctionCall(AdvanceFunctionInfo* functionInfo, x_uint16 index, const V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo, HazeVariableType* expectType)
{
	HAZE_STRING_STREAM hss;
	x_uint32 size = 0;

	HString varName;
	GetGlobalVariableName(this, thisPointerTo, varName);
	if (varName.empty())
	{
		GetCurrFunction()->FindLocalVariableName(thisPointerTo, varName);
		if (varName.empty())
		{
			HAZE_LOG_ERR_W("函数指针调用失败!\n");
			return nullptr;
		}
	}

	HazeVariableType funcType = functionInfo->Type;
	if (thisPointerTo->IsObjectBase())
	{
		static AdvanceFunctionInfo* s_objectBaseGetFunction = nullptr;
		if (!s_objectBaseGetFunction)
		{
			s_objectBaseGetFunction = m_Compiler->GetAdvanceFunctionInfo(HazeValueType::ObjectBase, HAZE_ADVANCE_GET_FUNCTION);
		}

		if (functionInfo == s_objectBaseGetFunction)
		{
			funcType = HAZE_VAR_TYPE((HazeValueType)m_Compiler->GetCompilerSymbol()->GetTypeInfoMap()->GetTypeById(thisPointerTo->GetVariableType().TypeId)->_ObjectBase.TypeId1);
		}
	}

	Share<CompilerValue> ret = nullptr;
	{
		PushTempRegister pushTempRegister(hss, m_Compiler, this, &funcType, &ret);
		FunctionCall(hss, nullptr, nullptr, functionInfo, size, params, thisPointerTo);
		GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, nullptr, nullptr, thisPointerTo, index);
	}

	if (!ret && expectType)
	{
		if (!IsVoidType(expectType->BaseType))
		{
			ret = m_Compiler->GetRetRegister(expectType->BaseType, expectType->TypeId);
		}
		else
		{
			HAZE_LOG_ERR_W("复杂类型函数指针调用失败, 未能输入非空的期望类型!\n");
		}
	}
	
	return ret;
}

Share<CompilerValue> CompilerModule::GetOrCreateGlobalStringVariable(const HString& str)
{
	auto it = m_HashMap_StringTable.find(str);
	if (it != m_HashMap_StringTable.end())
	{
		return it->second;
	}
	m_HashMap_StringTable[str] = nullptr;

	it = m_HashMap_StringTable.find(str);

	m_HashMap_StringMapping[(int)m_HashMap_StringMapping.size()] = &it->first;

	HazeVariableType defineVarType = HAZE_VAR_TYPE(HazeValueType::String);
	it->second = CreateVariable(this, defineVarType, HazeVariableScope::Global, HazeDataDesc::ConstantString, 0);

	HazeValue& hazeValue = const_cast<HazeValue&>(it->second->GetValue());
	hazeValue.Value.Extra.StringTableIndex = (int)m_HashMap_StringMapping.size() - 1;

	return it->second;
}

x_uint32 CompilerModule::GetGlobalStringIndex(Share<CompilerValue> value)
{
	for (auto& it : m_HashMap_StringTable)
	{
		if (value == it.second)
		{
			for (auto& It : m_HashMap_StringMapping)
			{
				if (&it.first == It.second)
				{
					return It.first;
				}
			}
		}
	}

	return 0;
}

Share<CompilerValue> CompilerModule::GetGlobalVariable(CompilerModule* m, const HString& name)
{
	SearchContext context;
	auto ret = m->GetGlobalVariable_Internal(name, context);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleGlobalVariable(name);
}

bool CompilerModule::GetGlobalVariableName(CompilerModule* m, const Share<CompilerValue>& value, HString& outName, bool getOffset,
	V_Array<Pair<x_uint64, CompilerValue*>>* offsets)
{
	SearchContext context;
	if (m->GetGlobalVariableName_Internal(value, outName, getOffset, offsets, context))
	{
		return true;
	}

	if (m->m_Compiler->GetBaseModuleGlobalVariableName(value, outName))
	{
		return true;
	}

	return value->TryGetVariableName(outName);
}

bool CompilerModule::GetClosureVariableName(CompilerModule* m, const Share<CompilerValue>& value, HString& outName)
{
	for (int i = (int)m->m_ClosureStack.size() - 1; i >= 0; i--)
	{
		if (m->m_ClosureStack[i]->FindLocalVariableName(value, outName))
		{
			return true;
		}
	}

	return false;
}

Share<CompilerFunction> CompilerModule::GetFunction_Internal(const HString& name, SearchContext& context)
{
	auto it = m_HashMap_Functions.find(name);
	if (it != m_HashMap_Functions.end())
	{
		return it->second;
	}
	else if (!m_CurrClass.empty())
	{
		auto iter = m_HashMap_Classes.find(m_CurrClass);
		if (iter != m_HashMap_Classes.end())
		{
			auto func = iter->second->FindFunction(name, nullptr);
			if (func)
			{
				return func;
			}

		}
	}

	if (context.CurrentDepth >= context.MaxDepth)
	{
		HAZE_LOG_ERR_W("查找类<%s>: 递归深度过深，可能存在循环依赖\n", name.c_str());
		return nullptr;
	}

	if (context.VisitedModules.find(this) != context.VisitedModules.end())
	{
		//HAZE_LOG_ERR_W("GetClass: 检测到循环依赖，模块<%s>已被访问\n", GetName().c_str());
		return nullptr;
	}

	context.VisitedModules.insert(this);
	context.CurrentDepth++;

	for (auto& iter : m_ImportModules)
	{
		auto func = iter->GetFunction_Internal(name, context);
		if (func)
		{
			return func;
		}
	}

	context.CurrentDepth--;

	return nullptr;
}

CompilerModule* CompilerModule::ExistGlobalValue_Internal(const HString& name, SearchContext& context)
{
	auto variables = m_GlobalDataFunction->GetEntryBlock()->GetAllocaList();
	for (auto& it : variables)
	{
		if (it.first == name)
		{
			return this;
		}
	}

	if (context.CurrentDepth >= context.MaxDepth)
	{
		HAZE_LOG_ERR_W("查找类<%s>: 递归深度过深，可能存在循环依赖\n", name.c_str());
		return nullptr;
	}

	if (context.VisitedModules.find(this) != context.VisitedModules.end())
	{
		//HAZE_LOG_ERR_W("GetClass: 检测到循环依赖，模块<%s>已被访问\n", GetName().c_str());
		return nullptr;
	}

	context.VisitedModules.insert(this);
	context.CurrentDepth++;

	for (auto& it : m_ImportModules)
	{
		auto m = it->ExistGlobalValue_Internal(name, context);
		if (m)
		{
			return m;
		}
	}

	context.CurrentDepth--;
	return nullptr;
}

Share<CompilerValue> CompilerModule::GetGlobalVariable_Internal(const HString& name, SearchContext& context)
{
	auto variables = m_GlobalDataFunction->GetEntryBlock()->GetAllocaList();
	for (auto& it : variables)
	{
		if (it.first == name)
		{
			return it.second;
		}
	}

	if (context.CurrentDepth >= context.MaxDepth)
	{
		HAZE_LOG_ERR_W("查找全局变量<%s>: 递归深度过深，可能存在循环依赖\n", name.c_str());
		return nullptr;
	}

	if (context.VisitedModules.find(this) != context.VisitedModules.end())
	{
		//HAZE_LOG_ERR_W("GetClass: 检测到循环依赖，模块<%s>已被访问\n", GetName().c_str());
		return nullptr;
	}

	context.VisitedModules.insert(this);
	context.CurrentDepth++;

	for (auto& m : m_ImportModules)
	{
		auto ret = m->GetGlobalVariable_Internal(name, context);
		if (ret)
		{
			return ret;
		}
	}

	context.CurrentDepth--;
	return nullptr;
}

bool CompilerModule::GetGlobalVariableName_Internal(const Share<CompilerValue>& value, HString& outName, bool getOffset,
	V_Array<Pair<x_uint64, CompilerValue*>>* offsets, SearchContext& context)
{
	if (value->IsRegister())
	{
		outName = m_Compiler->GetRegisterName(value);
		return true;
	}

	auto variables = m_GlobalDataFunction->GetEntryBlock()->GetAllocaList();
	for (auto& it : variables)
	{
		if (it.second == value)
		{
			outName = it.first;
			return true;
		}
	}

	if (context.CurrentDepth >= context.MaxDepth)
	{
		HAZE_LOG_ERR_W("查找全局变量名: 递归深度过深，可能存在循环依赖\n");
		return false;
	}

	if (context.VisitedModules.find(this) != context.VisitedModules.end())
	{
		//HAZE_LOG_ERR_W("GetClass: 检测到循环依赖，模块<%s>已被访问\n", GetName().c_str());
		return false;
	}

	context.VisitedModules.insert(this);
	context.CurrentDepth++;

	for (auto& it : m_ImportModules)
	{
		if (it->GetGlobalVariableName_Internal(value, outName, getOffset, offsets, context))
		{
			return true;
		}
	}

	for (auto& it : m_HashMap_StringTable)
	{
		if (it.second == value)
		{
			outName = HAZE_CONSTANT_STRING_NAME;//It.first;
			return true;
		}
	}

	context.CurrentDepth--;
	return false;
}

Share<CompilerClass> CompilerModule::GetClass_Internal(const HString& className, SearchContext& context)
{
	auto iter = m_HashMap_Classes.find(className);
	if (iter != m_HashMap_Classes.end())
	{
		return iter->second;
	}

	if (context.CurrentDepth >= context.MaxDepth)
	{
		HAZE_LOG_ERR_W("查找类<%s>: 递归深度过深，可能存在循环依赖\n", className.c_str());
		return nullptr;
	}

	if (context.VisitedModules.find(this) != context.VisitedModules.end())
	{
		//HAZE_LOG_ERR_W("GetClass: 检测到循环依赖，模块<%s>已被访问\n", GetName().c_str());
		return nullptr;
	}

	context.VisitedModules.insert(this);
	context.CurrentDepth++;

	for (auto& it : m_ImportModules)
	{
		auto ret = it->GetClass_Internal(className, context);
		if (ret)
		{
			return ret;
		}
	}

	context.CurrentDepth--;
	return nullptr;
}

Share<CompilerEnum> CompilerModule::GetEnum_Internal(const HString& name, SearchContext& context)
{
	auto iter = m_HashMap_Enums.find(name);
	if (iter != m_HashMap_Enums.end())
	{
		return iter->second;
	}

	if (context.CurrentDepth >= context.MaxDepth)
	{
		HAZE_LOG_ERR_W("查找枚举<%s>: 递归深度过深，可能存在循环依赖\n", name.c_str());
		return nullptr;
	}

	if (context.VisitedModules.find(this) != context.VisitedModules.end())
	{
		//HAZE_LOG_ERR_W("GetClass: 检测到循环依赖，模块<%s>已被访问\n", GetName().c_str());
		return nullptr;
	}

	context.VisitedModules.insert(this);
	context.CurrentDepth++;

	for (auto& it : m_ImportModules)
	{
		auto ret = it->GetEnum_Internal(name, context);
		if (ret)
		{
			return ret;
		}
	}

	context.CurrentDepth--;
	return nullptr;
}

Share<CompilerClass> CompilerModule::GetClass(const HString& className)
{
	SearchContext context;
	return GetClass_Internal(className, context);
}

x_uint32 CompilerModule::GetClassSize(const HString& className)
{
	return GetClass(className)->GetDataSize();
}

void CompilerModule::GenICode()
{
	HAZE_STRING_STREAM hss;
	hss << GetFileLastTime(m_Path) << HAZE_ENDL;
	hss << HAZE_VERSION << HAZE_ENDL;

	//版本 2个字节
	//FS_Ass << "1 1" << HAZE_ENDL;

	//堆栈 4个字节
	//FS_Ass << 1024 << HAZE_ENDL;

	//库类型
	if (m_Path.empty())
	{
		hss << GetName() << HAZE_ENDL;
	}
	else
	{
		hss << m_Path << HAZE_ENDL;
	}

	hss << (x_uint32)m_ModuleLibraryType << HAZE_ENDL;
	
	hss << GetImportHeaderString() << HAZE_ENDL;
	hss << m_ImportModules.size() << HAZE_ENDL;
	for (int i = 0; i < m_ImportModules.size(); i++)
	{
		if (!m_ImportModules[i]->GetPath().empty())
		{
			hss << GetImportHeaderModuleString() << " " << m_ImportModules[i]->GetPath() << HAZE_ENDL;
		}
		else
		{
			HAZE_LOG_ERR_W("生成引用表失败, <%s>模块的引用路径为空!", m_ImportModules[i]->GetName().c_str());
			return;
		}
	}

	/*
	*	全局数据 ：	个数
	*				数据名 数据类型 数据
	*/
	hss << GetGlobalDataHeaderString() << HAZE_ENDL;

	auto globaleVariables = m_GlobalDataFunction->GetEntryBlock()->GetAllocaList();
	hss << globaleVariables.size() << HAZE_ENDL;

	for (int i = 0; i < globaleVariables.size(); i++)
	{
		auto& var = globaleVariables[i].second;
	
		hss << globaleVariables[i].first << " ";
		var->GetVariableType().StringStreamTo(hss);
		hss << HAZE_ENDL;
	}

	//全局变量初始化
	/*hss << GetGlobalDataInitBlockStart() << HAZE_ENDL;
	for (size_t i = 0; i < m_ModuleIRCodes.size(); i++)
	{
		hss << m_ModuleIRCodes[i].c_str();
	}
	hss << GetGlobalDataInitBlockEnd() << HAZE_ENDL;*/
	

	/*
	*	字符串表 ：	个数
	*				字符串长度 字符串
	*/
	if (m_HashMap_StringMapping.size() != m_HashMap_StringTable.size())
	{
		HAZE_LOG_ERR_W("生成字符串表失败!",
			m_HashMap_StringMapping.size(), m_HashMap_StringTable.size());
		return;
	}
	hss << GetStringTableHeaderString() << HAZE_ENDL;
	hss << m_HashMap_StringMapping.size() << HAZE_ENDL;

	for (auto& it : m_HashMap_StringMapping)
	{
		hss << it.second->length() << " " << *it.second << HAZE_ENDL;
	}

	/*
	*	枚举表
	*/

	hss << GetEnumTableLabelHeader() << HAZE_ENDL;
	hss << m_HashMap_Enums.size() << HAZE_ENDL;;
	for (auto& iter : m_HashMap_Enums)
	{
		iter.second->GenEnum_I_Code(hss);
	}

	/*
	*	类表 ：	个数
	*				名称 指令流
	*
	*/
	x_uint64 functionSize = 0;

	hss << GetClassTableHeaderString() << HAZE_ENDL;
	hss << m_HashMap_Classes.size() << HAZE_ENDL;
	for (auto& iter : m_HashMap_Classes)
	{
		iter.second->GenClassData_I_Code(hss);
		functionSize += iter.second->GetFunctionNum();
	}

	/*
	*	函数表 ：	个数
	*				名称 指令流
	*
	*/
	hss << GetFucntionTableHeaderString() << HAZE_ENDL;
	hss << m_HashMap_Functions.size() + functionSize << " " << m_Closures.size() << HAZE_ENDL;

	m_GlobalDataFunction->GenI_Code(hss);

	for (auto& iter : m_HashMap_Classes)
	{
		iter.second->GenClassFunction_I_Code(hss);
	}

	for (auto& iter : m_HashMap_Functions)
	{
		if (iter.second != m_GlobalDataFunction)
		{
			iter.second->GenI_Code(hss);
		}
	}

	for (auto& iter : m_Closures)
	{
		iter->GenI_Code(hss);
	}

	*m_FS_I_Code << hss.str();

	if (m_IsGenTemplateCode)
	{
		m_IsGenTemplateCode = false;
	}
}