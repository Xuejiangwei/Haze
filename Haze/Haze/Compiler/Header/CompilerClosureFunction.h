#pragma once
#include "CompilerFunction.h"

class CompilerBlock;

class CompilerClosureFunction : public CompilerFunction
{
public:
	CompilerClosureFunction(CompilerModule* compilerModule, const HString& name, HazeVariableType& type, V_Array<HazeDefineVariable>& params);

	virtual ~CompilerClosureFunction() override;

	int AddRefValue(int variableIndex, Share<CompilerValue> refValue, const HString& name);

	Share<CompilerBlock> GetUpLevelBlock() { return m_UpLevelBlock; }

	void SetUpLevelEntry(Share<CompilerBlock> block) { m_UpLevelBlock = block; }

	void GenI_Code_RefVariable(HAZE_STRING_STREAM& hss);

	Share<CompilerValue> GetLocalVariable(const HString& name, Share<CompilerBlock> startBlock);

private:
	V_Array<Pair<int, int>> m_RefValues; // { 上个函数引用的变量索引, 这个函数变量所在的索引 }
	Share<CompilerBlock> m_UpLevelBlock;
};