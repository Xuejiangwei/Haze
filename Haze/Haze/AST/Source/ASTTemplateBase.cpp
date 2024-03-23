#include "ASTTemplateBase.h"

ASTTemplateBase::ASTTemplateBase(std::vector<HAZE_STRING>& templateTypes)
	: m_TemplateTypes(std::move(templateTypes))
{
}

ASTTemplateBase::~ASTTemplateBase()
{
}
