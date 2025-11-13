#include "HazePch.h"
#include "CompilerPointerFunction.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerSymbol.h"
#include "HazeTypeInfo.h"
#include "HazeLogDefine.h"

CompilerPointerFunction::CompilerPointerFunction(CompilerModule* compilerModule, const HazeVariableType& defineType,
	/*HazeVariableScope scope,*/ HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, /*scope,*/ desc, count), m_OwnerClass(nullptr)
{
}

CompilerPointerFunction::~CompilerPointerFunction()
{
}

HazeVariableType CompilerPointerFunction::GetFunctionType() const
{
	auto typeInfoMap = m_Module->GetCompiler()->GetCompilerSymbol()->GetTypeInfoMap();
	auto info = typeInfoMap->GetFunctionInfoByType(m_Type.TypeId);
	return info ? typeInfoMap->GetVarTypeById(info->at(0)) : HAZE_VAR_TYPE(HazeValueType::None);
}

HazeVariableType CompilerPointerFunction::GetParamTypeByIndex(x_uint64 index) const
{
	auto typeInfoMap = m_Module->GetCompiler()->GetCompilerSymbol()->GetTypeInfoMap();
	auto info = typeInfoMap->GetFunctionInfoByType(m_Type.TypeId);
	if (info && info->size() - 1 > index)
	{
		return typeInfoMap->GetVarTypeById(info->at(info->size() - 1 - index));
	}

	return HAZE_VAR_TYPE(HazeValueType::None);
}

HazeVariableType CompilerPointerFunction::GetParamTypeLeftToRightByIndex(x_uint64 index) const
{
	auto typeInfoMap = m_Module->GetCompiler()->GetCompilerSymbol()->GetTypeInfoMap();
	auto info = typeInfoMap->GetFunctionInfoByType(m_Type.TypeId);
	if (info && info->size() - 1 > index)
	{
		return typeInfoMap->GetVarTypeById(info->at(index + 1));
	}

	return HAZE_VAR_TYPE(HazeValueType::None);
}

const x_uint64 CompilerPointerFunction::GetParamCount() const
{
	return 0;// m_OwnerClass ? m_ParamTypes.size() - 1 : m_ParamTypes.size();
}