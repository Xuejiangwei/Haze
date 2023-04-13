#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerFunction;

class HazeBaseBlock : public std::enable_shared_from_this<HazeBaseBlock>
{
public:
	explicit HazeBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* ParentFunction, HazeBaseBlock* ParentBlock);

	std::shared_ptr<HazeCompilerValue> CreateAlloce(const HazeDefineVariable& Define, int Count);

	std::shared_ptr<HazeCompilerValue> CreateTempAlloce(const HazeDefineVariable& Define);

	const std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& GetAllocaList() const { return Vector_Alloca; }

	HazeBaseBlock* GetParentBlock() const { return ParentBlock; }

	const HAZE_STRING& GetName() const { return Name; }

	size_t GetIRCodeSize() { return Vector_IRCode.size() - 1; }

	const std::vector<HAZE_STRING>& GetIRCode() const { return Vector_IRCode; }

	bool FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	const bool BlockIsFinish() const { return IsFinish; }

	void AddChildBlock(std::shared_ptr<HazeBaseBlock> Block);

	void SetJmpOut();

	void FinishBlock(std::shared_ptr<HazeBaseBlock> MoveFinishPopBlock = nullptr, bool JmpOut = true);

	void GenI_Code_Alloca(HAZE_OFSTREAM& OFStream);

	void GenI_Code(HAZE_OFSTREAM& OFStream);

public:
	static std::shared_ptr<HazeBaseBlock> CreateBaseBlock(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerFunction> Parent, std::shared_ptr<HazeBaseBlock> ParentBlock);

	void PushIRCode(const HAZE_STRING& Code);

	void MergeJmpIRCode(std::shared_ptr<HazeBaseBlock> BB);

	//void CopyIRCode(std::shared_ptr<HazeBaseBlock> BB);

	void ClearTempIRCode();

public:
	HazeBaseBlock(const HazeBaseBlock&) = delete;

	HazeBaseBlock& operator=(const HazeBaseBlock&) = delete;

	~HazeBaseBlock();

private:
	HAZE_STRING Name;
	HazeCompilerFunction* ParentFunction;

	HazeBaseBlock* ParentBlock;
	std::list<std::shared_ptr<HazeBaseBlock>> List_ChildBlock;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> Vector_Alloca;

	std::vector<HAZE_STRING> Vector_IRCode;

	bool IsFinish;
};
