#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerClassValue.h"

HazeCompilerClass::HazeCompilerClass(HazeCompilerModule* Module, const HAZE_STRING& Name, std::vector<HazeDefineVariable*>& Data)
	: Module(Module), Name(Name)
{
	DataSize = 0;
	Vector_Data.resize(Data.size());
	for (size_t i = 0; i < Data.size(); i++)
	{
		Vector_Data[i] = *Data[i];


		DataSize += GetSizeByType(Vector_Data[i].Type, Module);

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
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		if (Vector_Data[i].Name == MemberName)
		{
			return i;
		}
	}

	return 0;
}

void HazeCompilerClass::GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	for (size_t i = 0; i < ThisValue->Vector_Data.size(); i++)
	{
		if (Value == ThisValue->Vector_Data[i])
		{
			OutName += HAZE_CLASS_THIS;
			OutName += HAZE_CLASS_POINTER_ATTR + Vector_Data[i].Name;
			return;
		}
	}
}

const HazeDefineVariable* HazeCompilerClass::GetClassMemberData(const HAZE_STRING& MemberName) const
{
	for (auto& Iter : Vector_Data)
	{
		if (Iter.Name == MemberName)
		{
			return &Iter;
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
		HAZE_STRING_STREAM HSS;
		StreamDefineVariable(HSS, Vector_Data[i]);
		OFStream << HSS.str();
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
