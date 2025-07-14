#pragma once

#include "HazeHeader.h"

class ASTBase;
class ASTClassFunctionSection;
class ASTFunctionDefine;

// 不允许菱形继承
class ASTClass
{
public:
	ASTClass(Compiler* compiler, /*const SourceLocation& Location,*/ HString& name, V_Array<HString>& parentClass,
		V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>>& data, Unique<ASTClassFunctionSection>& functionSection);
	
	~ASTClass();

	void CodeGen();

private:
	Compiler* m_Compiler;

	HString m_ClassName;
	V_Array<HString> m_ParentClasses;
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>> m_ClassDatas;
	Unique<ASTClassFunctionSection> m_ClassFunctionSection;
};

class ASTClassDefine
{
public:
	ASTClassDefine(Compiler* compiler, /*const SourceLocation& Location,*/ HString& name, 
		V_Array<V_Array<Unique<ASTBase>>>& data, V_Array<Unique<ASTFunctionDefine>>& function);

	~ASTClassDefine();

	void CodeGen();
private:
	Compiler* m_Compiler;

	HString m_ClassName;
	V_Array<V_Array<Unique<ASTBase>>> m_ClassDatas;
	V_Array<Unique<ASTFunctionDefine>> m_ClassFunctions;
};
