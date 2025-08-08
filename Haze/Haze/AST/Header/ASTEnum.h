#pragma once
#include "HazeHeader.h"
#include "HazeDebugInfo.h"

class ASTBase;
class Compiler;

class ASTEnum
{
public:
	ASTEnum(Compiler* compiler, const SourceLocation& location, HString& name, V_Array<Pair<HString, Unique<ASTBase>>>& enums);

	~ASTEnum();

	void CodeGen();

private:
	void AddEnumOneValueByType(HazeValue& value, const HazeValue& prValue);

private:
	Compiler* m_Compiler;
	HString m_EnumName;
	SourceLocation m_Location;
	V_Array<Pair<HString, Unique<ASTBase>>> m_Enums;
};