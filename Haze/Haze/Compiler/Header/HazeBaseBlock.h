#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerFunction;

class HazeBaseBlock : public std::enable_shared_from_this<HazeBaseBlock>
{
public:
	explicit HazeBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* Parent);

	std::shared_ptr<HazeCompilerValue> CreateAlloce(const HazeDefineVariable& Define);

	std::shared_ptr<HazeCompilerValue> CreateTempAlloce(const HazeDefineVariable& Define);

	const std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& GetAllocaList() const { return BlockAllocaList; }

	const HAZE_STRING& GetName() const { return Name; }

	size_t GetIRCodeSize() { return Vector_IRCode.size() - 1; }

	const std::vector<HAZE_STRING>& GetIRCode() const { return Vector_IRCode; }

	const bool BlockIsFinish() const { return IsFinish; }

	void SetJmpOut();

	void FinishBlock(std::shared_ptr<HazeBaseBlock> TopBlock, bool JmpOut = true);

public:
	static std::shared_ptr<HazeBaseBlock> CreateBaseBlock(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerFunction> Parent, std::shared_ptr<HazeBaseBlock> InsertBefore = nullptr);

	void PushIRCode(const HAZE_STRING& Code);

	void MergeJmpIRCode(std::shared_ptr<HazeBaseBlock> BB);

	void CopyIRCode(std::shared_ptr<HazeBaseBlock> BB);

	void ClearTempIRCode();

public:
	HazeBaseBlock(const HazeBaseBlock&) = delete;

	HazeBaseBlock& operator=(const HazeBaseBlock&) = delete;

	~HazeBaseBlock();

private:
	HAZE_STRING Name;
	HazeCompilerFunction* ParentFunction;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> BlockAllocaList;

	std::vector<HAZE_STRING> Vector_IRCode;

	bool IsFinish;
};
