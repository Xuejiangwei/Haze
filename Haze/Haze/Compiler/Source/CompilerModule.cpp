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
#include "HazeCompilerPointerValue.h"
#include "CompilerPointerFunction.h"
#include "CompilerClassValue.h"
#include "CompilerArrayValue.h"
#include "CompilerFunction.h"
#include "CompilerBlock.h"
#include "CompilerClass.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"

struct PushTempRegister
{
	PushTempRegister(HAZE_STRING_STREAM& hss, Compiler* compiler, CompilerModule* compilerModule, const HazeDefineType* defineType, Share<CompilerValue>* retValue)
		: Size(0), Hss(hss), Compiler(compiler), Module(compilerModule), DefineType(defineType), RetValue(retValue)
	{
		UseTempRegisters = Compiler->GetUseTempRegister();
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::PUSH) << " ";
			GenVariableHzic(Module, Hss, regi.second);
			Hss << std::endl;
			Size += regi.second->GetSize();
		}
	}

	~PushTempRegister()
	{
		for (auto& regi : UseTempRegisters)
		{
			Hss << GetInstructionString(InstructionOpCode::POP) << " ";
			GenVariableHzic(Module, Hss, regi.second);
			Hss << std::endl;
		}
		
		Compiler->GetInsertBlock()->PushIRCode(Hss.str());
		Compiler->ResetTempRegister(UseTempRegisters);

		if (DefineType && RetValue)
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
	const HazeDefineType* DefineType;
	Share<CompilerValue>* RetValue;
};


CompilerModule::CompilerModule(Compiler* compiler, const HString& moduleName, const HString& modulePath)
	: m_Compiler(compiler), m_ModuleLibraryType(HazeLibraryType::Normal), m_IsBeginCreateFunctionVariable(false),
	 m_IsGenTemplateCode(false), m_Path(modulePath)
{
#if HAZE_I_CODE_ENABLE

	auto functionName = GetHazeModuleGlobalDataInitFunctionName(moduleName);
	HazeDefineType functionType(HazeValueType::Void);
	V_Array<HazeDefineVariable> params;
	m_GlobalDataFunction = CreateFunction(functionName, functionType, params);
	m_GlobalDataFunction->m_DescType = InstructionFunctionType::HazeFunction;

	auto intermediateFilePath = GetIntermediateModuleFile(moduleName);
	bool newFS = true;
	if (FileExist(intermediateFilePath))
	{
		HAZE_IFSTREAM fs(intermediateFilePath);
		fs.imbue(std::locale("chs"));
		x_uint64 lastTime = 1;
		fs >> lastTime;

		newFS = modulePath.empty() ? false : !(lastTime == GetFileLastTime(GetPath()));

		if (!newFS)
		{
			newFS = !ParseIntermediateFile(fs, moduleName);
		}
	}

	m_FS_I_Code = nullptr;
	if (newFS)
	{
		m_FS_I_Code = new HAZE_OFSTREAM();
		m_FS_I_Code->imbue(std::locale("chs"));
		m_FS_I_Code->open(GetIntermediateModuleFile(moduleName));
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

bool CompilerModule::ParseIntermediateFile(HAZE_IFSTREAM& stream, const HString& moduleName)
{
	HString str;
	x_uint64 ui64;

	stream >> str;

	if (str != GetPath())
	{
		return false;
	}
	
	stream >> *(x_uint32*)(&m_ModuleLibraryType);
	stream >> str;
	if (str != GetImportHeaderString())
	{
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
			m = m_Compiler->GetModule(str);
			if (!m)
			{
				HAZE_LOG_ERR_W("<%s>解析临时文件失败，引入模块<%s>未能找到!\n", m_Path.c_str(), str.c_str());
				return false;
			}
		}
		else
		{
			return false;
		}

		m_ImportModules.push_back(m);
	}

	stream >> str;
	if (str != GetGlobalDataHeaderString())
	{
		return false;
	}

	stream >> ui64;
	{
		// 暂时先不处理
		if (ui64 > 0)
		{
			HAZE_LOG_ERR_W("<%s>解析临时文件失败，没有读取全局数据初始化操作!\n", GetName().c_str());
			return false;
		}
	}

	stream >> str;
	if (str != GetStringTableHeaderString())
	{
		return false;
	}

	stream >> ui64;
	{
		// 暂时先不处理
		if (ui64 > 0)
		{
			x_uint32 ui32;
			for (int i = 0; i < ui64; i++)
			{
				stream >> ui32 >> str;
			}
		}
	}

	stream >> str;
	if (str != GetClassTableHeaderString())
	{
		return false;
	}
	stream >> ui64;
	for (int i = 0; i < ui64; i++)
	{
		stream >> str;
		if (str != GetClassLabelHeader())
		{
			return false;
		}

		HString className;
		stream >> className;

		x_uint32 dataSize;
		stream >> dataSize;
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
		}

		CompilerClass::ParseIntermediateClass(stream, this, parentClass);

		V_Array<Pair<HString, Share<CompilerValue>>> classData(memberCount);
		for (x_uint32 j = 0; j < memberCount; ++j)
		{
			HazeDataDesc desc;
			stream >> classData[j].first >> *((x_uint32*)(&desc));
			auto memberType = HazeDefineType::StringStreamFrom<Compiler>(stream, m_Compiler, &Compiler::GetSymbolTableNameAddress);

			x_uint32 ui32;
			stream >> ui32 >> ui64;


			//暂时设置成员变量为公有的
			classData[i].second = CreateVariable(this, memberType, HazeVariableScope::None, desc, 0);
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
		return false;
	}
	stream >> ui64;// 包括类函数和普通函数

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
			auto type = HazeDefineType::StringStreamFrom<Compiler>(stream, m_Compiler, &Compiler::GetSymbolTableNameAddress);

			params.clear();

			stream >> str;
			while (str == GetFunctionParamHeader())
			{
				params.resize(params.size() + 1);
				stream >> params.back().Name;
				params.back().Type = HazeDefineType::StringStreamFrom<Compiler>(stream, m_Compiler, &Compiler::GetSymbolTableNameAddress);
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
			auto type = HazeDefineType::StringStreamFrom<Compiler>(stream, m_Compiler, &Compiler::GetSymbolTableNameAddress);

			params.clear();

			stream >> str;
			while (str == GetFunctionParamHeader())
			{
				params.resize(params.size() + 1);
				stream >> params.back().Name;
				params.back().Type = HazeDefineType::StringStreamFrom<Compiler>(stream, m_Compiler, &Compiler::GetSymbolTableNameAddress);
			}

			while (str != GetFunctionEndHeader())
			{
				stream >> str;
			}

			stream >> ui64;
			CreateFunction(compilerClass, (ClassCompilerFunctionType)functionAttrType, NativeClassFunctionName(compilerClass->GetName(), name), type, params);
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
		m_HashMap_Classes[name] = MakeShare<CompilerClass>(this, name, parentClass, classData);
		compilerClass = m_HashMap_Classes[name];

		m_Compiler->OnCreateClass(compilerClass);
	}

	m_CurrClass = name;
	return compilerClass;
}

Share<CompilerEnum> CompilerModule::CreateEnum(const HString& name, HazeValueType baseType)
{
	Share<CompilerEnum> compilerEnum = GetEnum(this, name);
	if (!compilerEnum)
	{
		compilerEnum = MakeShare<CompilerEnum>(this, name, baseType);
		m_HashMap_Enums[name] = compilerEnum;
	
		m_CurrEnum = name;
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

Share<CompilerFunction> CompilerModule::GetFunction(const HString& name)
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

	return nullptr;
}

void CompilerModule::StartCacheTemplate(HString& templateName, x_uint32 startLine, HString& templateText, V_Array<HString>& templateTypes)
{
	auto iter = m_HashMap_TemplateText.find(templateName);
	if (iter == m_HashMap_TemplateText.end())
	{
		m_HashMap_TemplateText.insert({ Move(templateName), { startLine, Move(templateText),
			Move(templateTypes) } });
	}
}

bool CompilerModule::IsTemplateClass(const HString& name)
{
	auto iter = m_HashMap_TemplateText.find(name);
	if (iter != m_HashMap_TemplateText.end())
	{
		return true;
	}

	for (auto& it : m_ImportModules)
	{
		if (it->IsTemplateClass(name))
		{
			return true;
		}
	}

	return false;
}

bool CompilerModule::ResetTemplateClassRealName(HString& inName, const V_Array<HazeDefineType>& templateTypes)
{
	for (auto& templateText : m_HashMap_TemplateText)
	{
		if (templateText.first == inName)
		{
			if (templateTypes.size() != templateText.second.Types.size())
			{
				HAZE_LOG_ERR_W("生成模板<%s>错误, 类型数应为%d, 实际为%d!\n", inName.c_str(), templateText.second.Types.size(), 
					templateTypes.size());
				return false;
			}

			//HString className = inName;
			//GetTemplateClassName(inName, templateTypes);

			for (auto& compilerClass : m_HashMap_Classes)
			{
				if (compilerClass.first == inName)
				{
					return true;
				}
			}

			Parse p(m_Compiler);
			p.InitializeString(templateText.second.Text, templateText.second.StartLine);
			p.ParseTemplateContent(GetName(), inName, templateText.second.Types, templateTypes);
			return true;
		}
	}

	for (auto& m : m_ImportModules)
	{
		if (m->ResetTemplateClassRealName(inName, templateTypes))
		{
			return true;
		}
	}

	return false;
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

Share<CompilerEnum> CompilerModule::GetEnum(CompilerModule* m, const HString& name)
{
	auto ret = m->GetEnum_Internal(name);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleEnum(name);
}

Share<CompilerFunction> CompilerModule::CreateFunction(const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params)
{
	Share<CompilerFunction> function = nullptr;
	auto it = m_HashMap_Functions.find(name);
	if (it == m_HashMap_Functions.end())
	{
		m_HashMap_Functions[name] = MakeShare<CompilerFunction>(this, name, type, params);
		m_HashMap_Functions[name]->InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, m_HashMap_Functions[name], nullptr));

		function = m_HashMap_Functions[name];
	}
	else
	{
		function = it->second;
	}

	m_CurrFunction = name;
	return function;
}

Share<CompilerFunction> CompilerModule::CreateFunction(Share<CompilerClass> compilerClass, ClassCompilerFunctionType classFunctionType,
	const HString& name, HazeDefineType& type, V_Array<HazeDefineVariable>& params)
{
	Share<CompilerFunction> function = compilerClass->FindFunction(name, &compilerClass->GetName());
	if (!function)
	{
		function = MakeShare<CompilerFunction>(this, name, type, params, compilerClass.get(),
			classFunctionType);
		compilerClass->AddFunction(function);

		function->InitEntryBlock(CompilerBlock::CreateBaseBlock(BLOCK_ENTRY_NAME, function, nullptr));
	}

	m_CurrFunction = name;
	return function;
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
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		if (!isPreInc)
		{
			retValue = m_Compiler->CreateMov(m_Compiler->GetTempRegister(value->GetValueType()), value);
		}
		//Compiler->CreateMov(Value, CreateAdd(Value, Compiler->GetConstantValueInt(1)));
		m_Compiler->CreateAdd(value, value, m_Compiler->GetConstantValueInt(1));
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Inc操作\n", GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

Share<CompilerValue> CompilerModule::CreateDec(Share<CompilerValue> value, bool isPreDec)
{
	Share<CompilerValue> retValue = value;
	if (IsHazeBaseType(value->GetValueType().PrimaryType))
	{
		if (!isPreDec)
		{
			retValue = m_Compiler->GetTempRegister(value->GetValueType());
			m_Compiler->CreateMov(retValue, value);
		}
		//Compiler->CreateMov(Value, CreateSub(Value, Compiler->GetConstantValueInt(1)));
		m_Compiler->CreateSub(value, value, m_Compiler->GetConstantValueInt(1));
	}
	else
	{
		HAZE_LOG_ERR_W("<%s>类型不能使用Dec操作\n", GetHazeValueTypeString(value->GetValueType().PrimaryType));
	}
	return retValue;
}

Share<CompilerValue> CompilerModule::CreateNew(Share<CompilerFunction> function, const HazeDefineType& data, V_Array<Share<CompilerValue>>* countValue)
{
	HAZE_STRING_STREAM hss;

	if (countValue)
	{
		for (x_uint64 i = 0; i < countValue->size(); i++)
		{
			GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, countValue->at(i));
		}
	}

	auto tempRegister = m_Compiler->GetTempRegister(data, countValue ? countValue->size() : 0);
	GenIRCode(hss, this, InstructionOpCode::NEW, tempRegister, m_Compiler->GetConstantValueUint64(countValue ? countValue->size() : 0),
		nullptr, &data);

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
	//	if (IsNumberType(oper1->GetValueType().PrimaryType))
	//	{
	//		auto& leftValue = const_cast<HazeValue&>(oper1->GetValue());
	//		HazeValue tempValue = leftValue;
	//		auto& rightValue = const_cast<HazeValue&>(oper2->GetValue());
	//		CalculateValueByType(oper1->GetValueType().PrimaryType, opCode, &rightValue, &leftValue);

	//		auto retValue = m_Compiler->GenConstantValue(oper1->GetValueType().PrimaryType, leftValue);
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
		HAZE_TO_DO(全局语句暂时不处理);
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
	hss << std::endl;
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_Cmp(HazeCmpType cmpType, Share<CompilerBlock> ifJmpBlock, Share<CompilerBlock> elseJmpBlock)
{
	HAZE_STRING_STREAM hss;

	if (cmpType == HazeCmpType::None)
	{
		HAZE_LOG_ERR_W("比较失败,比较类型为空,当前函数<%s>!\n", GetCurrFunction()->GetName().c_str());
	}

	GenIRCode(hss, this, GetInstructionOpCodeByCmpType(cmpType), ifJmpBlock, elseJmpBlock);
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

void CompilerModule::GenIRCode_JmpTo(Share<CompilerBlock> block)
{
	HAZE_STRING_STREAM hss;
	GenIRCode(hss, this, InstructionOpCode::JMP, block);
	m_Compiler->GetInsertBlock()->PushIRCode(hss.str());
}

Share<CompilerValue> CompilerModule::CreateGlobalVariable(const HazeDefineVariable& var, int line, Share<CompilerValue> refValue,
	x_uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	return m_GlobalDataFunction->CreateGlobalVariable(var, line, refValue, arrayDimension, params);
}

void CompilerModule::FunctionCall(HAZE_STRING_STREAM& hss, Share<CompilerFunction> callFunction, Share<CompilerValue> pointerFunction,
	AdvanceFunctionInfo* advancFunctionInfo, x_uint32& size, V_Array<Share<CompilerValue>>& params,
	Share<CompilerValue> thisPointerTo)
{
	static HazeDefineType s_UInt64 = HazeDefineType(HazeValueType::UInt64);
	static HazeDefineType s_Float64 = HazeDefineType(HazeValueType::Float64);

	Share<CompilerBlock> insertBlock = m_Compiler->GetInsertBlock();
	HString strName;

	auto pointerFunc = DynamicCast<CompilerPointerFunction>(pointerFunction);

	V_Array<const HazeDefineType*> funcTypes(params.size());
	if (!callFunction && !pointerFunc && !advancFunctionInfo)
	{
		COMPILER_ERR_MODULE_W("生成函数调用错误, <%s>为空", GetName().c_str(),
			callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"));
	}
	else
	{
		auto paramSize = callFunction ? callFunction->m_Params.size() : pointerFunc ? pointerFunc->m_ParamTypes.size() :
			advancFunctionInfo->Params.size();
		for (x_int64 i = params.size() - 1; i >= 0; i--)
		{
			auto Variable = params[i];
			auto type = callFunction ? &callFunction->GetParamTypeLeftToRightByIndex(params.size() - 1 - i) :
				pointerFunc ? &pointerFunc->GetParamTypeLeftToRightByIndex(params.size() - 1 - i) :
				advancFunctionInfo->Params.size() > params.size() - 1 - i ? &advancFunctionInfo->Params.at(params.size() - 1 - i)
				: &advancFunctionInfo->Params.at(advancFunctionInfo->Params.size() - 1);

			if (*type != Variable->GetValueType() && !Variable->GetValueType().IsStrongerType(*type))
			{
				if (i == (x_int64)params.size() - 1 && !IsMultiVariableTye(type->PrimaryType) && paramSize - (callFunction && callFunction->GetClass() ? 1 : 0) != params.size())
				{
					COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 应填入<%d>个参数，实际填入了<%d>个", GetName().c_str(),
						callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"), paramSize, params.size());
				}
				else if (IsMultiVariableTye(type->PrimaryType) && i == 0) {}
				else if (Variable->IsEnum())
				{
					auto enumValue = DynamicCast<CompilerEnumValue>(Variable);
					if (enumValue && enumValue->GetEnum() && enumValue->GetEnum()->GetParentType() == type->PrimaryType) {}
					else
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数枚举类型不匹配", GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : H_TEXT("函数指针"), params.size() - 1 - i);
					}
				}
				else if (type->IsStrongerType(Variable->GetValueType())) {}
				else if (IsRefrenceType(type->PrimaryType) && Variable->GetValueType().PrimaryType == type->SecondaryType) {}
				else if (!IsMultiVariableTye(type->PrimaryType))
				{
					COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数类型不匹配", GetName().c_str(),
						callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"),
						params.size() - 1 - i);
				}
			}

			if (IsMultiVariableTye(type->PrimaryType))
			{
				if (Variable->IsRefrence())
				{
					if (IsIntegerType(Variable->GetValueType().SecondaryType))
					{
						funcTypes[i] = &s_UInt64;
					}
					else if (IsFloatingType(Variable->GetValueType().SecondaryType))
					{
						funcTypes[i] = &s_Float64;
					}
					else
					{
						COMPILER_ERR_MODULE_W("生成函数调用<%s>错误, 第<%d>个参数引用类型不匹配", GetName().c_str(),
							callFunction ? callFunction->GetName().c_str() : pointerFunc ? H_TEXT("函数指针") : H_TEXT("复杂类型"),
							params.size() - 1 - i);
					}
				}
				else
				{
					funcTypes[i] = &Variable->GetValueType();
				}
			}
			else
			{
				funcTypes[i] = type;
			}
		}
	}

	for (x_uint64 i = 0; i < params.size(); i++)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, params[i], nullptr, funcTypes[i]);
		insertBlock->PushIRCode(hss.str());

		size += funcTypes[i] ? funcTypes[i]->GetCompilerTypeSize() : GetSizeByCompilerValue(params[i]);
		hss.str(H_TEXT(""));
		strName.clear();
	}

	if (thisPointerTo)
	{
		GenIRCode(hss, this, InstructionOpCode::PUSH, nullptr, thisPointerTo);
		insertBlock->PushIRCode(hss.str());

		hss.str(H_TEXT(""));

		size += GetSizeByCompilerValue(thisPointerTo);
	}

	hss << GetInstructionString(InstructionOpCode::PUSH) << " " << HAZE_CALL_PUSH_ADDRESS_NAME << " " << CAST_SCOPE(HazeVariableScope::None)
		<< " " << (x_uint32)HazeDataDesc::Address << " " << CAST_TYPE(HazeValueType::Int32) << std::endl;
	insertBlock->PushIRCode(hss.str());
	
	hss.str(H_TEXT(""));
}

Share<CompilerValue> CompilerModule::CreateFunctionCall(Share<CompilerFunction> callFunction, 
	V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo, const HString* nameSpace)
{
	HAZE_STRING_STREAM hss;
	x_uint32 size = 0;

	Share<CompilerValue> ret = nullptr;
	{
		PushTempRegister pushTempRegister(hss, m_Compiler, this, &callFunction->GetFunctionType(), &ret);
		FunctionCall(hss, callFunction, nullptr, nullptr, size, params, thisPointerTo);
		GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, callFunction, nullptr, nullptr, nullptr, nameSpace);
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
		PushTempRegister pushTempRegister(hss, m_Compiler, this, &DynamicCast<CompilerPointerFunction>(pointerFunction)->GetValueType(), &ret);
		FunctionCall(hss, nullptr, pointerFunction, nullptr, size, params, thisPointerTo);
		GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, nullptr, pointerFunction);
	}

	return ret;
}

Share<CompilerValue> CompilerModule::CreateAdvanceTypeFunctionCall(AdvanceFunctionInfo& functionInfo, 
	V_Array<Share<CompilerValue>>& params, Share<CompilerValue> thisPointerTo)
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

	{
		PushTempRegister pushTempRegister(hss, m_Compiler, this, nullptr, nullptr);
		FunctionCall(hss, nullptr, nullptr, &functionInfo, size, params, thisPointerTo);
		GenIRCode(hss, this, InstructionOpCode::CALL, params.size(), size, nullptr, nullptr, thisPointerTo, functionInfo.ClassFunc);
	}
	

	return Compiler::GetRegister(RET_REGISTER);
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

	HazeDefineType defineVarType;
	defineVarType.PrimaryType = HazeValueType::String;

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
	auto ret = m->GetGlobalVariable_Internal(name);
	if (ret)
	{
		return ret;
	}

	return m->m_Compiler->GetBaseModuleGlobalVariable(name);
}

bool CompilerModule::GetGlobalVariableName(CompilerModule* m, const Share<CompilerValue>& value, HString& outName, bool getOffset,
	V_Array<Pair<x_uint64, CompilerValue*>>* offsets)
{
	if (m->GetGlobalVariableName_Internal(value, outName, getOffset, offsets))
	{
		return true;
	}

	if (m->m_Compiler->GetBaseModuleGlobalVariableName(value, outName))
	{
		return true;
	}

	return value->TryGetVariableName(outName);
}

Share<CompilerValue> CompilerModule::GetGlobalVariable_Internal(const HString& name)
{
	auto variables = m_GlobalDataFunction->GetEntryBlock()->GetAllocaList();
	for (auto& it : variables)
	{
		if (it.first == name)
		{
			return it.second;
		}
	}

	for (auto& m : m_ImportModules)
	{
		auto ret = m->GetGlobalVariable_Internal(name);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

bool CompilerModule::GetGlobalVariableName_Internal(const Share<CompilerValue>& value, HString& outName, bool getOffset,
	V_Array<Pair<x_uint64, CompilerValue*>>* offsets)
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

	for (auto& it : m_ImportModules)
	{
		if (it->GetGlobalVariableName_Internal(value, outName, getOffset, offsets))
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

	return value->TryGetVariableName(outName);
}

Share<CompilerEnum> CompilerModule::GetEnum_Internal(const HString& name)
{
	auto iter = m_HashMap_Enums.find(name);
	if (iter != m_HashMap_Enums.end())
	{
		return iter->second;
	}

	for (auto& m : m_ImportModules)
	{
		auto ret = m->GetEnum_Internal(name);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

Share<CompilerClass> CompilerModule::GetClass(const HString& className)
{
	auto iter = m_HashMap_Classes.find(className);
	if (iter != m_HashMap_Classes.end())
	{
		return iter->second;
	}

	for (auto& it : m_ImportModules)
	{
		auto ret = it->GetClass(className);
		if (ret)
		{
			return ret;
		}
	}

	return nullptr;
}

x_uint32 CompilerModule::GetClassSize(const HString& className)
{
	return GetClass(className)->GetDataSize();
}

void CompilerModule::GenICode()
{
	HAZE_STRING_STREAM hss;
	hss << GetFileLastTime(m_Path) << std::endl;

	//版本 2个字节
	//FS_Ass << "1 1" << std::endl;

	//堆栈 4个字节
	//FS_Ass << 1024 << std::endl;

	//库类型
	if (m_Path.empty())
	{
		hss << GetName() << std::endl;
	}
	else
	{
		hss << m_Path << std::endl;
	}

	hss << (x_uint32)m_ModuleLibraryType << std::endl;
	
	hss << GetImportHeaderString() << std::endl;
	hss << m_ImportModules.size() << std::endl;
	for (int i = 0; i < m_ImportModules.size(); i++)
	{
		if (!m_ImportModules[i]->GetPath().empty())
		{
			hss << GetImportHeaderModuleString() << " " << m_ImportModules[i]->GetPath() << std::endl;
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
	hss << GetGlobalDataHeaderString() << std::endl;

	auto globaleVariables = m_GlobalDataFunction->GetEntryBlock()->GetAllocaList();
	hss << globaleVariables.size() << std::endl;

	for (int i = 0; i < globaleVariables.size(); i++)
	{
		auto& var = globaleVariables[i].second;
	
		hss << globaleVariables[i].first << " ";
		var->GetValueType().StringStreamTo(hss);
		hss << std::endl;
	}

	//全局变量初始化
	/*hss << GetGlobalDataInitBlockStart() << std::endl;
	for (size_t i = 0; i < m_ModuleIRCodes.size(); i++)
	{
		hss << m_ModuleIRCodes[i].c_str();
	}
	hss << GetGlobalDataInitBlockEnd() << std::endl;*/
	

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
	hss << GetStringTableHeaderString() << std::endl;
	hss << m_HashMap_StringMapping.size() << std::endl;

	for (auto& it : m_HashMap_StringMapping)
	{
		hss << it.second->length() << " " << *it.second << std::endl;
	}

	/*
	*	类表 ：	个数
	*				名称 指令流
	*
	*/
	x_uint64 functionSize = 0;

	hss << GetClassTableHeaderString() << std::endl;
	hss << m_HashMap_Classes.size() << std::endl;
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
	hss << GetFucntionTableHeaderString() << std::endl;
	hss << m_HashMap_Functions.size() + functionSize << std::endl;

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

	*m_FS_I_Code << hss.str();

	if (m_IsGenTemplateCode)
	{
		m_IsGenTemplateCode = false;
	}
}