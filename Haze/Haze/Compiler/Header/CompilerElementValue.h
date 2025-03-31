#pragma once
#include "CompilerValue.h"

class CompilerElementValue : public CompilerValue
{
public:
	explicit CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, Share<CompilerValue> element);

	// Dynamica Class ʹ��������캯��
	explicit CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, const HString& elementName);

	virtual ~CompilerElementValue() override;

	HazeValueType GetParentBaseType() const { return m_Parent->GetValueType().PrimaryType; }

	Share<CompilerValue> GetParent() const { return m_Parent; }

	Share<CompilerValue> GetElement() const { return m_Element; }

	const HString* GetElementName() const { return m_ElementName.get(); }

	Share<CompilerValue> CreateGetFunctionCall();

private:
	Share<CompilerValue> m_Parent;
	Share<CompilerValue> m_Element;		//�����ƫ�ƻ���ĳ�Ա
	Unique<HString> m_ElementName;		//DynamicClass��Ա��
};

