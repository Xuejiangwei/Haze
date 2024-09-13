#pragma once
#include "CompilerValue.h"

class CompilerElementValue : public CompilerValue
{
public:
	explicit CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, Share<CompilerValue> element);

	virtual ~CompilerElementValue() override;

	HazeValueType GetParentBaseType() const { return m_Parent->GetValueType().PrimaryType; }

	Share<CompilerValue> GetParent() const { return m_Parent; }

	Share<CompilerValue> GetElement() const { return m_Element; }

	Share<CompilerValue> CreateGetFunctionCall();

private:
	Share<CompilerValue> m_Parent;
	Share<CompilerValue> m_Element;		//数组的偏移或类的成员
};

