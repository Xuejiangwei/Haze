#pragma once

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerClass;

class HazeCompilerClassValue : public HazeCompilerValue
{
public:
	HazeCompilerClassValue(std::shared_ptr<HazeCompilerClass> OwnerClass);
	~HazeCompilerClassValue();

private:
	std::shared_ptr<HazeCompilerClass> OwnerClass;
};

