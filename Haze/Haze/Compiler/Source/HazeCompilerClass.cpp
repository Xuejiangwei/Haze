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

		if (Vector_Data[i].Type.CustomName.empty())
		{
			DataSize += GetSizeByType(Vector_Data[i].Type.Type);
		}
		else
		{
			DataSize += Module->FindClass(Vector_Data[i].Type.CustomName)->GetDataSize();
		}
	}
}

HazeCompilerClass::~HazeCompilerClass()
{
}

std::shared_ptr<HazeCompilerFunction> HazeCompilerClass::FindFunction(const HAZE_STRING& Name)
{
	auto Iter = HashMap_Function.find(Name);
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
	ThisValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(CreateVariable(Module, HazeDefineVariable(HazeDefineData(HazeValueType::Class, Name), HAZE_CLASS_THIS), InstructionScopeType::ClassThis));
}

size_t HazeCompilerClass::GetMemberIndex(const HAZE_STRING& Name)
{
	for (size_t i = 0; i < Vector_Data.size(); i++)
	{
		if (Vector_Data[i].Name == Name)
		{
			return i;
		}
	}

	return 0;
}

void HazeCompilerClass::GetMemberName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& Name)
{
	for (size_t i = 0; i < ThisValue->Vector_Data.size(); i++)
	{
		if (Value == ThisValue->Vector_Data[i])
		{
			Name += HAZE_CLASS_THIS;
			Name += HAZE_CLASS_POINTER_ATTR + Vector_Data[i].Name;
			return;
		}
	}
}

const HazeDefineVariable* HazeCompilerClass::GetClassData(const HAZE_STRING& Name) const
{
	for (auto& Iter : Vector_Data)
	{
		if (Iter.Name == Name) 
		{
			return &Iter;
		}
	}

	return nullptr;
}

void HazeCompilerClass::GenClassData_I_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream)
{
#if HAZE_I_CODE_ENABLE
	OFStream << GetClassLabelHeader() << " " << Name << " " << GetDataSize() << " " << Vector_Data.size() << std::endl;

	for (size_t i = 0; i < Vector_Data.size(); ++i)
	{
		OFStream << Vector_Data[i].Name << " " << HAZE_CAST_VALUE_TYPE(Vector_Data[i].Type.Type) << " "
			<< Vector_Data[i].Type.CustomName.size() << " " << Vector_Data[i].Type.CustomName << std::endl;
		
	}
#endif //HAZE_I_CODE_ENABLE
}

void HazeCompilerClass::GenClassFunction_I_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream)
{
#if HAZE_I_CODE_ENABLE
	for (auto& Iter : Vector_Function)
	{
		Iter->GenI_Code(Module, OFStream);
	}
#endif //HAZE_I_CODE_ENABLE
}

unsigned int HazeCompilerClass::GetDataSize()
{
	return DataSize;
}
