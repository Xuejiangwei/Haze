#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeUtility.h"
#include "HazeVM.h"
#include "HazeExecuteFile.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

#define BACKEND_INSTRUCTION_LOG			0

static std::pair<bool, int> ParseStringCount = { false, 0 };

static void FindObjectAndMemberName(const HAZE_STRING& inName, HAZE_STRING& outObjectName, 
	HAZE_STRING& outMemberName, bool& objectIsPointer)
{
	size_t pos = inName.find(HAZE_CLASS_POINTER_ATTR);
	if (pos != HAZE_STRING::npos)
	{
		outObjectName = inName.substr(0, pos);
		outMemberName = inName.substr(pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());
		objectIsPointer = true;
	}
	else
	{
		pos = inName.find(HAZE_CLASS_ATTR);
		if (pos != HAZE_STRING::npos)
		{
			outObjectName = inName.substr(0, pos);
			outMemberName = inName.substr(pos + HAZE_STRING(HAZE_CLASS_ATTR).size());
			objectIsPointer = false;
		}
	}
}

static bool IsIgnoreFindAddressInsCode(ModuleUnit::FunctionInstruction& ins)
{
	if (ins.InsCode == InstructionOpCode::LINE)
	{
		return true;
	}

	if (ins.InsCode == InstructionOpCode::CALL && ins.Operator[0].Variable.Type.PrimaryType == HazeValueType::Function)
	{
		return true;
	}

	if (ins.InsCode == InstructionOpCode::RET && IsVoidType(ins.Operator[0].Variable.Type.PrimaryType))
	{
		return true;
	}

	return false;
}

static bool IsIgnoreFindAddress(InstructionData& operatorData)
{
	if (operatorData.Desc == HazeDataDesc::Constant)
	{
		operatorData.AddressType = InstructionAddressType::Constant;
		return true;
	}

	if (operatorData.Desc == HazeDataDesc::ConstantString)
	{
		operatorData.AddressType = InstructionAddressType::ConstantString;
		return true;
	}

	if (operatorData.Desc == HazeDataDesc::CallFunctionModule)
	{
		return true;
	}

	if (operatorData.Variable.Name == HAZE_CALL_PUSH_ADDRESS_NAME)
	{
		operatorData.AddressType = InstructionAddressType::Local;
		return true;
	}

	if (IsRegisterDesc(operatorData.Desc))
	{
		operatorData.AddressType = InstructionAddressType::Register;
		return true;
	}

	return false;
}

BackendParse::BackendParse(HazeVM* vm) : m_VM(vm), m_CurrCode(nullptr)
{
}

BackendParse::~BackendParse()
{
}

void BackendParse::Parse()
{
	HAZE_STRING interCodePath;

	auto& refModules = m_VM->GetReferenceModules();

	HAZE_STRING codeText;

	for (auto& refModule : refModules)
	{
		m_CurrParseModule = std::make_shared<ModuleUnit>(refModule);
		m_Modules[refModule] = m_CurrParseModule;

		HAZE_IFSTREAM fs(GetIntermediateModuleFile(refModule));
		fs.imbue(std::locale("chs"));

		HAZE_STRING Content(std::istreambuf_iterator<HAZE_CHAR>(fs), {});

		codeText = std::move(Content);
		m_CurrCode = codeText.c_str();

		Parse_I_Code();

		fs.close();
	}

	GenOpCodeFile();
}

void BackendParse::GetNextLexeme()
{
	m_CurrLexeme.clear();
	while (HazeIsSpace(*m_CurrCode))
	{
		m_CurrCode++;
	}

	m_CurrLexeme.clear();
	while (!HazeIsSpace(*m_CurrCode) || (ParseStringCount.first && ParseStringCount.second > 0))
	{
		m_CurrLexeme += *(m_CurrCode++);
		ParseStringCount.second--;
	}
}

void BackendParse::Parse_I_Code()
{
	//Standard lib
	GetNextLexeme();
	m_CurrParseModule->m_LibraryType = (HazeLibraryType)StringToStandardType<uint32>(m_CurrLexeme);

	//Global data
	GetNextLexeme();
	Parse_I_Code_GlobalTable();

	//String table
	GetNextLexeme();
	Parse_I_Code_StringTable();

	//Class table
	GetNextLexeme();
	Parse_I_Code_ClassTable();

	//Function table
	GetNextLexeme();
	Parse_I_Code_FunctionTable();
}

void BackendParse::Parse_I_Code_GlobalTable()
{
	if (m_CurrLexeme == GetGlobalDataHeaderString())
	{
		GetNextLexeme();
		uint32 number = StringToStandardType<uint32>(m_CurrLexeme);

		ModuleUnit::GlobalDataTable& table = m_CurrParseModule->m_GlobalDataTable;
		table.m_Data.resize(number);

		for (size_t i = 0; i < table.m_Data.size(); i++)
		{
			GetNextLexmeAssign_HazeString(table.m_Data[i].m_Name);

			GetNextLexmeAssign_StandardType(table.m_Data[i].Size);

			GetNextLexmeAssign_CustomType<uint32>(table.m_Data[i].m_Type.PrimaryType);

			if (IsHazeDefaultType(table.m_Data[i].m_Type.PrimaryType))
			{
				GetNextLexeme();
				StringToHazeValueNumber(m_CurrLexeme, table.m_Data[i].m_Type.PrimaryType, table.m_Data[i].Value);
			}
			else
			{
				if (IsClassType(table.m_Data[i].m_Type.PrimaryType))
				{
					GetNextLexmeAssign_HazeString(table.m_Data[i].m_Type.CustomName);
					table.ClassObjectAllSize += table.m_Data[i].Size;
				}
			}
		}
	}
}

void BackendParse::Parse_I_Code_StringTable()
{
	if (m_CurrLexeme == GetStringTableHeaderString())
	{
		uint32 number;
		GetNextLexmeAssign_StandardType(number);

		ModuleUnit::StringTable& table = m_CurrParseModule->m_StringTable;
		table.Strings.resize(number);

		ParseStringCount.first = true;
		for (size_t i = 0; i < table.Strings.size(); i++)
		{
			GetNextLexmeAssign_StandardType(ParseStringCount.second);
			GetNextLexmeAssign_HazeString(table.Strings[i].String);
		}
		ParseStringCount.first = false;
	}
}

void BackendParse::Parse_I_Code_ClassTable()
{
	if (m_CurrLexeme == GetClassTableHeaderString())
	{
		uint32 number;
		GetNextLexmeAssign_StandardType(number);

		ModuleUnit::ClassTable& table = m_CurrParseModule->m_ClassTable;
		table.Classes.resize(number);

		for (size_t i = 0; i < table.Classes.size(); i++)
		{
			GetNextLexeme();

			if (m_CurrLexeme == GetClassLabelHeader())
			{
				auto& classData = table.Classes[i];

				GetNextLexmeAssign_HazeString(classData.m_Name);
				GetNextLexmeAssign_StandardType(classData.Size);
				GetNextLexmeAssign_StandardType(number);
				classData.Members.resize(number);

				for (size_t j = 0; j < classData.Members.size(); j++)
				{
					auto& classMember = classData.Members[j];

					GetNextLexmeAssign_HazeString(classMember.Variable.Name);

					GetNextLexmeAssign_CustomType<uint32>(classMember.Variable.Type.PrimaryType);

					if (classMember.Variable.Type.NeedSecondaryType())
					{
						GetNextLexmeAssign_CustomType<uint32>(classMember.Variable.Type.SecondaryType);
					}

					if (classMember.Variable.Type.NeedCustomName())
					{
						GetNextLexmeAssign_HazeString(classMember.Variable.Type.CustomName);
					}

					GetNextLexmeAssign_CustomType<uint32>(classMember.Offset);
					GetNextLexmeAssign_CustomType<uint32>(classMember.Size);
				}
			}
		}
	}
}

void BackendParse::Parse_I_Code_FunctionTable()
{
	if (m_CurrLexeme == GetFucntionTableHeaderString())
	{
		uint32 number;
		GetNextLexmeAssign_StandardType(number);

		ModuleUnit::FunctionTable& table = m_CurrParseModule->m_FunctionTable;
		table.m_Functions.resize(number);

		for (size_t i = 0; i < table.m_Functions.size(); i++)
		{
			GetNextLexeme();

			if (m_CurrLexeme == GetFunctionLabelHeader())
			{
				table.m_Functions[i].DescType = GetFunctionTypeByLibraryType(m_CurrParseModule->m_LibraryType);

				GetNextLexmeAssign_HazeString(table.m_Functions[i].Name);

				GetNextLexmeAssign_CustomType<uint32>(table.m_Functions[i].Type);

				GetNextLexeme();
				while (m_CurrLexeme == GetFunctionParamHeader())
				{
					HazeDefineVariable param;
					GetNextLexmeAssign_HazeString(param.Name);
					param.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeString,
						&BackendParse::GetNextLexmeAssign_CustomType<uint32>);

					table.m_Functions[i].Params.push_back(std::move(param));

					GetNextLexeme();
				}

				while (m_CurrLexeme == GetFunctionVariableHeader())
				{
					HazeVariableData var;
					GetNextLexmeAssign_HazeString(var.Variable.Name);

					var.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeString,
						&BackendParse::GetNextLexmeAssign_CustomType<uint32>);
					GetNextLexmeAssign_CustomType<int>(var.Offset);
					GetNextLexmeAssign_CustomType<uint32>(var.Size);
					GetNextLexmeAssign_CustomType<uint32>(var.Line);

					table.m_Functions[i].Variables.push_back(std::move(var));
					GetNextLexeme();
				}

				if (m_CurrLexeme == GetFunctionStartHeader())
				{
					GetNextLexmeAssign_CustomType<int>(table.m_Functions[i].StartLine);

					GetNextLexeme();
					while (m_CurrLexeme == BLOCK_START)
					{
						GetNextLexeme();
						table.m_Functions[i].Blocks.push_back({ m_CurrLexeme, 0, (int)table.m_Functions[i].Instructions.size() });
						auto& CurrBlock = table.m_Functions[i].Blocks.back();

						GetNextLexeme();
						while (m_CurrLexeme != BLOCK_START && m_CurrLexeme != GetFunctionEndHeader())
						{
							ModuleUnit::FunctionInstruction Instruction;
							Instruction.InsCode = GetInstructionByString(m_CurrLexeme);

							ParseInstruction(Instruction);

							table.m_Functions[i].Instructions.push_back(Instruction);

							CurrBlock.InstructionNum++;

							GetNextLexeme();
						}

						if (m_CurrLexeme == GetFunctionEndHeader())
						{
							GetNextLexmeAssign_CustomType<int>(table.m_Functions[i].EndLine);
							break;
						}
					}
				}
			}
		}
	}
}

void BackendParse::ParseInstructionData(InstructionData& data)
{
	GetNextLexmeAssign_HazeString(data.Variable.Name);
	GetNextLexmeAssign_CustomType<uint32>(data.Scope);
	GetNextLexmeAssign_CustomType<uint32>(data.Desc);

	data.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeString,
		&BackendParse::GetNextLexmeAssign_CustomType<uint32>);

	if (data.Desc == HazeDataDesc::ArrayElement || data.Desc == HazeDataDesc::ConstantString)
	{
		GetNextLexmeAssign_CustomType<uint32>(data.Extra.Index);
	}
}

void BackendParse::ParseInstruction(ModuleUnit::FunctionInstruction& instruction)
{
	switch (instruction.InsCode)
	{
	case InstructionOpCode::NONE:
		break;
	case InstructionOpCode::MOV:
	case InstructionOpCode::MOVPV:
	case InstructionOpCode::MOVTOPV:
	case InstructionOpCode::LEA:
	case InstructionOpCode::ADD:
	case InstructionOpCode::SUB:
	case InstructionOpCode::MUL:
	case InstructionOpCode::DIV:
	case InstructionOpCode::MOD:
	case InstructionOpCode::CMP:
	case InstructionOpCode::NOT:
	case InstructionOpCode::BIT_AND:
	case InstructionOpCode::BIT_OR:
	case InstructionOpCode::BIT_XOR:
	case InstructionOpCode::SHL:
	case InstructionOpCode::SHR:
	case InstructionOpCode::ADD_ASSIGN:
	case InstructionOpCode::SUB_ASSIGN:
	case InstructionOpCode::MUL_ASSIGN:
	case InstructionOpCode::DIV_ASSIGN:
	case InstructionOpCode::MOD_ASSIGN:
	case InstructionOpCode::BIT_AND_ASSIGN:
	case InstructionOpCode::BIT_OR_ASSIGN:
	case InstructionOpCode::BIT_XOR_ASSIGN:
	case InstructionOpCode::SHL_ASSIGN:
	case InstructionOpCode::SHR_ASSIGN:
	case InstructionOpCode::CVT:
	{
		InstructionData operatorOne;
		InstructionData operatorTwo;

		ParseInstructionData(operatorOne);
		ParseInstructionData(operatorTwo);

		instruction.Operator = { operatorOne, operatorTwo };
	}
	break;
	case InstructionOpCode::NEG:
	case InstructionOpCode::INC:
	case InstructionOpCode::DEC:
	case InstructionOpCode::BIT_NEG:
	case InstructionOpCode::PUSH:
	case InstructionOpCode::POP:
	case InstructionOpCode::RET:
	{
		InstructionData operatorOne;
		ParseInstructionData(operatorOne);

		instruction.Operator = { operatorOne };
	}
	break;
	case InstructionOpCode::CALL:
	{
		InstructionData operatorOne;
		InstructionData operatorTwo;

		GetNextLexmeAssign_HazeString(operatorOne.Variable.Name);

		GetNextLexmeAssign_CustomType<uint32>(operatorOne.Variable.Type.PrimaryType);

		if (IsPointerFunction(operatorOne.Variable.Type.PrimaryType))
		{
			GetNextLexmeAssign_CustomType<uint32>(operatorOne.Scope);
			GetNextLexmeAssign_CustomType<uint32>(operatorOne.Desc);
		}

		GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamNum);
		GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamByteSize);

		GetNextLexmeAssign_HazeString(operatorTwo.Variable.Name);
		operatorTwo.Desc = HazeDataDesc::CallFunctionModule;

		instruction.Operator = { operatorOne, operatorTwo };
	}
	break;
	case InstructionOpCode::NEW:
	{
		InstructionData operatorOne;

		GetNextLexmeAssign_CustomType<uint32>(operatorOne.Variable.Type.PrimaryType);

		if (operatorOne.Variable.Type.PrimaryType == HazeValueType::PointerClass)
		{
			GetNextLexmeAssign_HazeString(operatorOne.Variable.Type.CustomName);
		}
		else
		{
			GetNextLexmeAssign_CustomType<uint32>(operatorOne.Variable.Type.SecondaryType);
		}

		GetNextLexmeAssign_CustomType<uint32>(operatorOne.Scope);
		GetNextLexmeAssign_CustomType<uint32>(operatorOne.Desc);

		instruction.Operator = { operatorOne };
	}
	break;
	case InstructionOpCode::JMP:
	{
		InstructionData operatorOne;
		GetNextLexmeAssign_HazeString(operatorOne.Variable.Name);

		instruction.Operator = { operatorOne };
	}
	break;
	case InstructionOpCode::JNE:
	case InstructionOpCode::JNG:
	case InstructionOpCode::JNL:
	case InstructionOpCode::JE:
	case InstructionOpCode::JG:
	case InstructionOpCode::JL:
	{
		InstructionData operatorOne;
		InstructionData operatorTwo;

		GetNextLexmeAssign_HazeString(operatorOne.Variable.Name);

		GetNextLexmeAssign_HazeString(operatorTwo.Variable.Name);

		instruction.Operator = { operatorOne, operatorTwo };
	}
	break;
	case InstructionOpCode::LINE:
	{
		InstructionData operatorOne;
		GetNextLexmeAssign_CustomType<uint32>(operatorOne.Extra.Line);

		instruction.Operator = { operatorOne };
	}
	break;
	default:
		break;
	}
}

void BackendParse::GenOpCodeFile()
{
	HazeExecuteFile exeFile(ExeFileType::Out);

	if (m_VM->IsDebug())
	{
		exeFile.WriteModule(m_Modules);
	}

	ModuleUnit::GlobalDataTable newGlobalDataTable;
	ModuleUnit::StringTable newStringTable;
	ModuleUnit::ClassTable newClassTable;
	ModuleUnit::FunctionTable newFunctionTable;

	size_t functionCount = 0;
	for (auto& iter : m_Modules)
	{
		newFunctionTable.m_Functions.insert(newFunctionTable.m_Functions.end(), 
			iter.second->m_FunctionTable.m_Functions.begin(), iter.second->m_FunctionTable.m_Functions.end());
		ReplaceStringIndex(newStringTable, newFunctionTable, functionCount);

		newGlobalDataTable.m_Data.insert(newGlobalDataTable.m_Data.end(), 
			iter.second->m_GlobalDataTable.m_Data.begin(), iter.second->m_GlobalDataTable.m_Data.end());
		newGlobalDataTable.ClassObjectAllSize += iter.second->m_GlobalDataTable.ClassObjectAllSize;

		newStringTable.Strings.insert(newStringTable.Strings.end(), 
			iter.second->m_StringTable.Strings.begin(), iter.second->m_StringTable.Strings.end());
		newClassTable.Classes.insert(newClassTable.Classes.end(), iter.second->m_ClassTable.Classes.begin(), iter.second->m_ClassTable.Classes.end());
	}

	FindAddress(newGlobalDataTable, newFunctionTable);

	exeFile.WriteExecuteFile(newGlobalDataTable, newStringTable, newClassTable, newFunctionTable);
}

void BackendParse::ReplaceStringIndex(ModuleUnit::StringTable& newStringTable,
	ModuleUnit::FunctionTable& newFunctionTable, size_t& functionCount)
{
	for (; functionCount < newFunctionTable.m_Functions.size(); functionCount++)
	{
		for (auto& iter : newFunctionTable.m_Functions[functionCount].Instructions)
		{
			for (auto& oper : iter.Operator)
			{
				if (oper.Desc == HazeDataDesc::ConstantString)
				{
					oper.Extra.Index += (uint32)newStringTable.Strings.size();
					oper.AddressType = InstructionAddressType::ConstantString;
				}
			}
		}
	}
}

void ResetFunctionBlockOffset(InstructionData& operatorData, ModuleUnit::FunctionTableData& function)
{
	if (operatorData.Variable.Name != HAZE_JMP_NULL)
	{
		for (size_t i = 0; i < function.Blocks.size(); i++)
		{
			if (operatorData.Variable.Name == function.Blocks[i].BlockName)
			{
				operatorData.Extra.Jmp.StartAddress = function.Blocks[i].StartAddress;
				operatorData.Extra.Jmp.InstructionNum = function.Blocks[i].InstructionNum;
				break;
			}
		}
	}
}

inline void BackendParse::ResetLocalOperatorAddress(InstructionData& operatorData,ModuleUnit::FunctionTableData& function,
	std::unordered_map<HAZE_STRING, int>& localVariable, ModuleUnit::GlobalDataTable& newGlobalDataTable)
{
	HAZE_STRING objName;
	HAZE_STRING memberName;
	bool isPointer = false;

	if (IsClassMember(operatorData.Desc))
	{
		FindObjectAndMemberName(operatorData.Variable.Name, objName, memberName, isPointer);
		for (uint32 i = 0; i < function.Variables.size(); i++)
		{
			if (function.Variables[i].Variable.Name == objName)
			{
				auto classData = GetClass(function.Variables[i].Variable.Type.CustomName);
				if (isPointer)
				{
					operatorData.Extra.Address.BaseAddress = function.Variables[i].Offset;
					operatorData.Extra.Address.Offset = GetMemberOffset(*classData, memberName);
					operatorData.AddressType = InstructionAddressType::Local_BasePointer_Offset;
					break;
				}
				else
				{
					operatorData.Extra.Address.BaseAddress = function.Variables[i].Offset + GetMemberOffset(*classData, memberName);
					operatorData.AddressType = InstructionAddressType::Local;
					break;
				}
			}
		}
	}
	else if (operatorData.Desc == HazeDataDesc::ArrayElement)
	{
		auto iterIndex = localVariable.find(operatorData.Variable.Name);
		if (iterIndex != localVariable.end())
		{
			//避免先把Index值替换掉
			operatorData.Extra.Address.Offset = operatorData.Extra.Index * GetSizeByType(operatorData.Variable.Type, this);
			operatorData.Extra.Address.BaseAddress = function.Variables[iterIndex->second].Offset;
			operatorData.AddressType = InstructionAddressType::Local_Base_Offset;
		}
		else if (function.Variables[0].Variable.Name.substr(0, 1) == HAZE_CLASS_THIS)
		{
			auto classData = GetClass(function.Variables[0].Variable.Type.CustomName);
			if (classData)
			{
				operatorData.Extra.Address.Offset = GetMemberOffset(*classData, operatorData.Variable.Name)
					+ operatorData.Extra.Index * GetSizeByType(operatorData.Variable.Type, this);
				operatorData.Extra.Address.BaseAddress = function.Variables[0].Offset;
				operatorData.AddressType = InstructionAddressType::Local_BasePointer_Offset;
			}
			else
			{
				HAZE_LOG_ERR_W("查找变量<%s>的偏移地址错误,当前函数<%s>,当前类<%s>未找到!\n",
					operatorData.Variable.Name.c_str(), function.Name.c_str(),
					function.Variables[0].Variable.Type.CustomName.c_str());
			}
		}
		else
		{
			HAZE_LOG_ERR_W("查找变量<%s>的偏移地址错误,当前函数<%s>!\n", operatorData.Variable.Name.c_str(), 
				function.Name.c_str());
		}
	}
	else
	{
		auto iterIndex = localVariable.find(operatorData.Variable.Name);
		if (iterIndex != localVariable.end())
		{
			operatorData.Extra.Address.BaseAddress = function.Variables[iterIndex->second].Offset;
			operatorData.AddressType = InstructionAddressType::Local;
		}
		else
		{
			HAZE_LOG_ERR_W("在函数<%s>中查找<%s>的偏移值失败!\n", function.Name.c_str(),
				operatorData.Variable.Name.c_str());
			return;
		}
	}
}

inline void BackendParse::ResetGlobalOperatorAddress(InstructionData& operatorData, ModuleUnit::FunctionTableData& function,
	std::unordered_map<HAZE_STRING, int>& localVariable, ModuleUnit::GlobalDataTable& newGlobalDataTable)
{
	HAZE_STRING objName;
	HAZE_STRING memberName;
	bool isPointer = false;
	int index = 0;

	if (IsClassMember(operatorData.Desc))
	{
		FindObjectAndMemberName(operatorData.Variable.Name, objName, memberName, isPointer);
		index = (int)newGlobalDataTable.GetIndex(objName);
		if (index >= 0)
		{
			auto Class = GetClass(newGlobalDataTable.m_Data[index].m_Type.CustomName);
			if (Class)
			{
				operatorData.AddressType = InstructionAddressType::Global_Base_Offset;
				operatorData.Extra.Index = index;
				operatorData.Extra.Address.Offset = GetMemberOffset(*Class, memberName);
			}
		}
		else
		{
			HAZE_LOG_ERR_W("未能查找到全局变量类对象成员<%s>!\n", operatorData.Variable.Name.c_str());
			return;
		}
	}
	else if (operatorData.Desc == HazeDataDesc::ArrayElement)
	{
		index = (int)newGlobalDataTable.GetIndex(operatorData.Variable.Name);
		if (index >= 0)
		{
			//避免先把Index值替换掉
			operatorData.Extra.Address.Offset = operatorData.Extra.Index * GetSizeByType(operatorData.Variable.Type, this);
			operatorData.Extra.Index = index;
			operatorData.AddressType = InstructionAddressType::Global_Base_Offset;
		}
	}
	else
	{
		index = (int)newGlobalDataTable.GetIndex(operatorData.Variable.Name);
		if (index >= 0)
		{
			operatorData.Extra.Index = index;
			operatorData.AddressType = InstructionAddressType::Global;
		}
		else
		{
			if (operatorData.Desc == HazeDataDesc::Constant)
			{
				return;
			}

			HAZE_LOG_ERR_W("未能查找到全局变量<%s>!\n", operatorData.Variable.Name.c_str());
			return;
		}
	}
}

void BackendParse::FindAddress(ModuleUnit::GlobalDataTable& newGlobalDataTable,
	ModuleUnit::FunctionTable& newFunctionTable)
{
	std::unordered_map<HAZE_STRING, size_t> HashMap_FunctionIndexAndAddress;
	for (size_t i = 0; i < newFunctionTable.m_Functions.size(); i++)
	{
		HashMap_FunctionIndexAndAddress[newFunctionTable.m_Functions[i].Name] = i;
	}

	//替换变量为索引或相对函数起始偏移
	for (size_t k = 0; k < newFunctionTable.m_Functions.size(); ++k)
	{
#if BACKEND_INSTRUCTION_LOG
		std::wcout << NewFunctionTable.Vector_Function[k].Name << std::endl;
#endif
		auto& m_CurrFunction = newFunctionTable.m_Functions[k];

		std::unordered_map<HAZE_STRING, int> localVariables;
		for (size_t i = 0; i < m_CurrFunction.Variables.size(); i++)
		{
			localVariables[m_CurrFunction.Variables[i].Variable.Name] = (int)i;
		}

		for (size_t i = 0; i < m_CurrFunction.Instructions.size(); ++i)
		{
			if (IsJmpOpCode(m_CurrFunction.Instructions[i].InsCode))
			{
				//设置 Block块 偏移值
				for (auto& operatorData : m_CurrFunction.Instructions[i].Operator)
				{
					if (operatorData.Variable.Name != HAZE_JMP_NULL)
					{
						ResetFunctionBlockOffset(operatorData, m_CurrFunction);
					}
				}
			}
			/*else if (CurrFunction.Vector_Instruction[i].InsCode == InstructionOpCode::CALL
				&& CurrFunction.Vector_Instruction[i].Operator[0].Variable.Type.PrimaryType == HazeValueType::PointerFunction)
			{
				auto Iter_Index = HashMap_Variable.find(CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name);
				if (Iter_Index != HashMap_Variable.end())
				{
					CurrFunction.Vector_Instruction[i].Operator[1].Extra.Address.BaseAddress = CurrFunction.Vector_Variable[Iter_Index->second].Offset;
				}
				else
				{
					int Index = NewGlobalDataTable.GetIndex(CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name);
					if (Index >= 0)
					{
						CurrFunction.Vector_Instruction[i].Operator[1].Extra.Index = (int)Index;
					}
					else
					{
						HAZE_LOG_ERR(HAZE_TEXT("查找调用的函数指针<%s>失败!\n"), CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name.c_str());
					}
				}
			}*/
			else if (!IsIgnoreFindAddressInsCode(m_CurrFunction.Instructions[i]))
			{
				HAZE_STRING objName;
				HAZE_STRING memberName;

				for (auto& operatorData : m_CurrFunction.Instructions[i].Operator)
				{
					if (!IsIgnoreFindAddress(operatorData))
					{
						if (IS_SCOPE_LOCAL(operatorData.Scope))
						{
							ResetLocalOperatorAddress(operatorData, m_CurrFunction, localVariables, newGlobalDataTable);
						}
						else if (IS_SCOPE_GLOBAL(operatorData.Scope))
						{
							ResetGlobalOperatorAddress(operatorData, m_CurrFunction, localVariables, newGlobalDataTable);
						}
						else if (IS_SCOPE_TEMP(operatorData.Scope))
						{
							if (operatorData.Desc == HazeDataDesc::FunctionAddress)
							{
								operatorData.AddressType = InstructionAddressType::FunctionAddress;
							}
							else
							{
								HAZE_LOG_ERR_W("寻找临时变量<%s>的地址失败!\n", operatorData.Variable.Name.c_str());
							}
						}
						else
						{
							HAZE_LOG_ERR_W("寻找变量<%s>的地址失败!\n", operatorData.Variable.Name.c_str());
						}
					}
				}
			}

#if BACKEND_INSTRUCTION_LOG
			HAZE_STRING_STREAM WSS;
			WSS << "Replace: " << GetInstructionString(NewFunctionTable.Vector_Function[k].Vector_Instruction[i].InsCode) << " ";
			for (auto& it : NewFunctionTable.Vector_Function[k].Vector_Instruction[i].Operator)
			{
				WSS << it.Variable.Name << " Base " << it.Extra.Address.BaseAddress << " Offset " << it.Extra.Address.Offset << " ";
			}
			WSS << std::endl;
			std::wcout << WSS.str();
#endif // ENABLE_BACKEND_INSTRUCTION_LOG
		}
	}
}

const ModuleUnit::ClassTableData* const BackendParse::GetClass(const HAZE_STRING& className)
{
	for (auto& iter : m_Modules)
	{
		for (auto& classData : iter.second->m_ClassTable.Classes)
		{
			if (classData.m_Name == className)
			{
				return &classData;
			}
		}
	}

	return nullptr;
}

uint32 const BackendParse::GetClassSize(const HAZE_STRING& className)
{
	auto classData = GetClass(className);
	if (classData)
	{
		return classData->Size;
	}
	return 0;
}

uint32 BackendParse::GetMemberOffset(const ModuleUnit::ClassTableData& classData, const HAZE_STRING& memberName)
{
	for (auto& iter : classData.Members)
	{
		if (iter.Variable.Name == memberName)
		{
			return iter.Offset;
		}
	}

	return (uint32)-1;
}