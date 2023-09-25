#include "HazeDebugger.h"
#include "HazeLog.h"
#include "HazeVM.h"
#include "HazeStack.h"

#include "HazeDebuggerServer.h"

#define ENABLE_DEBUGGER_LOG 0

static HAZE_STRING GetFileName(const HAZE_CHAR*& Msg)
{
	bool IsNewLine = false;
	HAZE_STRING FileName;
	FileName.clear();
	while (!HazeIsSpace(*Msg, &IsNewLine))
	{
		FileName += *Msg++;
	}

	Msg++;

	return FileName;
}

static  HAZE_STRING GetFileModuleName(const HAZE_CHAR*& FilePath)
{
	return GetModuleNameByFilePath(GetFileName(FilePath));
}

void HookCall(HazeVM* m_VM)
{
}

void GetHazeValueByBaseType(open::OpenJson& Json, const char* Address, HazeValueType m_Type)
{
	switch (m_Type)
	{
	case HazeValueType::Bool:
	{
		bool Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::Byte:
	{
		hbyte Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::UnsignedByte:
	{
		uhbyte Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::Char:
	{
		char Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::Short:
	{
		short Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::UnsignedShort:
	{
		ushort Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::Int:
	{
		int Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::Float:
	{
		float Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::Long:
	{
		int64 Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::Double:
	{
		double Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::UnsignedInt:
	{
		uint32 Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	case HazeValueType::UnsignedLong:
	{
		uint64 Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	break;
	default:
		HAZE_LOG_ERR_W("Debug 获得基础类型<%d>数据错误!\n", (uint32)m_Type);
		break;
	}
}

HazeDebugger::HazeDebugger(HazeVM* m_VM, void(*EndCall)()) : m_VM(m_VM), EndCall(EndCall), HookFunctionCall(&HookCall), HookType(DebuggerHookType::Line),
IsPause(true)
{
	HashMap_BreakPoint.clear();
	HashMap_TempBreakPoint.clear();
	HashMap_IsStepOver.clear();
}

HazeDebugger::~HazeDebugger()
{
	if (EndCall)
	{
		EndCall();
	}
}

void HazeDebugger::SetHook(void(*HookCall)(HazeVM* m_VM), uint32 m_Type)
{
	HookFunctionCall = HookCall;
	HookType = m_Type;
}

void HazeDebugger::AddBreakPoint(const char* Message)
{
	auto Msg = String2WString(Message);
	auto HazeChar = Msg.c_str();

	auto FileName = GetFileName(HazeChar);
	auto m_ModuleName = GetModuleNameByFilePath(FileName);
	uint32 Line = StringToStandardType<uint32>(HAZE_STRING(HazeChar));

	auto Iter = HashMap_BreakPoint.find(m_ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		Iter->second.first.insert(Line);
	}
	else
	{
		HashMap_BreakPoint[m_ModuleName] = { { Line }, FileName };
	}

#if ENABLE_DEBUGGER_LOG

	HAZE_LOG_INFO(HAZE_TEXT("添加断点<%s><%s><%d>\n"), ModuleName.c_str(), FileName.c_str(), Line);

#endif
}

void HazeDebugger::DeleteBreakPoint(const char* Message)
{
	auto MsgString = String2WString(Message);
	auto Msg = MsgString.c_str();

	auto FileName = GetFileName(Msg);
	uint32 Line = StringToStandardType<uint32>(Msg);

	auto m_ModuleName = GetModuleNameByFilePath(FileName);
	auto Iter = HashMap_BreakPoint.find(FileName);
	if (Iter != HashMap_BreakPoint.end())
	{
		auto It = Iter->second.first.find(Line);
		if (It != Iter->second.first.end())
		{
			Iter->second.first.erase(It);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("删除断点错误,在模块<%s>未能找到<%d>行!\n"), FileName.c_str(), Line);
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("删除断点错误,未能找到模块<%s>!\n"), FileName.c_str());
	}
}

void HazeDebugger::DeleteAllBreakPoint()
{
	HashMap_BreakPoint.clear();
}

void HazeDebugger::DeleteModuleAllBreakPoint(const char* Message)
{
	auto Msgs = String2WString(Message);
	auto m_ModuleName = GetModuleNameByFilePath(Msgs);

	auto Iter = HashMap_BreakPoint.find(m_ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		Iter->second.first.clear();
	}

	HAZE_LOG_ERR(HAZE_TEXT("清除模块<%s>的所有断点!\n"), m_ModuleName.c_str());
}

void HazeDebugger::OnExecLine(uint32 Line)
{
#if ENABLE_DEBUGGER_LOG

	HAZE_LOG_INFO(HAZE_TEXT("运行到<%d>行!\n"), Line);

#endif // ENABLE_DEBUGGER_LOG

	auto m_ModuleName = m_VM->GetModuleNameByCurrFunction();
	auto Iter = HashMap_BreakPoint.find(*m_ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		auto It = Iter->second.first.find(Line);
		if (It != Iter->second.first.end())
		{
			if (HookType & DebuggerHookType::Line)
			{
				if (HookFunctionCall)
				{
					HookFunctionCall(m_VM);
				}

				CurrPauseModule = { Iter->first, Line };
				IsPause = true;
			}
		}
	}

	if (!IsPause && CurrModuleIsStepOver())
	{
		if (Line == CurrPauseModule.second && *m_ModuleName == CurrPauseModule.first)
		{
			IsPause = true;
		}
		else
		{
			auto TempIter = HashMap_TempBreakPoint.find(*m_ModuleName);
			if (TempIter != HashMap_TempBreakPoint.end())
			{
				auto TempIt = TempIter->second.first.find(Line);
				if (TempIt != TempIter->second.first.end())
				{
					if (HookType & DebuggerHookType::Line)
					{
						CurrPauseModule = { TempIter->first, Line };
						IsPause = true;

						TempIter->second.first.erase(TempIt);
					}
				}
			}
		}
	}

	/*if (IsStepOver && Line != CurrPauseModule.second)
	{
		IsPause = true;
		CurrPauseModule.second = Line;
	}*/

	if (IsPause)
	{
#if ENABLE_DEBUGGER_LOG

		HAZE_LOG_INFO(HAZE_TEXT("调试器暂停<%s><%d>!\n"), CurrPauseModule.first.c_str(), Line);

#endif

		SendBreakInfo();
	}
}

void HazeDebugger::Start()
{
	IsPause = false;
}

void HazeDebugger::StepOver()
{
	HashMap_IsStepOver[*m_VM->GetModuleNameByCurrFunction()] = true;

	if (IsPause)
	{
		IsPause = false;
		CurrPauseModule.second = m_VM->GetNextLine(CurrPauseModule.second);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("单步调试错误,不是暂停状态!\n"));
	}
}

void HazeDebugger::StepIn()
{
}

void HazeDebugger::StepInstruction()
{
}

void HazeDebugger::Continue()
{
	ClearCurrParseModuleData();
}

void HazeDebugger::SetJsonLocalVariable(open::OpenJson& Json)
{
	auto& Info = Json["LocalVariable"];

	auto& Frame = m_VM->VMStack->GetCurrFrame();
	if (Frame.FunctionInfo)
	{
		for (size_t i = 0; i < Frame.FunctionInfo->Variables.size(); i++)
		{
			if (Frame.FunctionInfo->Variables[i].Line < CurrPauseModule.second)
			{
				SetJsonVariableData(Info[i], Frame.FunctionInfo->Variables[i]);
			}
		}
	}

	if (Info.empty())
	{
		Info = "";
	}
}

void HazeDebugger::SetJsonModuleGlobalVariable(open::OpenJson& Json)
{
	auto& Info = Json["GlobalVariable"];

	auto& Frame = m_VM->VMStack->GetCurrFrame();
	if (Frame.FunctionInfo)
	{
		for (size_t i = 0; i < Frame.FunctionInfo->Variables.size(); i++)
		{
			SetJsonVariableData(Info[i], Frame.FunctionInfo->Variables[i]);
		}
	}
}

void HazeDebugger::AddTempBreakPoint(uint32 Line)
{
	if (CurrModuleIsStepOver())
	{
		auto m_ModuleName = m_VM->GetModuleNameByCurrFunction();
		HashMap_TempBreakPoint[*m_ModuleName].first.insert(Line);
	}
}

void HazeDebugger::SendProgramEnd()
{
	open::OpenJson json;
	json["Type"] = (int)HazeDebugInfoType::ProgramEnd;
	auto& data = json.encode();
	HazeDebuggerServer::SendData(const_cast<char*>(data.data()), data.length());
}

bool HazeDebugger::CurrModuleIsStepOver()
{
	auto m_ModuleName = m_VM->GetModuleNameByCurrFunction();
	auto Iter = HashMap_IsStepOver.find(*m_ModuleName);
	if (Iter != HashMap_IsStepOver.end())
	{
		return Iter->second;
	}

	return false;
}

void HazeDebugger::SendBreakInfo()
{
	auto Iter = HashMap_BreakPoint.find(CurrPauseModule.first);
	if (Iter != HashMap_BreakPoint.end())
	{
		open::OpenJson Json;
		SetJsonType(Json, HazeDebugInfoType::BreakInfo);
		SetJsonBreakFilePath(Json, Iter->second.second);
		SetJsonBreakLine(Json, CurrPauseModule.second);
		SetJsonLocalVariable(Json);
		//SetJsonModuleGlobalVariable(Json);

		auto& data = Json.encode();
		HazeDebuggerServer::SendData(const_cast<char*>(data.data()), data.length());
	}
}

void HazeDebugger::SetJsonVariableData(open::OpenJson& Json, const HazeVariableData& Variable, const char* Address, bool isStack)
{
	static std::string String;

	String = WString2String(Variable.Variable.m_Name);
	Json["Name"] = GB2312_2_UFT8(String.c_str());

	auto DataAddress = isStack ? m_VM->VMStack->GetAddressByEBP(Variable.Offset) : Address;
	if (Address)
	{
		DataAddress += Variable.Offset;
	}

	if (IsClassType(Variable.Variable.m_Type.PrimaryType))
	{
		auto Class = m_VM->FindClass(Variable.Variable.m_Type.CustomName);
		String = WString2String(Variable.Variable.m_Type.CustomName);
		Json["Type"] = GB2312_2_UFT8(String.c_str());

		for (size_t j = 0; j < Class->Members.size(); j++)
		{
			SetJsonVariableData(Json["Value"][j], Class->Members[j], DataAddress);
		}
	}
	else if (IsPointerType(Variable.Variable.m_Type.PrimaryType))
	{
		String = WString2String(HAZE_TEXT("指针"));
		Json["Type"] = GB2312_2_UFT8(String.c_str());

		uint64 Value;
		memcpy(&Value, DataAddress, sizeof(Value));
		Json["Value"] = ToString((void*)Value);

		if (Variable.Variable.m_Type.NeedCustomName())
		{
			auto Class = m_VM->FindClass(Variable.Variable.m_Type.CustomName);
			String = WString2String(Variable.Variable.m_Type.CustomName);
			Json["TypeClass"] = GB2312_2_UFT8(String.c_str());

			for (size_t j = 0; j < Class->Members.size(); j++)
			{
				SetJsonVariableData(Json["PointerValue"][j], Class->Members[j], (const char*)Value, false);
			}
		}

		if (Variable.Variable.m_Type.NeedSecondaryType())
		{
			String = WString2String(GetHazeValueTypeString(Variable.Variable.m_Type.SecondaryType));
			Json["SubType"] = GB2312_2_UFT8(String.c_str());
		}
	}
	else if (IsArrayType(Variable.Variable.m_Type.PrimaryType))
	{
		String = WString2String(GetHazeValueTypeString(Variable.Variable.m_Type.PrimaryType));
		Json["Type"] = GB2312_2_UFT8(String.c_str());
		
		String = WString2String(GetHazeValueTypeString(Variable.Variable.m_Type.SecondaryType));
		Json["SubType"] = GB2312_2_UFT8(String.c_str());

		int size = GetSizeByType(Variable.Variable.m_Type, m_VM);

		if (Variable.Variable.m_Type.CustomName.empty())
		{
			for (int i = 0; i < Variable.Size / size; i++)
			{
				GetHazeValueByBaseType(Json["Value"][i], DataAddress + i * size, Variable.Variable.m_Type.SecondaryType);
			}

		}
		else
		{
			for (int i = 0; i < Variable.Size / size; i++)
			{
				SetJsonVariableData(Json["Value"][i], Variable, DataAddress + size * i);
			}
		}
	}
	else if (IsReferenceType(Variable.Variable.m_Type.PrimaryType))
	{
		String = WString2String(GetHazeValueTypeString(Variable.Variable.m_Type.PrimaryType));
		Json["Type"] = GB2312_2_UFT8(String.c_str());

		uint64 Value;
		memcpy(&Value, Address, sizeof(Value));
		Json["Value"] = Value;
	}
	else
	{
		String = WString2String(GetHazeValueTypeString(Variable.Variable.m_Type.PrimaryType));
		Json["Type"] = GB2312_2_UFT8(String.c_str());
		GetHazeValueByBaseType(Json["Value"], DataAddress, Variable.Variable.m_Type.PrimaryType);
	}
}