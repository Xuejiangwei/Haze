#include "HazeCompilerArrayValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"

HazeCompilerArrayElementValue::HazeCompilerArrayElementValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
	HazeCompilerValue* Array, std::vector<HazeCompilerValue*> Index) : HazeCompilerValue(Module, DefineType, Scope, Desc, Count), ArrayOrPointer(Array), Index(Index)
{
}

HazeCompilerArrayElementValue::~HazeCompilerArrayElementValue()
{
}

HazeCompilerArrayValue::HazeCompilerArrayValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeVariableScope Scope, HazeDataDesc Desc, int Count,
	std::vector<std::shared_ptr<HazeCompilerValue>>& m_ArraySize)
	: HazeCompilerValue(Module, DefineType, Scope, Desc, Count), ArrayLength(0), Size(0)
{
	if (m_ArraySize.size() > 0)
	{
		ArrayLength = 1;
		for (auto& Iter : m_ArraySize)
		{
			ArrayLength *= Iter->GetValueType().PrimaryType == HazeValueType::UnsignedLong || Iter->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)Iter->GetValue().m_Value.UnsignedLong : Iter->GetValue().m_Value.UnsignedInt;
			Vector_Size.push_back(Iter);
		}

		Size = ArrayLength * GetSizeByType(DefineType, Module);
	}
}

HazeCompilerArrayValue::~HazeCompilerArrayValue()
{
}

uint32 HazeCompilerArrayValue::GetSizeByLevel(uint32 m_Level)
{
	uint32 Ret = 0;
	if (m_Level + 1 < Vector_Size.size())
	{
		Ret = 1;
		for (size_t i = m_Level + 1; i < Vector_Size.size(); i++)
		{
			Ret *= Vector_Size[i]->GetValueType().PrimaryType == HazeValueType::UnsignedLong || Vector_Size[i]->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)Vector_Size[i]->GetValue().m_Value.UnsignedLong : Vector_Size[i]->GetValue().m_Value.UnsignedInt;
		}
	}

	return Ret;
}