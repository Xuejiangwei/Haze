#pragma once
#include "HazeHeader.h"
#include "ASTTemplateBase.h"

class HazeCompiler;
class ASTBase;
class ASTClassFunctionSection;

class ASTTemplateClass : public ASTTemplateBase
{
public:
	ASTTemplateClass(HazeCompiler* compiler, HAZE_STRING& name, std::vector<HAZE_STRING>& parentClass,
		std::vector<HAZE_STRING>& templateTypes,
		std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>>& data, 
		std::unique_ptr<ASTClassFunctionSection>& functionSection);

	~ASTTemplateClass();

	virtual void CodeGen() override;

private:
	HazeCompiler* m_Compiler;

	HAZE_STRING m_ClassName;
	std::vector<HAZE_STRING> m_ParentClasses;
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> m_ClassDatas;
	std::unique_ptr<ASTClassFunctionSection> m_ClassFunctionSection;
};