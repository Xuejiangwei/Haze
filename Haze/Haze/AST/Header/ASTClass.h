#pragma once

#include "Haze.h"

class ASTBase;
class ASTClassFunctionSection;
class ASTFunctionDefine;

class ASTClass
{
public:
	ASTClass(HazeCompiler* Compiler, const SourceLocation& Location, HAZE_STRING& Name, std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>>& Data,
		std::unique_ptr<ASTClassFunctionSection>& FunctionSection);
	~ASTClass();

	void CodeGen();
private:
	HazeCompiler* Compiler;

	HAZE_STRING ClassName;
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>> Vector_ClassData;
	std::unique_ptr<ASTClassFunctionSection> ClassFunctionSection;
};

class ASTClassDefine
{
public:
	ASTClassDefine(HazeCompiler* Compiler, const SourceLocation& Location, HAZE_STRING& Name, std::vector<std::vector<std::unique_ptr<ASTBase>>>& Data, std::vector<std::unique_ptr<ASTFunctionDefine>>& Function);
	~ASTClassDefine();

	void CodeGen();
private:
	HazeCompiler* Compiler;

	HAZE_STRING ClassName;
	std::vector<std::vector<std::unique_ptr<ASTBase>>> ClassData;
	std::vector<std::unique_ptr<ASTFunctionDefine>> ClassFunction;
};
