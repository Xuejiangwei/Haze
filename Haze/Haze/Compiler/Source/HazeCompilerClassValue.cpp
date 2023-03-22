#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"

HazeCompilerClassValue::HazeCompilerClassValue(HazeCompilerModule* Module, const HazeDefineData& DefineType, InstructionScopeType Scope, std::shared_ptr<HazeCompilerValue> Parent)
	: HazeCompilerValue(Module, DefineType, Scope, Parent)
{
	OwnerClass = Module->FindClass(DefineType.CustomName);
}

HazeCompilerClassValue::~HazeCompilerClassValue()
{
}

void HazeCompilerClassValue::SetUseClassMember(int Offset, HazeDefineData& Data)
{
	CurrUseMember.first = Offset;
	CurrUseMember.second.CustomName = Data.CustomName;
	CurrUseMember.second.Type = Data.Type;
}
