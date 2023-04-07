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

	const std::vector<HAZE_STRING>& GetIRCode() const { return IRCode; }

public:
	static std::shared_ptr<HazeBaseBlock> CreateBaseBlock(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerFunction> Parent, std::shared_ptr<HazeBaseBlock> InsertBefore = nullptr);

	void PushIRCode(const HAZE_STRING& Code);

	void ClearTempIRCode();

public:
	HazeBaseBlock(const HazeBaseBlock&) = delete;

	HazeBaseBlock& operator=(const HazeBaseBlock&) = delete;

	~HazeBaseBlock();

private:
	HAZE_STRING Name;
	HazeCompilerFunction* ParentFunction;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> BlockAllocaList;

	std::vector<HAZE_STRING> IRCode;
};
