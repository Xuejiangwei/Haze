#pragma once

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerClass;

class HazeCompilerClassValue : public HazeCompilerValue
{
public:
	friend class HazeCompilerClass;

	explicit HazeCompilerClassValue(HazeCompilerModule* Module, const HazeDefineType& DefineType, HazeDataDesc Scope, int Count);

	virtual ~HazeCompilerClassValue() override;

	virtual uint32 GetSize() override;

	HazeCompilerClass* GetOwnerClass() { return OwnerClass; };

	const HAZE_STRING& GetOwnerClassName();

	std::shared_ptr<HazeCompilerValue> GetMember(const HAZE_STRING& Name);

	void GetMemberName(const std::shared_ptr<HazeCompilerValue>& MemberValue, HAZE_STRING& OutName);

private:
	HazeCompilerClass* OwnerClass;
	
	std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> Vector_Data;
};

