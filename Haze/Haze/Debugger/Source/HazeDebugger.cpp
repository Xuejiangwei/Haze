#include "HazeDebugger.h"
#include "HazeLog.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeFilePathHelper.h"
#include "HazeDebuggerServer.h"

#define ENABLE_DEBUGGER_LOG 1

static HAZE_STRING GetFileName(const HAZE_CHAR*& msg)
{
	bool IsNewLine = false;
	HAZE_STRING FileName;
	FileName.clear();
	while (!HazeIsSpace(*msg, &IsNewLine))
	{
		FileName += *msg++;
	}

	msg++;

	return FileName;
}

//static  HAZE_STRING GetFileModuleName(const HAZE_CHAR*& filePath)
//{
//	return GetModuleNameByFilePath(GetFileName(filePath));
//}

void HookCall(HazeVM* vm)
{
}

void GetHazeValueByBaseType(XJson& json, const char* address, HazeValueType type)
{
	switch (type)
	{
	case HazeValueType::Bool:
	{
		bool Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::Byte:
	{
		hbyte Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::UnsignedByte:
	{
		uhbyte Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::Char:
	{
		char Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::Short:
	{
		short Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::UnsignedShort:
	{
		ushort Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::Int:
	{
		int Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::Float:
	{
		float Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::Long:
	{
		int64 Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::Double:
	{
		double Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::UnsignedInt:
	{
		uint32 Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	case HazeValueType::UnsignedLong:
	{
		uint64 Value;
		memcpy(&Value, address, sizeof(Value));
		json = Value;
	}
	break;
	default:
		HAZE_LOG_ERR_W("Debug 获得基础类型<%d>数据错误!\n", (uint32)type);
		break;
	}
}

HazeDebugger::HazeDebugger(HazeVM* vm, void(*endCall)()) 
	: m_VM(vm), m_EndCall(endCall), m_HookFunctionCall(&HookCall), m_HookType((uint32)DebuggerHookType::Line), m_IsPause(true),
	m_IsStepIn(false), m_IsStepInInstruction(false)
{
	m_BreakPoints.clear();
	m_TempBreakPoints.clear();
	m_IsStepOvers.clear();
}

HazeDebugger::~HazeDebugger()
{
	if (m_EndCall)
	{
		m_EndCall();
	}
}

void HazeDebugger::SetHook(void(*HookCall)(HazeVM* vm), uint32 type)
{
	m_HookFunctionCall = HookCall;
	m_HookType = type;
}

void HazeDebugger::AddBreakPoint(const char* message)
{
	auto msg = String2WString(message);
	auto hazeChar = msg.c_str();

	auto fileName = GetFileName(hazeChar);
	auto moduleName = GetModuleNameByFilePath(fileName);
	uint32 Line = StringToStandardType<uint32>(HAZE_STRING(hazeChar));

	auto iter = m_BreakPoints.find(moduleName);
	if (iter != m_BreakPoints.end())
	{
		iter->second.first.insert(Line);
	}
	else
	{
		m_BreakPoints[moduleName] = { { Line }, fileName };
	}

#if ENABLE_DEBUGGER_LOG

	HAZE_LOG_INFO(HAZE_TEXT("添加断点<%s><%s><%d>\n"), moduleName.c_str(), fileName.c_str(), Line);

#endif
}

void HazeDebugger::DeleteBreakPoint(const char* message)
{
	auto msgString = String2WString(message);
	auto msg = msgString.c_str();

	auto fileName = GetFileName(msg);
	uint32 line = StringToStandardType<uint32>(msg);

	auto moduleName = GetModuleNameByFilePath(fileName);
	auto iter = m_BreakPoints.find(fileName);
	if (iter != m_BreakPoints.end())
	{
		auto it = iter->second.first.find(line);
		if (it != iter->second.first.end())
		{
			iter->second.first.erase(it);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("删除断点错误,在模块<%s>未能找到<%d>行!\n"), fileName.c_str(), line);
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("删除断点错误,未能找到模块<%s>!\n"), fileName.c_str());
	}
}

void HazeDebugger::DeleteAllBreakPoint()
{
	m_BreakPoints.clear();
}

void HazeDebugger::DeleteModuleAllBreakPoint(const char* message)
{
	auto msgs = String2WString(message);
	auto moduleName = GetModuleNameByFilePath(msgs);

	auto iter = m_BreakPoints.find(moduleName);
	if (iter != m_BreakPoints.end())
	{
		iter->second.first.clear();
	}

	HAZE_LOG_ERR(HAZE_TEXT("清除模块<%s>的所有断点!\n"), moduleName.c_str());
}

void HazeDebugger::OnExecLine(uint32 line)
{
#if ENABLE_DEBUGGER_LOG

	HAZE_LOG_INFO(HAZE_TEXT("运行到<%d>行!\n"), line);

#endif // ENABLE_DEBUGGER_LOG

	auto moduleName = m_VM->GetModuleNameByCurrFunction();
	auto iter = m_BreakPoints.find(*moduleName);
	if (iter != m_BreakPoints.end())
	{
		auto it = iter->second.first.find(line);
		if (it != iter->second.first.end())
		{
			if (m_HookType & (uint32)DebuggerHookType::Line)
			{
				if (m_HookFunctionCall)
				{
					m_HookFunctionCall(m_VM);
				}

				m_CurrPauseModule = { iter->first, line };
				m_IsPause = true;
			}
		}
	}

	if (!m_IsPause && (CurrModuleIsStepOver() || m_IsStepIn))
	{
		if (line == m_CurrPauseModule.second && *moduleName == m_CurrPauseModule.first)
		{
			m_IsPause = true;
		}
		else
		{
			auto tempIter = m_TempBreakPoints.find(*moduleName);
			if (tempIter != m_TempBreakPoints.end())
			{
				auto tempIt = tempIter->second.first.find(line);
				if (tempIt != tempIter->second.first.end())
				{
					if (m_HookType & (uint32)DebuggerHookType::Line)
					{
						m_CurrPauseModule = { tempIter->first, line };
						m_IsPause = true;
						m_IsStepIn = false;

						tempIter->second.first.erase(tempIt);
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

	if (m_IsPause)
	{
#if ENABLE_DEBUGGER_LOG
		HAZE_LOG_INFO(HAZE_TEXT("调试器暂停<%s><%d>!\n"), m_CurrPauseModule.first.c_str(), line);
#endif

		SendBreakInfo();
	}
}

void HazeDebugger::Start()
{
	m_IsPause = false;
}

void HazeDebugger::StepOver()
{
	m_IsStepOvers[*m_VM->GetModuleNameByCurrFunction()] = true;

	if (m_IsPause)
	{
		m_IsPause = false;
		m_CurrPauseModule.second = m_VM->GetNextLine(m_CurrPauseModule.second);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("单步调试错误,不是暂停状态!\n"));
	}
}

void HazeDebugger::StepIn()
{
	if (m_IsPause)
	{
		m_IsPause = false;
		m_IsStepIn = true;
		auto pauseModule = m_VM->GetStepIn(m_CurrPauseModule.second);
		AddTempBreakPoint(pauseModule.first, pauseModule.second);

		XJson json;
		SetJsonType(json, HazeDebugInfoType::StepInInfo);
		SetJsonBreakFilePath(json, GetModuleFilePath(pauseModule.first));
		SetJsonBreakLine(json, pauseModule.second);
		auto& data = json.Encode();
		HazeDebuggerServer::SendData(const_cast<char*>(data.data()), (int)data.length());
	}
}

void HazeDebugger::StepInstruction()
{
}

void HazeDebugger::Continue()
{
	ClearCurrParseModuleData();
}

void HazeDebugger::SetJsonLocalVariable(XJson& json)
{
	auto& info = json["LocalVariable"];

	auto& frame = m_VM->VMStack->GetCurrFrame();
	if (frame.FunctionInfo)
	{
		for (size_t i = 0; i < frame.FunctionInfo->Variables.size(); i++)
		{
			if (frame.FunctionInfo->Variables[i].Line < m_CurrPauseModule.second)
			{
				SetJsonVariableData(info[i], frame.FunctionInfo->Variables[i]);
			}
		}
	}

	if (info.Empty())
	{
		info = "";
	}
}

void HazeDebugger::SetJsonModuleGlobalVariable(XJson& json)
{
	auto& info = json["GlobalVariable"];

	auto& frame = m_VM->VMStack->GetCurrFrame();
	if (frame.FunctionInfo)
	{
		for (size_t i = 0; i < frame.FunctionInfo->Variables.size(); i++)
		{
			SetJsonVariableData(info[i], frame.FunctionInfo->Variables[i]);
		}
	}
}

void HazeDebugger::AddTempBreakPoint(uint32 line)
{
	if (CurrModuleIsStepOver())
	{
		auto moduleName = m_VM->GetModuleNameByCurrFunction();
		m_TempBreakPoints[*moduleName].first.insert(line);
	}
}

void HazeDebugger::AddTempBreakPoint(const HAZE_STRING& moduleName, uint32 line)
{
	m_TempBreakPoints[moduleName].first.insert(line);
}

void HazeDebugger::SendProgramEnd()
{
	XJson json;
	json["Type"] = (int)HazeDebugInfoType::ProgramEnd;
	auto& data = json.Encode();
	HazeDebuggerServer::SendData(const_cast<char*>(data.data()), (int)data.length());
}

bool HazeDebugger::CurrModuleIsStepOver()
{
	auto moduleName = m_VM->GetModuleNameByCurrFunction();
	auto iter = m_IsStepOvers.find(*moduleName);
	if (iter != m_IsStepOvers.end())
	{
		return iter->second;
	}

	return false;
}

void HazeDebugger::SendBreakInfo()
{
	//auto iter = m_BreakPoints.find(m_CurrPauseModule.first);
	//if (iter != m_BreakPoints.end())
	{
		XJson json;
		SetJsonType(json, HazeDebugInfoType::BreakInfo);
		SetJsonBreakFilePath(json, GetModuleFilePath(m_CurrPauseModule.first));
		SetJsonBreakLine(json, m_CurrPauseModule.second);
		SetJsonLocalVariable(json);
		//SetJsonModuleGlobalVariable(json);

		auto& data = json.Encode();
		HazeDebuggerServer::SendData(const_cast<char*>(data.data()), (int)data.length());
	}
}

void HazeDebugger::SetJsonVariableData(XJson& json, const HazeVariableData& variable, const char* address, bool isStack)
{
	static std::string s_String;

	s_String = WString2String(variable.Variable.Name);
	json["Name"] = GB2312_2_UFT8(s_String.c_str());

	auto dataAddress = isStack ? m_VM->VMStack->GetAddressByEBP(variable.Offset) : address;
	if (address)
	{
		dataAddress += variable.Offset;
	}

	if (IsClassType(variable.Variable.Type.PrimaryType))
	{
		auto classData = m_VM->FindClass(variable.Variable.Type.CustomName);
		s_String = WString2String(variable.Variable.Type.CustomName);
		json["Type"] = GB2312_2_UFT8(s_String.c_str());

		for (size_t j = 0; j < classData->Members.size(); j++)
		{
			SetJsonVariableData(json["Value"][j], classData->Members[j], dataAddress);
		}
	}
	else if (IsPointerType(variable.Variable.Type.PrimaryType))
	{
		s_String = WString2String(HAZE_TEXT("指针"));
		json["Type"] = GB2312_2_UFT8(s_String.c_str());

		uint64 value;
		memcpy(&value, dataAddress, sizeof(value));
		json["Value"] = ToString((void*)value);

		if (variable.Variable.Type.NeedCustomName())
		{
			auto classData = m_VM->FindClass(variable.Variable.Type.CustomName);
			s_String = WString2String(variable.Variable.Type.CustomName);
			json["TypeClass"] = GB2312_2_UFT8(s_String.c_str());

			for (size_t j = 0; j < classData->Members.size(); j++)
			{
				SetJsonVariableData(json["PointerValue"][j], classData->Members[j], (const char*)value, false);
			}
		}

		if (variable.Variable.Type.NeedSecondaryType())
		{
			s_String = WString2String(GetHazeValueTypeString(variable.Variable.Type.SecondaryType));
			json["SubType"] = GB2312_2_UFT8(s_String.c_str());
		}
	}
	else if (IsArrayType(variable.Variable.Type.PrimaryType))
	{
		s_String = WString2String(GetHazeValueTypeString(variable.Variable.Type.PrimaryType));
		json["Type"] = GB2312_2_UFT8(s_String.c_str());
		
		s_String = WString2String(GetHazeValueTypeString(variable.Variable.Type.SecondaryType));
		json["SubType"] = GB2312_2_UFT8(s_String.c_str());

		int size = GetSizeByType(variable.Variable.Type, m_VM);

		if (variable.Variable.Type.CustomName.empty())
		{
			for (int i = 0; i < (int)variable.Size / size; i++)
			{
				GetHazeValueByBaseType(json["Value"][i], dataAddress + i * size,
					variable.Variable.Type.SecondaryType);
			}

		}
		else
		{
			for (int i = 0; i < (int)variable.Size / size; i++)
			{
				SetJsonVariableData(json["Value"][i], variable, dataAddress + size * i);
			}
		}
	}
	else if (IsReferenceType(variable.Variable.Type.PrimaryType))
	{
		s_String = WString2String(GetHazeValueTypeString(variable.Variable.Type.PrimaryType));
		json["Type"] = GB2312_2_UFT8(s_String.c_str());

		uint64 value;
		memcpy(&value, address, sizeof(value));
		json["Value"] = value;
	}
	else
	{
		s_String = WString2String(GetHazeValueTypeString(variable.Variable.Type.PrimaryType));
		json["Type"] = GB2312_2_UFT8(s_String.c_str());
		GetHazeValueByBaseType(json["Value"], dataAddress, variable.Variable.Type.PrimaryType);
	}
}