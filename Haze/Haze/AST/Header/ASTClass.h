#pragma once

#include "Haze.h"

class HazeVM;
class ASTBase;
class ASTFunctionSection;
class ASTFunctionDefine;

class ASTClass
{
public:
	ASTClass(HazeVM* VM, HAZE_STRING& Name, std::vector<std::vector<std::unique_ptr<ASTBase>>>& Data, std::unique_ptr<ASTFunctionSection>& FunctionSection);
	~ASTClass();

	void CodeGen();
private:
	HazeVM* VM;

	HAZE_STRING ClassName;
	std::vector<std::vector<std::unique_ptr<ASTBase>>> Vector_ClassData;
	std::unique_ptr<ASTFunctionSection> ClassFunctionSection;
};

class ASTClassDefine
{
public:
	ASTClassDefine(HazeVM* VM, HAZE_STRING& Name, std::vector<std::vector<std::unique_ptr<ASTBase>>>& Data, std::vector<std::unique_ptr<ASTFunctionDefine>>& Function);
	~ASTClassDefine();

	void CodeGen();
private:
	HazeVM* VM;

	HAZE_STRING ClassName;
	std::vector<std::vector<std::unique_ptr<ASTBase>>> ClassData;
	std::vector<std::unique_ptr<ASTFunctionDefine>> ClassFunction;
};
