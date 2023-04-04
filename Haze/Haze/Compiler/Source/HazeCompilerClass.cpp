#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerClassValue.h"

HazeCompilerClass::HazeCompilerClass(HazeCompilerModule* Module, const HAZE_STRING& Name, std::vector<std::pair<HazeDataDesc, std::vector<HazeDefineVariable*>>>& Data)
	: Module(Module), Name(Name)
{
	DataSize = 0;
	Vector_Data.resize(Data.size());
	for (size_t i = 0; i < Data.size(); i++)
	{
		Vector_Data[i].first = Data[i].first;

		Vector_Data[i].second.resize(Data[i].second.size());
		for (size_t j = 0; j < Data[i].second.size(); j++)
		{
			Vector_Data[i].second[j] = *(Data[i].second[j]);
			
			DataSize += GetSizeByType(Vector_Data[i].second[j].Type, Module);
		}

		/*if (Vector_Data[i].Type.CustomName.empty())
		{
			DataSize += GetSizeByHazeType(Vector_Data[i].Type.Type);
		}
		else
		{
			DataSize += Module->FindClass(Vector_Data[i].Type.CustomName)->GetDataSize();
		}*/
	}
}

HazeCompilerClass::~HazeCompilerClass()
{
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerClass::FindFunction(const HAZE_STRING& FunctionName)
{
	auto Iter = HashMap_Function.find(FunctionName);
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
	ThisValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(Module, HazeDefineVariable(HazeDefineType(HazeValueType::Class, Name), HAZE_CLASS_THIS), HazeDataDesc::ClassThis));
}

size_t HazeCompilerClass::GetMemberIndex(const HAZE_STRING& MemberName)
{
	size_t Index = 0;
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			if (Vector_Data[i].second[j].Name == MemberName)
			{
				return Index;
			}

			Index++;
		}
		
	}

	return 0;
}

void HazeCompilerClass::GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < ThisValue->Vector_Data.size(); i++)
	{
		for (size_t j = 0; j < ThisValue->Vector_Data[i].second.size(); j++)
		{
			if (Value == ThisValue->Vector_Data[i].second[j])
			{
				OutName += HAZE_CLASS_THIS;
				OutName += HAZE_CLASS_POINTER_ATTR + Vector_Data[i].second[j].Name;
				return;
			}
		}
	}
}

const HazeDefineVariable* HazeCompilerClass::GetClassMemberData(const HAZE_STRING& MemberName) const
{
	for (auto& Iter : Vector_Data)
	{
		for (size_t i = 0; i < Iter.second.size(); i++)
		{
			if (Iter.second[i].Name == MemberName)
			{
				return &Iter.second[i];
			}
		}
	}

	return nullptr;
}

void HazeCompilerClass::GenClassData_I_Code(HAZE_OFSTREAM& OFStream)
{
#if HAZE_I_CODE_ENABLE
	OFStream << GetClassLabelHeader() << " " << Name << " " << GetDataSize() << " " << Vector_Data.size() << std::endl;

	for (size_t i = 0; i < Vector_Data.size(); ++i)
	{
		for (size_t j = 0; j < Vector_Data[i].second.size(); j++)
		{
			HAZE_STRING_STREAM HSS;
			StreamDefineVariable(HSS, Vector_Data[i].second[j]);
			OFStream << HSS.str();
		}
	}
#endif //HAZE_I_CODE_ENABLE
}

void HazeCompilerClass::GenClassFunction_I_Code(HAZE_OFSTREAM& OFStream)
{
#if HAZE_I_CODE_ENABLE
	for (auto& Iter : Vector_Function)
	{
		Iter->GenI_Code(OFStream);
	}
#endif //HAZE_I_CODE_ENABLE
}

unsigned int HazeCompilerClass::GetDataSize()
{
	return DataSize;
}
