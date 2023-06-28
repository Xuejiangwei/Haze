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

void HookCall(HazeVM* VM)
{
}

void GetHazeValueByBaseType(open::OpenJson& Json, const char* Address, HazeValueType Type)
{
	switch (Type)
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
	case HazeValueType::Array:
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerFunction:
	case HazeValueType::PointerArray:
	case HazeValueType::PointerPointer:
	case HazeValueType::ReferenceBase:
	case HazeValueType::ReferenceClass:
	{
		uint64 Value;
		memcpy(&Value, Address, sizeof(Value));
		Json = Value;
	}
	default:
		break;
	}
}

HazeDebugger::HazeDebugger(HazeVM* VM, void(*EndCall)()) : VM(VM), EndCall(EndCall), HookFunctionCall(&HookCall), HookType(DebuggerHookType::Line),
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

void HazeDebugger::SetHook(void(*HookCall)(HazeVM* VM), uint32 Type)
{
	HookFunctionCall = HookCall;
	HookType = Type;
}

void HazeDebugger::AddBreakPoint(const char* Message)
{
	auto Msg = String2WString(Message);
	auto HazeChar = Msg.c_str();

	auto FileName = GetFileName(HazeChar);
	auto ModuleName = GetModuleNameByFilePath(FileName);
	uint32 Line = StringToStandardType<uint32>(HAZE_STRING(HazeChar));

	auto Iter = HashMap_BreakPoint.find(ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		Iter->second.first.insert(Line);
	}
	else
	{
		HashMap_BreakPoint[ModuleName] = { { Line }, FileName };
	}

#if ENABLE_DEBUGGER_LOG

	HAZE_LOG_INFO(HAZE_TEXT("��Ӷϵ�<%s><%s><%d>\n"), ModuleName.c_str(), FileName.c_str(), Line);

#endif
}

void HazeDebugger::DeleteBreakPoint(const char* Message)
{
	auto MsgString = String2WString(Message);
	auto Msg = MsgString.c_str();

	auto FileName = GetFileName(Msg);
	uint32 Line = StringToStandardType<uint32>(Msg);

	auto ModuleName = GetModuleNameByFilePath(FileName);
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
			HAZE_LOG_ERR(HAZE_TEXT("ɾ���ϵ����,��ģ��<%s>δ���ҵ�<%d>��!\n"), FileName.c_str(), Line);
		}
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("ɾ���ϵ����,δ���ҵ�ģ��<%s>!\n"), FileName.c_str());
	}
}

void HazeDebugger::DeleteAllBreakPoint()
{
	HashMap_BreakPoint.clear();
}

void HazeDebugger::DeleteModuleAllBreakPoint(const char* Message)
{
	auto Msgs = String2WString(Message);
	auto ModuleName = GetModuleNameByFilePath(Msgs);

	auto Iter = HashMap_BreakPoint.find(ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		Iter->second.first.clear();
	}

	HAZE_LOG_ERR(HAZE_TEXT("���ģ��<%s>�����жϵ�!\n"), ModuleName.c_str());
}

void HazeDebugger::OnExecLine(uint32 Line)
{
#if ENABLE_DEBUGGER_LOG

	HAZE_LOG_INFO(HAZE_TEXT("���е�<%d>��!\n"), Line);

#endif // ENABLE_DEBUGGER_LOG

	auto ModuleName = VM->GetModuleNameByCurrFunction();
	auto Iter = HashMap_BreakPoint.find(*ModuleName);
	if (Iter != HashMap_BreakPoint.end())
	{
		auto It = Iter->second.first.find(Line);
		if (It != Iter->second.first.end())
		{
			if (HookType & DebuggerHookType::Line)
			{
				if (HookFunctionCall)
				{
					HookFunctionCall(VM);
				}

				CurrPauseModule = { Iter->first, Line };
				IsPause = true;
			}
		}
	}

	if (!IsPause && CurrModuleIsStepOver())
	{
		if (Line == CurrPauseModule.second && *ModuleName == CurrPauseModule.first)
		{
			IsPause = true;
		}
		else
		{
			auto TempIter = HashMap_TempBreakPoint.find(*ModuleName);
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

		HAZE_LOG_INFO(HAZE_TEXT("��������ͣ<%s><%d>!\n"), CurrPauseModule.first.c_str(), Line);

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
	HashMap_IsStepOver[*VM->GetModuleNameByCurrFunction()] = true;

	if (IsPause)
	{
		IsPause = false;
		CurrPauseModule.second = VM->GetNextLine(CurrPauseModule.second);
	}
	else
	{
		HAZE_LOG_ERR(HAZE_TEXT("�������Դ���,������ͣ״̬!\n"));
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

	auto& Frame = VM->VMStack->GetCurrFrame();
	if (Frame.FunctionInfo)
	{
		for (size_t i = 0; i < Frame.FunctionInfo->Vector_Variable.size(); i++)
		{
			SetJsonVariableData(Info[i], Frame.FunctionInfo->Vector_Variable[i]);
		}
	}
}

void HazeDebugger::AddTempBreakPoint(uint32 Line)
{
	if (CurrModuleIsStepOver())
	{
		auto ModuleName = VM->GetModuleNameByCurrFunction();
		HashMap_TempBreakPoint[*ModuleName].first.insert(Line);
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
	auto ModuleName = VM->GetModuleNameByCurrFunction();
	auto Iter = HashMap_IsStepOver.find(*ModuleName);
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

		auto& data = Json.encode();
		HazeDebuggerServer::SendData(const_cast<char*>(data.data()), data.length());
	}
}

void HazeDebugger::SetJsonVariableData(open::OpenJson& Json, const HazeVariableData& Variable, const char* Address)
{
	static std::string String;

	String = WString2String(Variable.Variable.Name);
	Json["Name"] = GB2312_2_UFT8(String.c_str());

	auto DataAddress = VM->VMStack->GetAddressByEBP(Variable.Offset);
	if (Address)
	{
		DataAddress += Variable.Offset;
	}

	if (IsClassType(Variable.Variable.Type.PrimaryType))
	{
		auto Class = VM->FindClass(Variable.Variable.Type.CustomName);
		String = WString2String(Variable.Variable.Type.CustomName);
		Json["Type"] = GB2312_2_UFT8(String.c_str());

		for (size_t j = 0; j < Class->Vector_Member.size(); j++)
		{
			SetJsonVariableData(Json["Value"][j], Class->Vector_Member[j], DataAddress);
		}
	}
	else if (Variable.Variable.Type.NeedSecondaryType())
	{
	}
	else
	{
		String = WString2String(GetHazeValueTypeString(Variable.Variable.Type.PrimaryType));
		Json["Type"] = GB2312_2_UFT8(String.c_str());
		GetHazeValueByBaseType(Json["Value"], DataAddress, Variable.Variable.Type.PrimaryType);
	}
}