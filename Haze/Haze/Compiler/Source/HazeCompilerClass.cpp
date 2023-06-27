#include "HazeLog.h"

#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HazeCompilerClass::HazeCompilerClass(HazeCompilerModule* Module, const HAZE_STRING& Name, 
	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>>& Data)
	: Module(Module), Name(Name), Vector_Data(std::move(Data))
{
	DataSize = 0;

	uint32 MemberNum = 0;
	std::shared_ptr<HazeCompilerValue> LastMember = nullptr;
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			Vector_Data[i].second[j].second->Desc = Vector_Data[i].first;
			MemberNum++;
			LastMember = Vector_Data[i].second[j].second;
		}
	}

	MemoryAlign(MemberNum);
	DataSize = LastMember ? Vector_Offset.back() + HAZE_ALIGN(LastMember->GetSize(), HAZE_ALIGN_BYTE) : 0;
	//HAZE_LOG_INFO(HAZE_TEXT("¿‡ <%s> DataSize %d\n"), Name.c_str(), DataSize);
}

HazeCompilerClass::~HazeCompilerClass()
{
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerClass::FindFunction(const HAZE_STRING& FunctionName)
{
	auto Iter = HashMap_Function.find(GetHazeClassFunctionName(Name, FunctionName));
	if (Iter != HashMap_Function.end())
	{
		return Vector_Function[Iter->second];
	}
	
	return nullptr;
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerClass::AddFunction(std::shared_ptr<HazeCompilerFunction>& Function)
{
	auto Iter = HashMap_Function.find(Function->GetName());
	if (Iter == HashMap_Function.end())
	{
		Vector_Function.push_back(Function);
		HashMap_Function[Function->GetName()] = (unsigned int)Vector_Function.size() - 1;

		return Function;
	}

	return Vector_Function[Iter->second];
}

void HazeCompilerClass::InitThisValue()
{
	NewPointerToValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, Name), 
		HAZE_TEXT("")), HazeVariableScope::None, HazeDataDesc::ClassThis, 0));
	ThisClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, Name), 
		HAZE_CLASS_THIS), HazeVariableScope::Local, HazeDataDesc::ClassThis, 0));
	
	ThisPointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(CreateVariable(Module, HazeDefineVariable(HazeDefineType(HazeValueType::PointerClass, Name),
		 HAZE_CLASS_THIS), HazeVariableScope::Local, HazeDataDesc::ClassThis, 0));
}

uint64 HazeCompilerClass::GetMemberIndex(const HAZE_STRING& MemberName)
{
	size_t Index = 0;
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			if (Vector_Data[i].second[j].first == MemberName)
			{
				return Index;
			}

			Index++;
		}
		
	}

	return uint64(-1);
}

bool HazeCompilerClass::GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	return GetMemberName(Value.get(), OutName);
}

bool HazeCompilerClass::GetMemberName(const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < ThisClassValue->Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < ThisClassValue->Vector_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { Vector_Data[i].second[j].first, ThisClassValue->Vector_Data[i].second[j] }, Value, OutName))
			{
				return true;
			}
		}
	}

	/*for (size_t i = 0; i < NewPointerToValue->Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < NewPointerToValue->Vector_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { Vector_Data[i].second[j].first, NewPointerToValue->Vector_Data[i].second[j] }, Value, OutName))
			{
				return true;
			}
		}
	}*/

	return false;
}

bool HazeCompilerClass::GetMemberName(HazeCompilerClassValue* ClassValue, const HazeCompilerValue* Value, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < ClassValue->Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < ClassValue->Vector_Data[i].second.size(); j++)
		{
			if (TrtGetVariableName(nullptr, { Vector_Data[i].second[j].first, ClassValue->Vector_Data[i].second[j] }, Value, OutName))
			{
				return true;
			}
		}
	}

	return false;
}

void HazeCompilerClass::GenClassData_I_Code(HAZE_STRING_STREAM& SStream)
{
#if HAZE_I_CODE_ENABLE

	size_t DataNum = 0;
	for (auto& Datas : Vector_Data)
	{
		DataNum += Datas.second.size();
	}

	SStream << GetClassLabelHeader() << " " << Name << " " << GetDataSize() << " " << DataNum << std::endl;

	uint32 Index = 0;
	for (size_t i = 0; i < Vector_Data.size(); ++i)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			SStream << Vector_Data[i].second[j].first << " " << CAST_TYPE(Vector_Data[i].second[j].second->GetValueType().PrimaryType);
			if (Vector_Data[i].second[j].second->IsPointerBase())
			{
				SStream << " " << CAST_TYPE(Vector_Data[i].second[j].second->GetValueType().SecondaryType);
			}
			else if (Vector_Data[i].second[j].second->IsPointerClass() || Vector_Data[i].second[j].second->IsClass())
			{
				SStream << " " << Vector_Data[i].second[j].second->GetValueType().CustomName;
			}

			SStream << " " << Vector_Offset[Index] << " " << Vector_Data[i].second[j].second->GetSize() << std::endl;
			Index++;
		}
	}

#endif //HAZE_I_CODE_ENABLE
}

void HazeCompilerClass::GenClassFunction_I_Code(HAZE_STRING_STREAM& SStream)
{
#if HAZE_I_CODE_ENABLE

	for (auto& Iter : Vector_Function)
	{
		Iter->GenI_Code(SStream);
	}

#endif //HAZE_I_CODE_ENABLE
}

uint32 HazeCompilerClass::GetDataSize()
{
	return DataSize;
}

uint32 HazeCompilerClass::GetAlignSize()
{
	uint32 MaxMemberSize = 0;
	for (auto& It : Vector_Data)
	{
		for (auto& Iter : It.second)
		{
			if (Iter.second->GetSize() > MaxMemberSize)
			{
				MaxMemberSize = Iter.second->GetSize();
			}
		}
	}

	return MaxMemberSize < HAZE_ALIGN_BYTE ? MaxMemberSize : HAZE_ALIGN_BYTE;
}

uint32 HazeCompilerClass::GetOffset(uint32 Index, std::shared_ptr<HazeCompilerValue> Member)
{
	return Index * Member->GetSize();
}

void HazeCompilerClass::MemoryAlign(uint32 MemberNum)
{
	Vector_Offset.resize(MemberNum);
	uint32 Align = GetAlignSize();
	uint32 Index = 0;

	uint32 Size = 0;
	for (auto& It : Vector_Data)
	{
		for (auto& Iter : It.second)
		{
			uint32 ModSize = Size % Align;

			if (ModSize + Iter.second->GetSize() <= Align)
			{
				uint32 ModValue = ModSize % Iter.second->GetSize();
				if (ModValue == 0)
				{
					Vector_Offset[Index] = Size;
					Size += Iter.second->GetSize();
				}
				else
				{
					Vector_Offset[Index] = Size + ModValue;
					Size += (ModValue + Iter.second->GetSize());
				}
			}
			else
			{
				Vector_Offset[Index] = Size + (Align - ModSize);
				Size += ((Align - ModSize) + Iter.second->GetSize());
			}

			Index++;
		}
	}
}
