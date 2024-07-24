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
			m_ArrayLength *= iter->GetValueType().PrimaryType == HazeValueType::UInt64 || iter->GetValueType().PrimaryType == HazeValueType::Int64 ?
				(uint32)iter->GetValue().Value.UInt64 : iter->GetValue().Value.UInt32;
			m_SizeValues.push_back(iter);
		}

		//m_Size = m_ArrayLength * GetSizeByType(defineType, compilerModule);
	}
}

HazeCompilerArrayValue::~HazeCompilerArrayValue()
{
}

uint64 HazeCompilerArrayValue::GetSizeByLevel(uint64 level)
{
	uint64 ret = 0;
	if (level + 1 < m_SizeValues.size())
	{
		ret = 1;
		for (size_t i = level + 1; i < m_SizeValues.size(); i++)
		{
			ret *= m_SizeValues[i]->GetValueType().PrimaryType == HazeValueType::UInt64 || m_SizeValues[i]->GetValueType().PrimaryType == HazeValueType::Int64 ?
				m_SizeValues[i]->GetValue().Value.UInt64 : m_SizeValues[i]->GetValue().Value.UInt32;
		}
	}

	return ret;
}