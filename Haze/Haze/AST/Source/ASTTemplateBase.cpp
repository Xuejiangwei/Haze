#include "HazePch.h"
#include "ASTTemplateBase.h"

ASTTemplateBase::ASTTemplateBase(V_Array<HString>& templateTypes)
	: m_TemplateTypes(std::move(templateTypes))
{
}

ASTTemplateBase::~ASTTemplateBase()
{
}
