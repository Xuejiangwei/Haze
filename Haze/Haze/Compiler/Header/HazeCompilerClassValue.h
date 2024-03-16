#pragma once

#include "HazeHeader.h"
#include "HazeCompilerValue.h"

class HazeCompilerClass;

class HazeCompilerClassValue : public HazeCompilerValue
{
public:
	friend class HazeCompilerClass;

	explicit HazeCompilerClassValue(HazeCompilerModule* compilerModule, const HazeDefineType& defineType, HazeVariableScope scope,
		HazeDataDesc desc, int count);

	virtual ~HazeCompilerClassValue() override;

	virtual uint32 GetSize() override;

	HazeCompilerClass* GetOwnerClass() { return m_OwnerClass; };

	const HAZE_STRING& GetOwnerClassName();

	std::shared_ptr<HazeCompilerValue> GetMember(const HAZE_STRING& name);

	void GetMemberName(const std::shared_ptr<HazeCompilerValue>& memberValue, HAZE_STRING& outName);

	void GetMemberName(const HazeCompilerValue* memberValue, HAZE_STRING& outName);

private:
	HazeCompilerClass* m_OwnerClass;

	std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> m_Data;
};
