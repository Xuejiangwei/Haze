#pragma once

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <fstream>

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerModule;
class HazeCompilerClass;
class HazeBaseBlock;

class HazeCompilerFunction
{
public:
	friend class HazeCompiler;
	friend class HazeCompilerModule;

	HazeCompilerFunction(HazeCompilerModule* m_Module, const HAZE_STRING& m_Name, HazeDefineType& m_Type, std::vector<HazeDefineVariable>& Param, HazeCompilerClass* Class = nullptr);

	~HazeCompilerFunction();

	void SetStartEndLine(uint32 stratLine, uint32 endLine) { m_StartLine = stratLine; m_EndLine = endLine; };

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& VariableName);

	const HAZE_STRING& GetName() const { return m_Name; }

	const HazeDefineType& GetFunctionType() const { return m_Type; }

	HazeCompilerModule* GetModule() const { return m_Module; }

	HazeCompilerClass* GetClass() const { return OwnerClass; }

	std::shared_ptr<HazeBaseBlock> GetEntryBlock() { return EntryBlock; }

	void FunctionFinish();

	void GenI_Code(HAZE_STRING_STREAM& SStream);

	HAZE_STRING GenDafaultBlockName();

	HAZE_STRING GenIfThenBlockName();

	HAZE_STRING GenElseBlockName();

	HAZE_STRING GenLoopBlockName();

	HAZE_STRING GenWhileBlockName();

	HAZE_STRING GenForBlockName();

	HAZE_STRING GenForConditionBlockName();

	HAZE_STRING GenForStepBlockName();

	bool FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	bool FindLocalVariableName(const HazeCompilerValue* Value, HAZE_STRING& OutName);

	void AddLocalVariable(std::shared_ptr<HazeCompilerValue> Value, int Line);

private:
	void AddFunctionParam(const HazeDefineVariable& Variable);

	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(const HazeDefineVariable& Variable, int Line, std::shared_ptr<HazeCompilerValue> RefValue = nullptr,
		std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize = {}, std::vector<HazeDefineType>* Params = nullptr);

	std::shared_ptr<HazeCompilerValue> CreateNew(const HazeDefineType& m_Data);

	void InitEntryBlock(std::shared_ptr<HazeBaseBlock> Block) { EntryBlock = Block; }

private:
	HazeCompilerModule* m_Module;
	HazeCompilerClass* OwnerClass;

	HAZE_STRING m_Name;
	HazeDefineType m_Type;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorParam; //���ҵ���������

	std::vector<std::pair<std::shared_ptr<HazeCompilerValue>, int>> Vector_LocalVariable;

	std::shared_ptr<HazeBaseBlock> EntryBlock;

	int CurrBlockCount;
	int CurrVariableCount;
	
	uint32 m_StartLine;
	uint32 m_EndLine;
};
