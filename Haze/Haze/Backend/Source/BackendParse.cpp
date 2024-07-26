#include "HazePch.h"
#include <filesystem>
#include <fstream>

#include "BackendParse.h"
#include "HazeUtility.h"
#include "HazeVM.h"
#include "HazeExecuteFile.h"
#include "HazeFilePathHelper.h"
#include "HazeLog.h"

#define BACKEND_INSTRUCTION_LOG			0

static Pair<bool, int> ParseStringCount = { false, 0 };

static void FindObjectAndMemberName(const HString& inName, HString& outObjectName, 
	HString& outMemberName, bool& objectIsPointer)
{
	/*size_t pos = inName.find(TOKEN_THIS);
	if (pos != HString::npos)
	{
		outObjectName = inName.substr(0, pos);
		outMemberName = inName.substr(pos + HString(TOKEN_THIS).size());
		objectIsPointer = true;
	}
	else
	{
		pos = inName.find(TOKEN_THIS);
		if (pos != HString::npos)
		{
			outObjectName = inName.substr(0, pos);
			outMemberName = inName.substr(pos + HString(TOKEN_THIS).size());
			objectIsPointer = false;
		}
	}*/
}

static bool IsIgnoreFindAddressInsCode(ModuleUnit::FunctionInstruction& ins)
{
	if (ins.InsCode == InstructionOpCode::LINE || ins.InsCode == InstructionOpCode::SIGN)
	{
		return true;
	}

	if (ins.InsCode == InstructionOpCode::CALL && ins.Operator[0].Variable.Type.PrimaryType == HazeValueType::Function && 
		(ins.Operator[0].Scope == HazeVariableScope::Temp || ins.Operator[0].Scope == HazeVariableScope::Ignore))
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
	if (operatorData.Desc == HazeDataDesc::CallFunctionPointer)
	{
		operatorData.AddressType = InstructionAddressType::PointerAddress;
		return true;
	}

	if (operatorData.Desc == HazeDataDesc::Constant)
	{
		operatorData.AddressType = InstructionAddressType::Constant;
		return true;
	}

	if (operatorData.Desc == HazeDataDesc::NullPtr)
	{
		operatorData.AddressType = InstructionAddressType::NullPtr;
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
	auto& refModules = m_VM->GetReferenceModules();
	HString codeText;

	{
		HAZE_IFSTREAM fs(GetIntermediateModuleFile(HAZE_INTER_SYMBOL_TABLE));
		fs.imbue(std::locale("chs"));
		codeText = HString(std::istreambuf_iterator<HChar>(fs), {});
		Parse_I_Symbol();
		fs.close();
	}

	for (auto& refModule : refModules)
	{
		m_CurrParseModule = MakeShare<ModuleUnit>(refModule);
		m_Modules[refModule] = m_CurrParseModule;

		HAZE_IFSTREAM fs(GetIntermediateModuleFile(refModule));
		fs.imbue(std::locale("chs"));

		HString Content(std::istreambuf_iterator<HChar>(fs), {});

		codeText = Move(Content);
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
		if (!ParseStringCount.first)
		{
		}
		else
		{
			break;
		}
	}

	m_CurrLexeme.clear();
	while (!HazeIsSpace(*m_CurrCode) || (ParseStringCount.first && ParseStringCount.second > 0))
	{
		m_CurrLexeme += *(m_CurrCode++);
		ParseStringCount.second--;
	}
}

void BackendParse::GetNextLexmeAssign_HazeStringCustomClassName(const HString*& dst)
{
	GetNextLexeme();
	auto iter = m_InterSymbol.find(m_CurrLexeme);
	if (iter != m_InterSymbol.end())
	{
		dst = &(*iter);
	}
	else
	{
		BACKEND_ERR_W("在符号表中未能查找到<%s>类型信息", m_CurrLexeme.c_str());
	}
}

void BackendParse::Parse_I_Symbol()
{
	while (m_CurrCode && HazeIsSpace(*m_CurrCode))
	{
		GetNextLexeme();
		m_InterSymbol.insert(m_CurrLexeme);
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
	HString str;
	if (m_CurrLexeme == GetGlobalDataHeaderString())
	{
		GetNextLexeme();
		uint32 number = StringToStandardType<uint32>(m_CurrLexeme);

		ModuleUnit::GlobalDataTable& table = m_CurrParseModule->m_GlobalDataTable;
		table.Data.resize(number);

		for (size_t i = 0; i < table.Data.size(); i++)
		{
			//因为将所有的指令合在一起了，需要重新计算
			GetNextLexmeAssign_CustomType<uint32>(table.Data[i].StartAddress);
			GetNextLexmeAssign_CustomType<uint32>(table.Data[i].EndAddress);

			GetNextLexmeAssign_HazeString(table.Data[i].Name);

			GetNextLexmeAssign_StandardType(table.Data[i].Size);

			GetNextLexmeAssign_CustomType<uint32>(table.Data[i].Type.PrimaryType);

			if (IsHazeBaseType(table.Data[i].Type.PrimaryType))
			{
				GetNextLexeme();
				StringToHazeValueNumber(m_CurrLexeme, table.Data[i].Type.PrimaryType, table.Data[i].Value);
			}
			else
			{
				if (table.Data[i].Type.NeedCustomName())
				{
					GetNextLexmeAssign_HazeString(str);
					auto iter = m_InterSymbol.find(str);
					if (iter != m_InterSymbol.end())
					{
						table.Data[i].Type.CustomName = &(*iter);
					}
					else
					{
						HAZE_LOG_ERR_W("解析全局变量错误, 在符号表没找到对应类<%s>", str.c_str());
					}

					if (IsClassType(table.Data[i].Type.PrimaryType))
					{
						table.ClassObjectAllSize += table.Data[i].Size;
					}
				}
			}
		}

		GetNextLexeme();
		if (m_CurrLexeme == GetGlobalDataInitBlockStart())
		{
			GetNextLexeme();
			while (m_CurrLexeme != GetGlobalDataInitBlockEnd())
			{
				ModuleUnit::FunctionInstruction Instruction;
				Instruction.InsCode = GetInstructionByString(m_CurrLexeme);

				ParseInstruction(Instruction);

				table.Instructions.push_back(Instruction);

				GetNextLexeme();

				if (m_CurrLexeme == GetGlobalDataInitBlockEnd())
				{
					break;
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
		HString str;

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
						GetNextLexmeAssign_HazeString(str);
						
						auto iter = m_InterSymbol.find(str);
						if (iter != m_InterSymbol.end())
						{
							classMember.Variable.Type.CustomName = &(*iter);
						}
						else
						{
							HAZE_LOG_ERR_W("解析类成员错误, 在符号表没找到对应类<%s>", str.c_str());
						}
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

				table.m_Functions[i].Type.StringStream<BackendParse>(this, 
					&BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
					&BackendParse::GetNextLexmeAssign_CustomType<uint32>);

				GetNextLexeme();
				while (m_CurrLexeme == GetFunctionParamHeader())
				{
					HazeDefineVariable param;
					GetNextLexmeAssign_HazeString(param.Name);
					param.Type.StringStream<BackendParse>(this, 
						&BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
						&BackendParse::GetNextLexmeAssign_CustomType<uint32>);

					table.m_Functions[i].Params.push_back(Move(param));

					GetNextLexeme();
				}

				while (m_CurrLexeme == GetFunctionVariableHeader())
				{
					HazeVariableData var;
					GetNextLexmeAssign_CustomType<int>(var.Offset);

					GetNextLexmeAssign_HazeString(var.Variable.Name);

					var.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
						&BackendParse::GetNextLexmeAssign_CustomType<uint32>);
					GetNextLexmeAssign_CustomType<uint32>(var.Size);
					GetNextLexmeAssign_CustomType<uint32>(var.Line);

					table.m_Functions[i].Variables.push_back(Move(var));
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

	data.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
		&BackendParse::GetNextLexmeAssign_CustomType<uint32>);

	if (data.Desc == HazeDataDesc::ArrayElement || data.Desc == HazeDataDesc::ConstantString)
	{
		GetNextLexmeAssign_CustomType<uint32>(data.Extra.Index);
	}
}

void BackendParse::ParseInstruction(ModuleUnit::FunctionInstruction& instruction)
{
	HString str;
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

		//if (IsFunctionType(operatorOne.Variable.Type.PrimaryType))
		{
			GetNextLexmeAssign_CustomType<uint32>(operatorOne.Scope);
			GetNextLexmeAssign_CustomType<uint32>(operatorOne.Desc);
		}

		if (operatorOne.Scope == HazeVariableScope::Ignore)
		{
			GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamNum);
			GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamByteSize);
		}
		else
		{
			GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamNum);
			GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamByteSize);
			operatorOne.Extra.Address.BaseAddress = 0;
			operatorOne.Extra.Address.Offset = 0;
		}


		GetNextLexmeAssign_HazeString(operatorTwo.Variable.Name);
		GetNextLexmeAssign_CustomType<uint32>(operatorTwo.Desc);

		if (operatorTwo.Desc == HazeDataDesc::CallFunctionPointer)
		{
			GetNextLexmeAssign_CustomType<void*>(operatorTwo.Extra.Pointer);
		}


		instruction.Operator = { operatorOne, operatorTwo };
	}
	break;
	case InstructionOpCode::NEW:
	{
		InstructionData operatorOne;
		GetNextLexmeAssign_HazeString(operatorOne.Variable.Name);


		operatorOne.Variable.Type.StringStream<BackendParse>(this,
			&BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
			&BackendParse::GetNextLexmeAssign_CustomType<uint32>);

		GetNextLexmeAssign_CustomType<uint32>(operatorOne.Scope);
		GetNextLexmeAssign_CustomType<uint32>(operatorOne.Desc);

		InstructionData operatorTwo;
		ParseInstructionData(operatorTwo);

		instruction.Operator = { operatorOne, operatorTwo };
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
	case InstructionOpCode::SIGN:
	{
		InstructionData operatorOne;
		GetNextLexmeAssign_CustomType<uint64>(operatorOne.Extra.SignData);

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

		newGlobalDataTable.Data.insert(newGlobalDataTable.Data.end(),
			iter.second->m_GlobalDataTable.Data.begin(), iter.second->m_GlobalDataTable.Data.end());
		newGlobalDataTable.ClassObjectAllSize += iter.second->m_GlobalDataTable.ClassObjectAllSize;
		
		int globalInstructionSize = (int)newGlobalDataTable.Instructions.size();
		for (auto i = newGlobalDataTable.Data.size() - iter.second->m_GlobalDataTable.Data.size(); i < newGlobalDataTable.Data.size(); i++)
		{
			newGlobalDataTable.Data[i].StartAddress += globalInstructionSize;
			newGlobalDataTable.Data[i].EndAddress += globalInstructionSize;
		}
		
		newGlobalDataTable.Instructions.insert(newGlobalDataTable.Instructions.end(),
			iter.second->m_GlobalDataTable.Instructions.begin(), iter.second->m_GlobalDataTable.Instructions.end());

		newStringTable.Strings.insert(newStringTable.Strings.end(), 
			iter.second->m_StringTable.Strings.begin(), iter.second->m_StringTable.Strings.end());
		newClassTable.Classes.insert(newClassTable.Classes.end(), iter.second->m_ClassTable.Classes.begin(), iter.second->m_ClassTable.Classes.end());
	}

	FindAddress(newGlobalDataTable, newFunctionTable);

	//指令总个数，在全局变量初始化和函数指令间添加一个空指令，防止Call指令执行的PC-1时指到全局变量初始化的指令上
	newGlobalDataTable.Instructions.push_back(ModuleUnit::FunctionInstruction());

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
	HashMap<HString, int>& localVariable, ModuleUnit::GlobalDataTable& newGlobalDataTable)
{
	HString objName;
	HString memberName;
	bool isPointer = false;

	if (IsClassMember(operatorData.Desc))
	{
		FindObjectAndMemberName(operatorData.Variable.Name, objName, memberName, isPointer);
		for (uint32 i = 0; i < function.Variables.size(); i++)
		{
			if (function.Variables[i].Variable.Name == objName)
			{
				auto classData = GetClass(*function.Variables[i].Variable.Type.CustomName);
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
		else if (function.Variables[0].Variable.Name.substr(0, 1) == TOKEN_THIS)
		{
			auto classData = GetClass(*function.Variables[0].Variable.Type.CustomName);
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
					function.Variables[0].Variable.Type.CustomName->c_str());
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

inline void BackendParse::ResetGlobalOperatorAddress(InstructionData& operatorData, ModuleUnit::GlobalDataTable& newGlobalDataTable)
{
	HString objName;
	HString memberName;
	bool isPointer = false;
	int index = 0;

	if (IsClassMember(operatorData.Desc))
	{
		FindObjectAndMemberName(operatorData.Variable.Name, objName, memberName, isPointer);
		index = (int)newGlobalDataTable.GetIndex(objName);
		if (index >= 0)
		{
			auto Class = GetClass(*newGlobalDataTable.Data[index].Type.CustomName);
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
		else if (NEW_REGISTER == operatorData.Variable.Name)
		{
			operatorData.AddressType = InstructionAddressType::Register;
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
	HashMap<HString, size_t> HashMap_FunctionIndexAndAddress;
	for (size_t i = 0; i < newFunctionTable.m_Functions.size(); i++)
	{
		HashMap_FunctionIndexAndAddress[newFunctionTable.m_Functions[i].Name] = i;
	}

	for (size_t i = 0; i < newGlobalDataTable.Instructions.size(); i++)
	{
		if (!IsIgnoreFindAddressInsCode(newGlobalDataTable.Instructions[i]))
		{
			HString objName;
			HString memberName;

			for (auto& operatorData : newGlobalDataTable.Instructions[i].Operator)
			{
				if (!IsIgnoreFindAddress(operatorData))
				{
					if (IS_SCOPE_GLOBAL(operatorData.Scope))
					{
						ResetGlobalOperatorAddress(operatorData, newGlobalDataTable);
					}
					else if (IS_SCOPE_TEMP(operatorData.Scope))
					{
						if (operatorData.Desc == HazeDataDesc::FunctionAddress)
						{
							operatorData.AddressType = InstructionAddressType::FunctionAddress;
						}
						else
						{
							HAZE_LOG_ERR_W("寻找全局变量<%s>的地址失败!\n", operatorData.Variable.Name.c_str());
						}
					}
					else
					{
						HAZE_LOG_ERR_W("寻找变量<%s>的地址失败!\n", operatorData.Variable.Name.c_str());
					}
				}
			}
		}
	}

	//替换变量为索引或相对函数起始偏移
	for (size_t k = 0; k < newFunctionTable.m_Functions.size(); ++k)
	{
#if BACKEND_INSTRUCTION_LOG
		std::wcout << NewFunctionTable.Vector_Function[k].Name << std::endl;
#endif
		auto& m_CurrFunction = newFunctionTable.m_Functions[k];

		HashMap<HString, int> localVariables;
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
						HAZE_LOG_ERR_W("查找调用的函数指针<%s>失败!\n"), CurrFunction.Vector_Instruction[i].Operator[0].Variable.Name.c_str());
					}
				}
			}*/
			else if (!IsIgnoreFindAddressInsCode(m_CurrFunction.Instructions[i]))
			{
				HString objName;
				HString memberName;

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
							ResetGlobalOperatorAddress(operatorData, newGlobalDataTable);
						}
						//else if (IS_SCOPE_TEMP(operatorData.Scope))
						//{
						//	/*if (operatorData.Desc == HazeDataDesc::FunctionAddress)
						//	{
						//		operatorData.AddressType = InstructionAddressType::FunctionAddress;
						//	}
						//	else*/
						//	{
						//		operatorData.AddressType = InstructionAddressType::Register;
						//		HAZE_LOG_ERR_W("寻找临时变量<%s>的地址失败!\n", operatorData.Variable.Name.c_str());
						//	}
						//}
						else if ((IS_SCOPE_IGNORE(operatorData.Scope) || IS_SCOPE_TEMP(operatorData.Scope)) && operatorData.Desc == HazeDataDesc::FunctionAddress)
						{
							operatorData.AddressType = InstructionAddressType::FunctionAddress;
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

const ModuleUnit::ClassTableData* const BackendParse::GetClass(const HString& className)
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

uint32 const BackendParse::GetClassSize(const HString& className)
{
	auto classData = GetClass(className);
	if (classData)
	{
		return classData->Size;
	}
	return 0;
}

uint32 BackendParse::GetMemberOffset(const ModuleUnit::ClassTableData& classData, const HString& memberName)
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