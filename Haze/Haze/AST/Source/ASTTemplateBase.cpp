#include "HazePch.h"
#include "ASTTemplateBase.h"

ASTTemplateBase::ASTTemplateBase(V_Array<HString>& templateTypes)
	: m_TemplateTypes(Move(templateTypes))
{
}

ASTTemplateBase::~ASTTemplateBase()
{
}
