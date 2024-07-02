#pragma once

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <fstream>

#include "HazeHeader.h"
#include "HazeCompilerValue.h"

class HazeCompilerModule;
class HazeCompilerClass;
class HazeBaseBlock;

class HazeCompilerFunction
{
public:
	friend class HazeCompiler;
	friend class HazeCompilerModule;

	HazeCompilerFunction(HazeCompilerModule* compilerModule, const HAZE_STRING& name, HazeDefineType& type,
		std::vector<HazeDefineVariable>& params, HazeCompilerClass* compilerClass = nullptr);

	~HazeCompilerFunction();

	void SetStartEndLine(uint32 startLine, uint32 endLine);

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& VariableName);

	const HAZE_STRING& GetName() const { return m_Name; }

	const HazeDefineType& GetFunctionType() const { return m_Type; }

	HazeCompilerModule* GetModule() const { return m_Module; }

	HazeCompilerClass* GetClass() const { return m_OwnerClass; }

	std::shared_ptr<HazeBaseBlock> GetEntryBlock() { return m_EntryBlock; }

	void FunctionFinish();

	void GenI_Code(HAZE_STRING_STREAM& hss);

	HAZE_STRING GenDafaultBlockName();

	HAZE_STRING GenIfThenBlockName();

	HAZE_STRING GenElseBlockName();

	HAZE_STRING GenLoopBlockName();

	HAZE_STRING GenWhileBlockName();

	HAZE_STRING GenForBlockName();

	HAZE_STRING GenForConditionBlockName();

	HAZE_STRING GenForStepBlockName();

	bool FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName);

	bool FindLocalVariableName(const HazeCompilerValue* value, HAZE_STRING& outName);

	bool HasParam() const { return m_Params.size() > 0; }

	void AddLocalVariable(std::shared_ptr<HazeCompilerValue> value, int line);

private:
	void AddFunctionParam(const HazeDefineVariable& variable);

	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(const HazeDefineVariable& variable, int line,
		std::shared_ptr<HazeCompilerValue> refValue = nullptr,
		std::vector<std::shared_ptr<HazeCompilerValue>> arraySize = {},
		std::vector<HazeDefineType>* params = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateNew(const HazeDefineType& data, std::shared_ptr<HazeCompilerValue> countValue);

	void InitEntryBlock(std::shared_ptr<HazeBaseBlock> block) { m_EntryBlock = block; }

private:
	HazeCompilerModule* m_Module;
	HazeCompilerClass* m_OwnerClass;

	HAZE_STRING m_Name;
	HazeDefineType m_Type;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> m_Params;	//从右到左加入参数

	std::vector<std::pair<std::shared_ptr<HazeCompilerValue>, int>> m_LocalVariables;

	std::shared_ptr<HazeBaseBlock> m_EntryBlock;

	int m_CurrBlockCount;
	int m_CurrVariableCount;
	
	uint32 m_StartLine;
	uint32 m_EndLine;
};
