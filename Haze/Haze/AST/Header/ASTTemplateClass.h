#pragma once
#include "HazeHeader.h"
#include "ASTTemplateBase.h"

class Compiler;
class ASTBase;
class ASTClassFunctionSection;

class ASTTemplateClass : public ASTTemplateBase
{
public:
	ASTTemplateClass(Compiler* compiler, HString& name, V_Array<HString>& parentClass,
		V_Array<HString>& templateTypes,
		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>>& data, 
		Unique<ASTClassFunctionSection>& functionSection);

	~ASTTemplateClass();

	virtual void CodeGen() override;

private:
	Compiler* m_Compiler;

	HString m_ClassName;
	V_Array<HString> m_ParentClasses;
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> m_ClassDatas;
	Unique<ASTClassFunctionSection> m_ClassFunctionSection;
};