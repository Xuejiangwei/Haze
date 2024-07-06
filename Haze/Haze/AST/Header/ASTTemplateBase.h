#pragma once

class ASTTemplateBase
{
public:
	ASTTemplateBase(V_Array<HString>& templateTypes);

	~ASTTemplateBase();

	virtual void CodeGen() {}

protected:
	V_Array<HString> m_TemplateTypes;
};