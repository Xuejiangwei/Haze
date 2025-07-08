#include "HazePch.h"
#include "CompilerArrayValue.h"
#include "CompilerClass.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerFunction.h"
#include "CompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"
#include "HazeLogDefine.h"

CompilerArrayValue::CompilerArrayValue(CompilerModule* compilerModule, const HazeVariableType& defineType, HazeVariableScope scope,
	HazeDataDesc desc, int count)
	: CompilerValue(compilerModule, defineType, scope, desc, count)
{
}

CompilerArrayValue::~CompilerArrayValue()
{
}

HazeVariableType CompilerArrayValue::GetElementType() const
{
	auto typeInfoMap = m_Module->GetCompiler()->GetTypeInfoMap();
	auto info = typeInfoMap->GetTypeInfoById(m_Type.TypeId);
	HazeVariableType elementType;
	if (info->_Array.Dimension > 1)
	{
		HazeComplexTypeInfo elementInfo = *info;
		elementInfo._Array.Dimension -= 1;

		elementType.BaseType = elementInfo._Array.BaseType;
		elementType.TypeId = typeInfoMap->RegisterType(HAZE_COMPLEX_BASE_CAST(elementInfo));
	}
	else
	{
		auto elementInfo = typeInfoMap->GetTypeInfoById(info->_Array.TypeId1);
		elementType.BaseType = elementInfo->_BaseType.BaseType;
		elementType.TypeId = info->_Array.TypeId1;
	}
	
	return elementType;
}