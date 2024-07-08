#pragma once
#include "HazeHeader.h"
#include "HazeDebugInfo.h"

class ASTBase;
class HazeCompiler;

class ASTEnum
{
public:
	ASTEnum(HazeCompiler* compiler, const SourceLocation& location, HString& name, HazeValueType baseType,
		V_Array<Pair<HString, Unique<ASTBase>>>& enums);

	~ASTEnum();

	void CodeGen();

private:
	void AddEnumOneValueByType(HazeValue& value, const HazeValue& prValue);

private:
	HazeCompiler* m_Compiler;
	HString m_EnumName;
	HazeValueType m_BaseType;
	SourceLocation m_Location;
	V_Array<Pair<HString, Unique<ASTBase>>> m_Enums;
};