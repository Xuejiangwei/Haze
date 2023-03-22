#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"

HazeCompilerClass::HazeCompilerClass(HazeCompilerModule* Module, const HAZE_STRING& Name, std::vector<HazeDefineVariable*>& Data)
	:Module(Module), Name(Name)
{
	for (size_t i = 0; i < Data.size(); i++)
	{
		auto Param = CreateVariable(Module, *Data[i], InstructionScopeType::Class);
		Vector_Data.push_back(Param);
		HashMap_Data[Data[i]->Name] = (unsigned int)i;
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

std::shared_ptr<HazeCompilerValue> HazeCompilerClass::GetClassData(const HAZE_STRING& Name)
{
	auto Iter = HashMap_Data.find(Name);
	if (Iter != HashMap_Data.end())
	{
		return Vector_Data[Iter->second];
	}

	return nullptr;
}

bool HazeCompilerClass::GetDataName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName)
{
	for (auto& Iter : HashMap_Data)
	{
		if (Vector_Data[Iter.second] == Value)
		{
			OutName += HAZE_CLASS_THIS;
			OutName += HAZE_CLASS_POINTER_ATTR;
			OutName += Iter.first;
			return true;
		}
	}
	return false;
}

void HazeCompilerClass::GenClassData_I_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream)
{
#if HAZE_I_CODE_ENABLE
	OFStream << GetClassLabelHeader() << " " << Name << " " << Vector_Data.size() << std::endl;

	for (size_t i = 0; i < Vector_Data.size(); ++i)
	{
		for (auto& Iter : HashMap_Data)
		{
			if (Iter.second == i)
			{
				OFStream << Iter.first << " " << HAZE_CAST_VALUE_TYPE(Vector_Data[i]->GetValue().Type) << std::endl;
				break;
			}
		}

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
