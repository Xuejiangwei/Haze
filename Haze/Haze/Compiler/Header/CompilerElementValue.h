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

	HazeValueType GetParentBaseType() const { return m_Parent->GetValueType().PrimaryType; }

	Share<CompilerValue> GetParent() const { return m_Parent; }

	Share<CompilerValue> GetElement() const { return m_Element; }

	const HString* GetElementName() const { return m_ElementName.get(); }

	Share<CompilerValue> CreateGetFunctionCall();

	CompilerClass* GetRealClass() const;

	x_uint64 TryGetArrayDimension() const { return m_ValueTypeArrayDimension; }

private:
	Share<CompilerValue> m_Parent;
	Share<CompilerValue> m_Element;		//数组的偏移或类的成员
	Unique<HString> m_ElementName;		//DynamicClass成员名
	x_uint64  m_ValueTypeArrayDimension;
};

