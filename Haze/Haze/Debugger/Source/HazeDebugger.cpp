#include "HazePch.h"
#include "HazeDebugger.h"
#include "HazeLog.h"
#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeFilePathHelper.h"
#include "HazeDebuggerServer.h"

#include "ObjectString.h"
#include "ObjectClosure.h"
#include "ObjectArray.h"
#include "ObjectHash.h"
#include "ObjectBase.h"
#include "ObjectClass.h"

#define ENABLE_DEBUGGER_LOG 0

static STDString GetFileName(const x_HChar*& msg)
{
	bool IsNewLine = false;
	STDString FileName;
	FileName.clear();
	while (!HazeIsSpace(*msg, &IsNewLine))
	{
		FileName += *msg++;
	}

	msg++;

	return FileName;
}

//static  HString GetFileModuleName(const HChar*& filePath)
//{
//	return GetModuleNameByFilePath(GetFileName(filePath));
//}

void HookCall(HazeVM* vm)
{
	if (vm)
	{
		HAZE_LOG_INFO_W("断点命中\n");
	}
}

void GetHazeValueByBaseType(HazeVM* vm, XJson& json, const char* address, HazeValueType type, bool expand)
{
#define ADDRESS_TEXT			"Address"
#define VALUE_TYPE_TEXT			"ValueType"
#define SIZE_TEXT				"Length"
#define CAPACITY_TEXT			"Capacity"
#define JSON_MEMBER(VAR)		auto& VAR = json["Member"]

#define HASH_KEY_TEXT			"Key"
#define HASH_VALUE_TEXT			"Value"

	json["Type"] = WString2String(*vm->GetTypeInfoMap()->GetTypeName(HAZE_TYPE_ID(type)));
	auto& valueJson = json["Value"];
	switch (type)
	{
		case HazeValueType::Bool:
		{
			bool Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::Int8:
		{
			x_int8 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::UInt8:
		{
			x_uint8 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::Int16:
		{
			x_int16 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::UInt16:
		{
			x_uint16 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::Int32:
		{
			x_int32 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::UInt32:
		{
			x_uint32 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::Int64:
		{
			x_int64 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break; 
		case HazeValueType::UInt64:
		{
			x_uint64 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::Float32:
		{
			x_float32 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::Float64:
		{
			x_float64 Value;
			memcpy(&Value, address, sizeof(Value));
			valueJson = Value;
		}
		break;
		case HazeValueType::String:
		{
			ObjectString* obj;
			memcpy(&obj, address, sizeof(obj));

			json[ADDRESS_TEXT] = ToString((void*)obj);
			valueJson = GB2312_2_UFT8(WString2String(obj->GetData()).c_str());

			json[SIZE_TEXT] = obj->GetLength();
			json[CAPACITY_TEXT] = obj->GetCapacity();
		}
		break;
		case HazeValueType::ObjectBase:
		{
			ObjectBase* obj;
			memcpy(&obj, address, sizeof(obj));

			json[ADDRESS_TEXT] = ToString((void*)obj);

			GetHazeValueByBaseType(vm, valueJson, (char*)obj->GetBaseData(), obj->GetBaseType(), false);
			json[VALUE_TYPE_TEXT] = WString2String(*vm->GetTypeInfoMap()->GetTypeName(HAZE_TYPE_ID(obj->GetBaseType())));
		}
		break;
		case HazeValueType::Closure:
		{
			ObjectClosure* obj;
			memcpy(&obj, address, sizeof(obj));
			json[ADDRESS_TEXT] = ToString((void*)obj);

			if (expand)
			{
				JSON_MEMBER(memberJson);
				for (x_uint64 i = 0; i < obj->GetRefVariableSize(); i++)
				{
					auto refVar = obj->GetRefVariableDataByIndex(i);
					auto varData = obj->GetRefFunctionVariableDataByIndex(i);
					GetHazeValueByBaseType(vm, memberJson[GB2312_2_UFT8(WString2String(GetFunctionVariableNameTrimCount(varData->Variable.Name)).c_str())], 
						(const char*)(&refVar->Object), refVar->Type.BaseType, false);
				}
			}
		}
		break;
		case HazeValueType::Array:
		{
			ObjectArray* obj;
			memcpy(&obj, address, sizeof(obj));
			json[ADDRESS_TEXT] = ToString((void*)obj);
			json[SIZE_TEXT] = obj->GetLength();
			json[CAPACITY_TEXT] = obj->GetCapacity();

			json[VALUE_TYPE_TEXT] = WString2String(*vm->GetTypeInfoMap()->GetTypeName(HAZE_TYPE_ID(type)));

			if (expand)
			{
				JSON_MEMBER(memberJson);
				for (x_uint64 i = 0; i < obj->GetLength(); i++)
				{
					GetHazeValueByBaseType(vm, memberJson[i], obj->GetIndex(i), obj->GetType().BaseType, false);
				}
			}
		}
		break;
		case HazeValueType::Hash:
		{
			ObjectHash* obj;
			memcpy(&obj, address, sizeof(obj));
			json[ADDRESS_TEXT] = ToString((void*)obj);
			json[SIZE_TEXT] = obj->GetLength();
			json[CAPACITY_TEXT] = obj->GetCapacity();

			json[VALUE_TYPE_TEXT] = WString2String(*vm->GetTypeInfoMap()->GetTypeName(HAZE_TYPE_ID(type)));

			if (expand)
			{
				JSON_MEMBER(memberJson);
				auto keyType = obj->GetKeyBaseType().BaseType;
				auto valueType = obj->GetValueBaseType().BaseType;
				for (x_uint64 i = 0; i < obj->GetCapacity(); i++)
				{
					auto node = obj->GetIndex(i);
					if (!node->IsNone())
					{
						GetHazeValueByBaseType(vm, memberJson[i][HASH_KEY_TEXT], (const char*)(&node->Key), keyType, false);
						GetHazeValueByBaseType(vm, memberJson[i][HASH_VALUE_TEXT], (const char*)(&node->Value), valueType, false);
					}
				}
			}
		}
		break;
		case HazeValueType::Class:
		{
			ObjectClass* obj;
			memcpy(&obj, address, sizeof(obj));
			json[ADDRESS_TEXT] = ToString((void*)obj);


			auto classData = obj->GetClassData();
			json[VALUE_TYPE_TEXT] = GB2312_2_UFT8(WString2String(*vm->GetTypeInfoMap()->GetTypeName(classData->TypeId)).c_str());

			if (expand)
			{
				JSON_MEMBER(memberJson);
				for (x_uint64 i = 0; i < classData->Members.size(); i++)
				{
					auto& member = classData->Members[i];
					GetHazeValueByBaseType(vm, memberJson[GB2312_2_UFT8(WString2String(member.Variable.Name).c_str())], obj->GetMemberByIndex(i), member.Variable.Type.BaseType, false);
				}
			}
		}
		break;
		case HazeValueType::DynamicClass:
		{
			ObjectClass* obj;
			memcpy(&obj, address, sizeof(obj));
			json[ADDRESS_TEXT] = ToString((void*)obj);
			json[VALUE_TYPE_TEXT] = WString2String(*vm->GetTypeInfoMap()->GetTypeName(HAZE_TYPE_ID(type)));
		}
		break;
		default:
			HAZE_LOG_ERR_W("Debug 获得基础类型<%d>数据错误!\n", (x_uint32)type);
			break;
	}
}

HazeDebugger::HazeDebugger(HazeVM* vm, void(*endCall)()) 
	: m_VM(vm), m_EndCall(endCall), m_HookFunctionCall(&HookCall), m_HookType((x_uint32)DebuggerHookType::Line), m_IsStart(false), m_IsPause(true),
	 m_IsStepIn(false), m_IsStepInInstruction(false)
{
	m_BreakPoints.clear();
	m_TempBreakPoints.clear();
	m_IsStepOvers.clear();
	m_StepInStack.clear();
}

HazeDebugger::~HazeDebugger()
{
	if (m_EndCall)
	{
		m_EndCall();
	}
}

void HazeDebugger::SetHook(void(*HookCall)(HazeVM* vm), x_uint32 type)
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
	x_uint32 Line = StringToStandardType<x_uint32>(STDString(hazeChar));

	auto iter = m_BreakPoints.find(moduleName);
	if (iter != m_BreakPoints.end())
	{
		iter->second.State[Line] = { BreakPointState::None };
	}
	else
	{
		m_BreakPoints[moduleName].Path = fileName;
		m_BreakPoints[moduleName].State[Line] = BreakPointState::None;
	}

	HAZE_LOG_INFO(H_TEXT("添加断点<%s><%s><%d>\n"), moduleName.c_str(), fileName.c_str(), Line);
}

void HazeDebugger::DeleteBreakPoint(const char* message)
{
	auto msgString = String2WString(message);
	auto msg = msgString.c_str();

	auto fileName = GetFileName(msg);
	x_uint32 line = StringToStandardType<x_uint32>(msg);

	auto moduleName = GetModuleNameByFilePath(fileName);
	auto iter = m_BreakPoints.find(fileName);
	if (iter != m_BreakPoints.end())
	{
		auto it = iter->second.State.find(line);
		if (it != iter->second.State.end())
		{
			iter->second.State.erase(it);
		}
		else
		{
			HAZE_LOG_ERR_W("删除断点错误,在模块<%s>未能找到<%d>行!\n", fileName.c_str(), line);
		}
	}
	else
	{
		HAZE_LOG_ERR_W("删除断点错误,未能找到模块<%s>!\n", fileName.c_str());
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
		iter->second.State.clear();
	}

	HAZE_LOG_ERR_W("清除模块<%s>的所有断点!\n", moduleName.c_str());
}

void HazeDebugger::OnExecLine(x_uint32 line)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (line == 0 || !m_IsStart)
	{
		return;
	}

	auto moduleName = m_VM->GetModuleNameByCurrFunction();
	auto iter = m_BreakPoints.find(*moduleName);
	if (iter != m_BreakPoints.end())
	{
		auto it = iter->second.State.find(line);
		if (it != iter->second.State.end())
		{
			if (m_HookType & (x_uint32)DebuggerHookType::Line && it->second != BreakPointState::Hit)
			{
				if (m_HookFunctionCall)
				{
					m_HookFunctionCall(m_VM);
				}

				it->second = BreakPointState::Hit;
				m_CurrPauseModule = { iter->first, line, (x_uint32)m_VM->m_Stack->m_StackFrame.size() };
				m_IsPause = true;
			}
		}

		for (auto& info : iter->second.State)
		{
			if (info.first != line && info.second == BreakPointState::Hit)
			{
				info.second = BreakPointState::None;
			}
		}
	}

	if (!m_IsPause)
	{
		if (CurrModuleIsStepOver())
		{
			auto stackDepth = m_VM->m_Stack->m_StackFrame.size();
			if (stackDepth <= m_CurrPauseModule.StackDepth && line != m_CurrPauseModule.CurrLine)
			{
				m_IsPause = true;
				m_CurrPauseModule.ModuleName = *moduleName;
				m_CurrPauseModule.CurrLine = line;
			}
			/*else if (line <= m_CurrPauseModule.CurrLine && m_VM->FindClass)
			{

			}*/
		}
			
		if (m_IsStepIn)
		{
			auto tempIter = m_TempBreakPoints.find(*moduleName);
			if (tempIter != m_TempBreakPoints.end())
			{
				auto tempIt = tempIter->second.first.find(line);
				if (tempIt != tempIter->second.first.end())
				{
					if (m_HookType & (x_uint32)DebuggerHookType::Line)
					{
						m_CurrPauseModule = { tempIter->first, line, (x_uint32)m_VM->m_Stack->m_StackFrame.size() };
						m_IsPause = true;
						m_IsStepIn = false;

						tempIter->second.first.erase(tempIt);
					}
				}
			}
		}

		if (m_StepInStack.size() > 0)
		{
			auto& stepInCache = m_StepInStack.back();
			if (line == stepInCache.second && *moduleName == stepInCache.first)
			{
				m_IsPause = true;
				m_StepInStack.pop_back();
				m_CurrPauseModule = { *moduleName, line, (x_uint32)m_VM->m_Stack->m_StackFrame.size() };
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
		HAZE_LOG_INFO(H_TEXT("调试器暂停<%s><%d>!\n"), m_CurrPauseModule.ModuleName.c_str(), line);
#endif

		SendBreakInfo();
	}
}

void HazeDebugger::Start()
{
	m_IsStart = true;
	Continue();
}

void HazeDebugger::End()
{
	Continue();
	exit(0);
}

void HazeDebugger::StepOver()
{
	m_IsStepOvers[*m_VM->GetModuleNameByCurrFunction()] = true;

	if (m_IsPause)
	{
		Continue();
	}
	else
	{
		HAZE_LOG_ERR_W("单步调试错误,不是暂停状态!\n");
	}
}

void HazeDebugger::StepIn()
{
	if (m_IsPause)
	{
		Continue();

		m_IsStepIn = true;
		m_StepInStack.push_back({ m_CurrPauseModule.ModuleName, m_VM->GetNextInstructionLine(m_CurrPauseModule.CurrLine) });
		auto pauseModule = m_VM->GetStepIn(m_CurrPauseModule.CurrLine);
		AddTempBreakPoint(pauseModule.first.data(), pauseModule.second);

		XJson json;
		SetJsonType(json, HazeDebugInfoType::StepInInfo);
		SetJsonBreakFilePath(json, GetModuleFilePath(pauseModule.first.data()));
		SetJsonBreakLine(json, pauseModule.second);
		auto& data = json.Encode();
		HazeDebuggerServer::SendData(const_cast<char*>(data.data()), (int)data.length());
	}
}

void HazeDebugger::StepInstruction()
{
	Continue();
}

void HazeDebugger::Continue(bool clearAll)
{
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_IsPause = false;

		if (clearAll)
		{
			m_IsStepOvers.clear();
			m_CurrPauseModule.ModuleName.clear();
			m_TempBreakPoints.clear();
			m_StepInStack.clear();
		}
	}
	m_CV.notify_one();
}

void HazeDebugger::SetJsonLocalVariable(XJson& json)
{
	auto& info = json["LocalVariable"];
	auto& frame = m_VM->m_Stack->GetCurrFrame();
	if (frame.FunctionInfo)
	{
		for (size_t i = 0; i < frame.FunctionInfo->Variables.size(); i++)
		{
			if (frame.FunctionInfo->Variables[i].Line < m_CurrPauseModule.CurrLine)
			{
				SetJsonVariableData(info[i], frame.FunctionInfo->Variables[i]);
			}
		}
	}
}

void HazeDebugger::SetJsonModuleGlobalVariable(XJson& json)
{
	auto& info = json["GlobalVariable"];
	auto funcation = m_VM->GetFunctionDataByName(GetHazeModuleGlobalDataInitFunctionName(m_CurrPauseModule.ModuleName));
	if (funcation)
	{
		for (x_uint64 i = 0; i < funcation->Variables.size(); i++)
		{
			auto name = GetFunctionVariableNameTrimCount(funcation->Variables[i].Variable.Name);
			SetJsonVariableData(info[i], funcation->Variables[i], m_VM->GetGlobalValueById(HazeTypeInfoMap::GenSymbolId(name)), false);
		}
	}
}

void HazeDebugger::WaitIfPaused()
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	m_CV.wait(lock, [this]
		{
			return !m_IsPause;
		}
	);
}

void HazeDebugger::AddTempBreakPoint(x_uint32 line)
{
	if (CurrModuleIsStepOver())
	{
		auto moduleName = m_VM->GetModuleNameByCurrFunction();
		m_TempBreakPoints[*moduleName].first.insert(line);
	}
}

void HazeDebugger::AddTempBreakPoint(const STDString& moduleName, x_uint32 line)
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

void HazeDebugger::SendVariableInfo(const char* jsonStr)
{
	XJson json;
	json.Decode(jsonStr);
	auto id = json["QueryId"].StringToInt32();
	auto addressStr = HAZE_BINARY_STRING("0x") + json["Address"].Data();
	auto name = json["VariableName"].Data();
	auto type = json["Type"].Data();

	auto address = std::stoull(addressStr, nullptr, 16);

	XJson json1;
	json1["QueryId"] = id;
	json1["Type"] = (int)HazeDebugInfoType::VariableInfo;

	auto varType = m_VM->GetTypeInfoMap()->GetBaseIdByTypeName(String2WString(type));
	GetHazeValueByBaseType(m_VM, json1["Variable"], (const char*)(&address), HAZE_ID_2_TYPE(varType), true);
	
	auto& data = json1.Encode();
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

void HazeDebugger::SetJsonBreakFilePath(XJson& json, STDString path)
{
	ReplacePathSlash(path);
	auto m_Name = WString2String(path);
	json["BreakPathFile"] = GB2312_2_UFT8(m_Name.c_str());
}

void HazeDebugger::SendBreakInfo()
{
	auto path = m_VM->GetModulePathByName(m_CurrPauseModule.ModuleName);

	XJson json;
	SetJsonType(json, HazeDebugInfoType::BreakInfo);
	SetJsonBreakFilePath(json, *path);
	SetJsonBreakLine(json, m_CurrPauseModule.CurrLine);

	SetJsonLocalVariable(json);
	SetJsonModuleGlobalVariable(json);


	if (json.Empty())
	{
		json = "";
	}

	auto& data = json.Encode();
	HazeDebuggerServer::SendData(const_cast<char*>(data.data()), (int)data.length());
}

void HazeDebugger::SetJsonVariableData(XJson& json, const HazeVariableData& variable, const char* address, bool isStack)
{
	static std::string s_String;

	s_String = WString2String(GetFunctionVariableNameTrimCount(variable.Variable.Name));
	json["Name"] = GB2312_2_UFT8(s_String.c_str());

	auto dataAddress = isStack ? m_VM->m_Stack->GetAddressByEBP(variable.Offset) : address;
	if (address)
	{
		dataAddress += variable.Offset;
	}
	
	GetHazeValueByBaseType(m_VM, json, dataAddress, variable.Variable.Type.BaseType, false);
}