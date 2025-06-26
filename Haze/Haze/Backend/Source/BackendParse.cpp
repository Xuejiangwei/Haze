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

static bool IsIgnoreFindAddressInsCode(ModuleUnit::FunctionInstruction& ins)
{
	if (ins.InsCode == InstructionOpCode::LINE || ins.InsCode == InstructionOpCode::NEW_SIGN)
	{
		return true;
	}

	if (ins.InsCode == InstructionOpCode::CALL && ins.Operator[0].Variable.Type.PrimaryType == HazeValueType::ObjectFunction)
	{
		return true;
	}

	if (ins.InsCode == InstructionOpCode::CALL && ins.Operator[0].Variable.Type.PrimaryType == HazeValueType::Function && 
		(ins.Operator[0].Desc == HazeDataDesc::RegisterTemp || ins.Operator[0].Scope == HazeVariableScope::Ignore))
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

	if (IsRegisterDesc(operatorData.Desc) && operatorData.Desc != HazeDataDesc::RegisterTemp)
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
		codeText = HString(std::istreambuf_iterator<x_HChar>(fs), {});
		m_CurrCode = codeText.c_str();
		Parse_I_Symbol();
		fs.close();
	}

	for (auto& refModule : refModules)
	{
		m_CurrParseModule = MakeShare<ModuleUnit>(refModule);
		m_Modules[refModule] = m_CurrParseModule;

		HAZE_IFSTREAM fs(GetIntermediateModuleFile(refModule));
		fs.imbue(std::locale("chs"));

		HString Content(std::istreambuf_iterator<x_HChar>(fs), {});

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
	GetNextLexeme();
	if (m_CurrLexeme == GetSymbolBeginHeader())
	{
		GetNextLexeme();
		while (m_CurrLexeme != GetSymbolEndHeader())
		{
			m_InterSymbol.insert(m_CurrLexeme);
			GetNextLexeme();
		}
	}
}

void BackendParse::Parse_I_Code()
{
	x_uint64 timestamp;
	GetNextLexmeAssign_CustomType<x_uint32>(timestamp);

	//暂时用来读取版本号
	GetNextLexmeAssign_HazeString(m_CurrParseModule->m_Path);

	//Standard lib
	GetNextLexmeAssign_HazeString(m_CurrParseModule->m_Path);
	GetNextLexmeAssign_CustomType<x_uint32>(m_CurrParseModule->m_LibraryType);

	GetNextLexeme();
	Parse_I_Code_ImportTable();

	//Global data
	GetNextLexeme();
	Parse_I_Code_GlobalTable();

	//String table
	GetNextLexeme();
	Parse_I_Code_StringTable();

	//Enum table
	GetNextLexeme();
	Parse_I_Code_EnumTable();

	//Class table
	GetNextLexeme();
	Parse_I_Code_ClassTable();

	//Function table
	GetNextLexeme();
	Parse_I_Code_FunctionTable();
}

void BackendParse::Parse_I_Code_ImportModule()
{
	HString str;
	if (m_CurrLexeme == GetImportHeaderString())
	{
		GetNextLexeme();
		x_uint32 number = StringToStandardType<x_uint32>(m_CurrLexeme);

		for (x_uint64 i = 0; i < number; i++)
		{
			GetNextLexeme();
			GetNextLexeme();
		}
	}
}

void BackendParse::Parse_I_Code_ImportTable()
{
	HString str;
	if (m_CurrLexeme == GetImportHeaderString())
	{
		x_uint64 count;
		GetNextLexmeAssign_StandardType(count);
		for (x_uint64 i = 0; i < count; i++)
		{
			GetNextLexeme();
			if (m_CurrLexeme == GetImportHeaderModuleString())
			{
				GetNextLexmeAssign_HazeString(str);
			}
			else
			{
				HAZE_LOG_ERR_W("后端解析失败,引用Label错误<%s>!\n", m_CurrLexeme.c_str());
				return;
			}
		}
	}
}

void BackendParse::Parse_I_Code_GlobalTable()
{
	HString str;
	if (m_CurrLexeme == GetGlobalDataHeaderString())
	{
		GetNextLexeme();
		x_uint32 number = StringToStandardType<x_uint32>(m_CurrLexeme);

		ModuleUnit::GlobalDataTable& table = m_CurrParseModule->m_GlobalDataTable;
		table.Data.resize(number);

		for (x_uint64 i = 0; i < table.Data.size(); i++)
		{
			//因为将所有的指令合在一起了，需要重新计算
			/*GetNextLexmeAssign_CustomType<uint32>(table.Data[i].StartAddress);
			GetNextLexmeAssign_CustomType<uint32>(table.Data[i].EndAddress);*/

			GetNextLexmeAssign_HazeString(table.Data[i].Name);

			//GetNextLexmeAssign_StandardType(table.Data[i].Size);

			table.Data[i].Type.StringStream<BackendParse>(this,
				&BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
				&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);
		}

		/*GetNextLexeme();
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
		}*/
	}

}

void BackendParse::Parse_I_Code_StringTable()
{
	if (m_CurrLexeme == GetStringTableHeaderString())
	{
		x_uint32 number;
		GetNextLexmeAssign_StandardType(number);

		ModuleUnit::StringTable& table = m_CurrParseModule->m_StringTable;
		table.Strings.resize(number);

		ParseStringCount.first = true;
		for (x_uint64 i = 0; i < table.Strings.size(); i++)
		{
			GetNextLexmeAssign_StandardType(ParseStringCount.second);
			GetNextLexmeAssign_HazeString(table.Strings[i].String);
		}
		ParseStringCount.first = false;
	}
}

void BackendParse::Parse_I_Code_EnumTable()
{
	if (m_CurrLexeme == GetEnumTableLabelHeader())
	{
		x_uint32 number;
		GetNextLexmeAssign_StandardType(number);

		//暂不记录
		HString str;
		x_uint32 v;
		for (x_uint32 i = 0; i < number; i++)
		{
			GetNextLexmeAssign_HazeString(str);
			if (str == GetEnumStartHeader())
			{
				GetNextLexmeAssign_HazeString(str);
				while (str != GetEnumEndHeader())
				{
					GetNextLexmeAssign_StandardType(v);
					GetNextLexmeAssign_HazeString(str);
				}
			}
		}
	}
}

void BackendParse::Parse_I_Code_ClassTable()
{
	if (m_CurrLexeme == GetClassTableHeaderString())
	{
		x_uint32 number;
		HString str;

		GetNextLexmeAssign_StandardType(number);

		ModuleUnit::ClassTable& table = m_CurrParseModule->m_ClassTable;
		table.Classes.resize(number);

		for (x_uint64 i = 0; i < table.Classes.size(); i++)
		{
			GetNextLexeme();

			if (m_CurrLexeme == GetClassLabelHeader())
			{
				auto& classData = table.Classes[i];

				GetNextLexmeAssign_HazeString(classData.Name);
				GetNextLexmeAssign_StandardType(classData.Size);

				GetNextLexmeAssign_StandardType(number);
				classData.ParentClasses.resize(number);
				for (x_uint64 j = 0; j < classData.ParentClasses.size(); j++)
				{
					GetNextLexmeAssign_HazeString(classData.ParentClasses[j]);
				}

				GetNextLexmeAssign_StandardType(number);
				classData.Members.resize(number);

				for (x_uint64 j = 0; j < classData.Members.size(); j++)
				{
					auto& classMember = classData.Members[j];

					GetNextLexmeAssign_HazeString(classMember.Variable.Name);

					//HazeDesc
					GetNextLexmeAssign_StandardType(number);
					
					classMember.Variable.Type.StringStream<BackendParse>(this,
						&BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
						&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);

					GetNextLexmeAssign_CustomType<x_uint32>(classMember.Offset);
					GetNextLexmeAssign_CustomType<x_uint32>(classMember.Size);
				}
			}
		}
	}
}

void BackendParse::Parse_I_Code_FunctionTable()
{
	if (m_CurrLexeme == GetFucntionTableHeaderString())
	{
		x_uint32 number;
		x_uint64 count = 0;
		GetNextLexmeAssign_StandardType(number);
		count += number;
		
		GetNextLexmeAssign_StandardType(number);
		count += number;

		m_CurrParseModule->m_FunctionTable.m_Functions.resize(count);

		for (x_uint64 i = 0; i < count; i++)
		{
			ModuleUnit::FunctionTableData& functionData = m_CurrParseModule->m_FunctionTable.m_Functions[i];

			GetNextLexeme();

			if (m_CurrLexeme == GetFunctionLabelHeader() || m_CurrLexeme == GetClassFunctionLabelHeader())
			{
				bool isNormalFunc = m_CurrLexeme == GetFunctionLabelHeader();
				GetNextLexmeAssign_CustomType<int>(functionData.DescType);
				if (isNormalFunc)
				{
					functionData.ClassName = H_TEXT("None");
				}
				else
				{
					GetNextLexmeAssign_HazeString(functionData.ClassName);
					GetNextLexmeAssign_CustomType<x_uint32>(number);
				}
				GetNextLexmeAssign_HazeString(functionData.Name);

				functionData.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
					&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);

				GetNextLexeme();
				while (m_CurrLexeme == GetFunctionParamHeader())
				{
					HazeDefineVariable param;
					GetNextLexmeAssign_HazeString(param.Name);
					param.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
						&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);

					functionData.Params.push_back(Move(param));
					GetNextLexeme();
				}

				while (m_CurrLexeme == GetFunctionVariableHeader())
				{
					HazeVariableData var;
					GetNextLexmeAssign_CustomType<int>(var.Offset);

					GetNextLexmeAssign_HazeString(var.Variable.Name);

					var.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
						&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);
					GetNextLexmeAssign_CustomType<x_uint32>(var.Size);
					GetNextLexmeAssign_CustomType<x_uint32>(var.Line);

					functionData.Variables.push_back(Move(var));
					GetNextLexeme();
				}
				
				while (m_CurrLexeme == GetFunctionTempRegisterHeader())
				{
					HazeTempRegisterData data;

					GetNextLexmeAssign_HazeString(data.Name);
					BackendParse::GetNextLexmeAssign_CustomType<x_uint32>(data.Offset);
					data.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
						&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);

					functionData.TempRegisters.push_back(Move(data));
					GetNextLexeme();
				}

				while (m_CurrLexeme == GetClosureRefrenceVariableHeader())
				{
					int refIndex = -1;
					int varIndex = -1;
					GetNextLexmeAssign_CustomType<int>(refIndex);
					GetNextLexmeAssign_CustomType<int>(varIndex);
					functionData.RefVariable.push_back({ refIndex, varIndex });
					
					GetNextLexeme();
				}

				if (m_CurrLexeme == GetFunctionStartHeader())
				{
					GetNextLexmeAssign_CustomType<int>(functionData.StartLine);

					GetNextLexeme();
					while (m_CurrLexeme == BLOCK_START)
					{
						GetNextLexeme();
						functionData.Blocks.push_back({ m_CurrLexeme, 0, (int)functionData.Instructions.size() });
						auto& CurrBlock = functionData.Blocks.back();

						GetNextLexeme();
						while (m_CurrLexeme != BLOCK_START && m_CurrLexeme != GetFunctionEndHeader())
						{
							ModuleUnit::FunctionInstruction Instruction;
							Instruction.InsCode = GetInstructionByString(m_CurrLexeme);

							ParseInstruction(Instruction);

							functionData.Instructions.push_back(Instruction);

							CurrBlock.InstructionNum++;

							GetNextLexeme();
						}

						if (m_CurrLexeme == GetFunctionEndHeader())
						{
							GetNextLexmeAssign_CustomType<int>(functionData.EndLine);
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
	GetNextLexmeAssign_CustomType<x_uint32>(data.Scope);
	GetNextLexmeAssign_CustomType<x_uint32>(data.Desc);

	data.Variable.Type.StringStream<BackendParse>(this, &BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
		&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);

	if (data.Desc == HazeDataDesc::ConstantString)
	{
		GetNextLexmeAssign_CustomType<x_uint32>(data.Extra.Index);
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
		case InstructionOpCode::NEG:
		case InstructionOpCode::NOT:
		case InstructionOpCode::CMP:
		case InstructionOpCode::BIT_NEG:
		case InstructionOpCode::NEW:
		case InstructionOpCode::CVT:
		case InstructionOpCode::MOV_DCU:
		{
			InstructionData operatorOne;
			InstructionData operatorTwo;

			ParseInstructionData(operatorOne);
			ParseInstructionData(operatorTwo);

			instruction.Operator = { operatorOne, operatorTwo };
		}
			break;
		case InstructionOpCode::ADD:
		case InstructionOpCode::SUB:
		case InstructionOpCode::MUL:
		case InstructionOpCode::DIV:
		case InstructionOpCode::MOD:
		case InstructionOpCode::BIT_AND:
		case InstructionOpCode::BIT_OR:
		case InstructionOpCode::BIT_XOR:
		case InstructionOpCode::SHL:
		case InstructionOpCode::SHR:
		{
			InstructionData operatorOne;
			InstructionData operatorTwo;
			InstructionData operatorThree;

			ParseInstructionData(operatorOne);
			ParseInstructionData(operatorTwo);
			ParseInstructionData(operatorThree);

			instruction.Operator = { operatorOne, operatorTwo, operatorThree };
		}
			break;
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
			GetNextLexmeAssign_CustomType<x_uint32>(operatorOne.Variable.Type.PrimaryType);

			//if (IsFunctionType(operatorOne.Variable.Type.PrimaryType))
			{
				GetNextLexmeAssign_CustomType<x_uint32>(operatorOne.Scope);
				GetNextLexmeAssign_CustomType<x_uint32>(operatorOne.Desc);
		
				GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamNum);
				GetNextLexmeAssign_CustomType<int>(operatorOne.Extra.Call.ParamByteSize);
			}

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
			GetNextLexmeAssign_CustomType<x_uint32>(operatorOne.Extra.Line);

			instruction.Operator = { operatorOne };
		}
			break;
		case InstructionOpCode::NEW_SIGN:
		{
			InstructionData operatorOne;
			operatorOne.Variable.Type.StringStream<BackendParse>(this,
				&BackendParse::GetNextLexmeAssign_HazeStringCustomClassName,
				&BackendParse::GetNextLexmeAssign_CustomType<x_uint32>);

			BackendParse::GetNextLexmeAssign_CustomType<x_uint32>(operatorOne.Extra.SignData.ArrayDimension);
			
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

	exeFile.WriteModule(m_Modules);
	ModuleUnit::GlobalDataTable newGlobalDataTable;
	ModuleUnit::StringTable newStringTable;
	ModuleUnit::ClassTable newClassTable;
	ModuleUnit::FunctionTable newFunctionTable;

	x_uint64 functionCount = 0;
	for (auto& iter : m_Modules)
	{
		newGlobalDataTable.InitFunctionIndex.push_back(newFunctionTable.m_Functions.size());
		newFunctionTable.m_Functions.insert(newFunctionTable.m_Functions.end(), 
			iter.second->m_FunctionTable.m_Functions.begin(), iter.second->m_FunctionTable.m_Functions.end());

		ReplaceStringIndex(newStringTable, newFunctionTable, functionCount);

		newGlobalDataTable.Data.insert(newGlobalDataTable.Data.end(),
			iter.second->m_GlobalDataTable.Data.begin(), iter.second->m_GlobalDataTable.Data.end());

		newStringTable.Strings.insert(newStringTable.Strings.end(), 
			iter.second->m_StringTable.Strings.begin(), iter.second->m_StringTable.Strings.end());

		newClassTable.Classes.insert(newClassTable.Classes.end(), 
			iter.second->m_ClassTable.Classes.begin(), iter.second->m_ClassTable.Classes.end());
		for (x_uint64 i = newClassTable.Classes.size() - iter.second->m_ClassTable.Classes.size(); i < newClassTable.Classes.size(); i++)
		{
			newClassTable.IndexMap[newClassTable.Classes[i].Name] = (x_uint32)i;
		}
	}

	FindAddress(newGlobalDataTable, newFunctionTable);
	exeFile.WriteExecuteFile(newGlobalDataTable, newStringTable, newClassTable, newFunctionTable);
}

void BackendParse::ReplaceStringIndex(ModuleUnit::StringTable& newStringTable,
	ModuleUnit::FunctionTable& newFunctionTable, x_uint64& functionCount)
{
	for (; functionCount < newFunctionTable.m_Functions.size(); functionCount++)
	{
		for (auto& iter : newFunctionTable.m_Functions[functionCount].Instructions)
		{
			for (auto& oper : iter.Operator)
			{
				if (oper.Desc == HazeDataDesc::ConstantString)
				{
					oper.Extra.Index += (x_uint32)newStringTable.Strings.size();
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
		for (x_uint64 i = 0; i < function.Blocks.size(); i++)
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
	HashMap<HString, int>& localVariable, HashMap<HString, int> tempRegister, ModuleUnit::GlobalDataTable& newGlobalDataTable)
{
	{
		auto iterIndex = localVariable.find(operatorData.Variable.Name);
		if (iterIndex != localVariable.end())
		{
			operatorData.Extra.Address.BaseAddress = function.Variables[iterIndex->second].Offset;
			operatorData.AddressType = InstructionAddressType::Local;
		}
		else
		{
			auto tempRegIter = tempRegister.find(operatorData.Variable.Name);
			if (tempRegIter != tempRegister.end())
			{
				operatorData.Extra.Address.BaseAddress = function.TempRegisters[tempRegIter->second].Offset;
				operatorData.AddressType = InstructionAddressType::Local;
			}
			else if (IsPureStringType(operatorData.Variable.Type.PrimaryType))
			{
				operatorData.AddressType = InstructionAddressType::PureString;
			}
			else
			{
				HAZE_LOG_ERR_W("在函数<%s>中查找<%s>的偏移值失败!\n", function.Name.c_str(),
					operatorData.Variable.Name.c_str());
				return;
			}
		}
	}
}

inline void BackendParse::ResetGlobalOperatorAddress(InstructionData& operatorData, ModuleUnit::GlobalDataTable& newGlobalDataTable)
{
	int index = (int)newGlobalDataTable.GetIndex(operatorData.Variable.Name);
	if (index >= 0)
	{
		operatorData.Extra.Index = index;
		operatorData.AddressType = InstructionAddressType::Global;
	}
	/*else if (NEW_REGISTER == operatorData.Variable.Name)
	{
		operatorData.AddressType = InstructionAddressType::Register;
	}*/
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

void BackendParse::FindAddress(ModuleUnit::GlobalDataTable& newGlobalDataTable,
	ModuleUnit::FunctionTable& newFunctionTable)
{
	HashMap<HString, x_uint64> HashMap_FunctionIndexAndAddress;
	for (x_uint64 i = 0; i < newFunctionTable.m_Functions.size(); i++)
	{
		HashMap_FunctionIndexAndAddress[newFunctionTable.m_Functions[i].Name] = i;
	}

	//替换变量为索引或相对函数起始偏移
	for (x_uint64 k = 0; k < newFunctionTable.m_Functions.size(); ++k)
	{
#if BACKEND_INSTRUCTION_LOG
		std::wcout << NewFunctionTable.Vector_Function[k].Name << HAZE_ENDL;
#endif
		auto& m_CurrFunction = newFunctionTable.m_Functions[k];

		HashMap<HString, int> localVariables;
		for (x_uint64 i = 0; i < m_CurrFunction.Variables.size(); i++)
		{
			localVariables[m_CurrFunction.Variables[i].Variable.Name] = (int)i;
		}

		HashMap<HString, int> tempRegisters;
		for (x_uint64 i = 0; i < m_CurrFunction.TempRegisters.size(); i++)
		{
			tempRegisters[m_CurrFunction.TempRegisters[i].Name] = (int)i;
		}

		for (x_uint64 i = 0; i < m_CurrFunction.Instructions.size(); ++i)
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
							ResetLocalOperatorAddress(operatorData, m_CurrFunction, localVariables, tempRegisters, newGlobalDataTable);
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
						else if (IS_SCOPE_IGNORE(operatorData.Scope))
						{
							if (operatorData.Desc == HazeDataDesc::FunctionAddress)
							{
								operatorData.AddressType = InstructionAddressType::FunctionAddress;
							}
							else if (operatorData.Desc == HazeDataDesc::FunctionDynamicAddress)
							{
								operatorData.AddressType = InstructionAddressType::FunctionDynamicAddress;
							}
							/*else if (operatorData.Desc == HazeDataDesc::FunctionObjectAddress)
							{
								operatorData.AddressType = InstructionAddressType::FunctionObjectAddress;
							}*/
							else
							{
								HAZE_LOG_ERR_W("寻找忽略变量<%s>的地址失败!\n", operatorData.Variable.Name.c_str());
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
			WSS << HAZE_ENDL;
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
			if (classData.Name == className)
			{
				return &classData;
			}
		}
	}

	return nullptr;
}

x_uint32 const BackendParse::GetClassSize(const HString& className)
{
	auto classData = GetClass(className);
	if (classData)
	{
		return classData->Size;
	}
	return 0;
}

x_uint32 BackendParse::GetMemberOffset(const ModuleUnit::ClassTableData& classData, const HString& memberName)
{
	for (auto& iter : classData.Members)
	{
		if (iter.Variable.Name == memberName)
		{
			return iter.Offset;
		}
	}

	return (x_uint32)-1;
}