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

	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			DataSize += Vector_Data[i].second[j].second->GetSize();
		}
	}
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
		HAZE_TEXT("")), HazeDataDesc::ClassThis, 0));
	ThisClassValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, Name), 
		HAZE_CLASS_THIS), HazeDataDesc::ClassThis, 0));
	
	ThisPointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(CreateVariable(Module, 
		HazeDefineVariable(HazeDefineType(HazeValueType::PointerClass, Name), HAZE_CLASS_THIS), HazeDataDesc::ClassThis, 0));
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
				OutName = HAZE_CLASS_THIS + OutName;
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

//const HazeDefineVariable* HazeCompilerClass::GetClassMemberData(const HAZE_STRING& MemberName) const
//{
//	for (auto& Iter : Vector_Data)
//	{
//		for (size_t i = 0; i < Iter.second.size(); i++)
//		{
//			if (Iter.second[i].first == MemberName)
//			{
//				return Iter.second[i].second.get();
//			}
//		}
//	}
//
//	return nullptr;
//}

void HazeCompilerClass::GenClassData_I_Code(HAZE_STRING_STREAM& SStream)
{
#if HAZE_I_CODE_ENABLE

	size_t DataNum = 0;
	for (auto& Datas : Vector_Data)
	{
		DataNum += Datas.second.size();
	}

	SStream << GetClassLabelHeader() << " " << Name << " " << GetDataSize() << " " << DataNum << std::endl;

	for (size_t i = 0; i < Vector_Data.size(); ++i)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			SStream << Vector_Data[i].second[j].first << " " << HAZE_CAST_VALUE_TYPE(Vector_Data[i].second[j].second->GetValueType().PrimaryType);
			if (Vector_Data[i].second[j].second->GetValueType().PrimaryType == HazeValueType::PointerBase)
			{
				SStream << " " << HAZE_CAST_VALUE_TYPE(Vector_Data[i].second[j].second->GetValueType().SecondaryType);
			}
			else if (Vector_Data[i].second[j].second->GetValueType().PrimaryType == HazeValueType::PointerClass ||
				Vector_Data[i].second[j].second->GetValueType().PrimaryType == HazeValueType::Class)
			{
				SStream << " " << Vector_Data[i].second[j].second->GetValueType().CustomName;
			}

			SStream << std::endl;
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

unsigned int HazeCompilerClass::GetDataSize()
{
	return DataSize;
}
