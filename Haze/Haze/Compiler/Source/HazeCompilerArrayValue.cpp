#include "HazePch.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerClassValue.h"
#include "HazeLogDefine.h"

HazeCompilerArrayElementValue::HazeCompilerArrayElementValue(HazeCompilerModule* compilerModule, 
	const HazeDefineType& defineType, HazeVariableScope scope, HazeDataDesc desc, int count,
	HazeCompilerValue* arrayValue, V_Array<HazeCompilerValue*> index) 
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count), m_ArrayOrPointer(arrayValue), m_ArrayIndex(index)
{
}

HazeCompilerArrayElementValue::~HazeCompilerArrayElementValue()
{
}

HazeCompilerArrayValue::HazeCompilerArrayValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
	HazeDataDesc desc, int count, V_Array<Share<HazeCompilerValue>>& arraySize)
	: HazeCompilerValue(compilerModule, defineType, scope, desc, count), m_ArrayLength(0), m_Size(0)
{
	if (arraySize.size() > 0)
	{
		m_ArrayLength = 1;
		for (auto& iter : arraySize)
		{
			m_ArrayLength *= iter->GetValueType().PrimaryType == HazeValueType::UnsignedLong || iter->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)iter->GetValue().Value.UnsignedLong : iter->GetValue().Value.UnsignedInt;
			m_SizeValues.push_back(iter);
		}

		//m_Size = m_ArrayLength * GetSizeByType(defineType, compilerModule);
	}
	else if (IsArrayPointerType(defineType.PrimaryType))
	{
		//m_Size = GetSizeByHazeType(defineType.PrimaryType);
	}
}

HazeCompilerArrayValue::~HazeCompilerArrayValue()
{
}

uint32 HazeCompilerArrayValue::GetSizeByLevel(uint32 level)
{
	uint32 ret = 0;
	if (level + 1 < m_SizeValues.size())
	{
		ret = 1;
		for (size_t i = level + 1; i < m_SizeValues.size(); i++)
		{
			ret *= m_SizeValues[i]->GetValueType().PrimaryType == HazeValueType::UnsignedLong || m_SizeValues[i]->GetValueType().PrimaryType == HazeValueType::Long ?
				(uint32)m_SizeValues[i]->GetValue().Value.UnsignedLong : m_SizeValues[i]->GetValue().Value.UnsignedInt;
		}
	}

	return ret;
}