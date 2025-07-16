#pragma once
#include "CompilerValue.h"

class CompilerClass;

class CompilerElementValue : public CompilerValue
{
public:
	explicit CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, Share<CompilerValue> element);

	// Dynamica Class 使用这个构造函数
	explicit CompilerElementValue(CompilerModule* compilerModule, Share<CompilerValue> parent, const HString& elementName);

	virtual ~CompilerElementValue() override;

	virtual HazeValueType GetBaseType() const { return m_Element->GetBaseType(); }

	HazeVariableType GetParentBaseType() const { return m_Parent->GetVariableType(); }

	Share<CompilerValue> GetParent() const { return m_Parent; }

	Share<CompilerValue> GetElement() const { return m_Element; }

	const HString* GetElementName() const { return m_ElementName.get(); }

	Share<CompilerValue> CreateGetFunctionCall();

	CompilerClass* GetRealClass() const;

private:
	Share<CompilerValue> m_Parent;
	Share<CompilerValue> m_Element;		//数组的偏移或类的成员
	Unique<HString> m_ElementName;		//DynamicClass成员名
};

