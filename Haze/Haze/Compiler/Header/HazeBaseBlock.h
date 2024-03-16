#pragma once

#include <memory>
#include "HazeHeader.h"

class HazeCompilerValue;
class HazeCompilerFunction;

class HazeBaseBlock : public std::enable_shared_from_this<HazeBaseBlock>
{
public:
	explicit HazeBaseBlock(const HAZE_STRING& name, HazeCompilerFunction* parentFunction, HazeBaseBlock* parentBlock);

	~HazeBaseBlock();

	HazeBaseBlock(const HazeBaseBlock&) = delete;

	HazeBaseBlock& operator=(const HazeBaseBlock&) = delete;

	std::shared_ptr<HazeCompilerValue> CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, 
		std::shared_ptr<HazeCompilerValue> refValue = nullptr, std::vector<std::shared_ptr<HazeCompilerValue>> m_ArraySize = {},
		std::vector<HazeDefineType>* Params = nullptr);

	std::shared_ptr<HazeBaseBlock> GetShared() { return shared_from_this(); }

	const std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& GetAllocaList() const { return m_Allocas; }

	HazeBaseBlock* GetParentBlock() const { return m_ParentBlock; }

	const HAZE_STRING& GetName() const { return m_Name; }

	uint64 GetIRCodeSize() { return m_IRCodes.size() - 1; }

	const std::vector<HAZE_STRING>& GetIRCode() const { return m_IRCodes; }

	bool FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName);

	bool FindLocalVariableName(const HazeCompilerValue* value, HAZE_STRING& outName);

	void SetLoopEnd(HazeBaseBlock* block) { m_LoopEndBlock = block; }

	void SetLoopStep(HazeBaseBlock* block) { m_LoopStepBlock = block; }

	HazeBaseBlock* GetLoopEnd() const { return m_LoopEndBlock; }

	HazeBaseBlock* GetLoopStep() const { return m_LoopStepBlock; }

	HazeBaseBlock* FindLoopBlock();

	bool IsLoopBlock() const;

	void AddChildBlock(std::shared_ptr<HazeBaseBlock> block);

	void GenI_Code(HAZE_STRING_STREAM& hss);

	void ClearLocalVariable();

public:
	static std::shared_ptr<HazeBaseBlock> CreateBaseBlock(const HAZE_STRING& name,
		std::shared_ptr<HazeCompilerFunction> parent, std::shared_ptr<HazeBaseBlock> parentBlock);

	void PushIRCode(const HAZE_STRING& code);

private:
	HAZE_STRING m_Name;
	HazeCompilerFunction* m_ParentFunction;

	HazeBaseBlock* m_ParentBlock;
	HazeBaseBlock* m_LoopEndBlock;
	HazeBaseBlock* m_LoopStepBlock;

	std::list<std::shared_ptr<HazeBaseBlock>> m_ChildBlocks;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> m_Allocas;

	std::vector<HAZE_STRING> m_IRCodes;
};
