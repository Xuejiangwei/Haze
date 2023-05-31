#pragma once

#include <memory>
#include "Haze.h"

class HazeCompilerValue;
class HazeCompilerFunction;

class HazeBaseBlock : public std::enable_shared_from_this<HazeBaseBlock>
{
public:
	explicit HazeBaseBlock(const HAZE_STRING& Name, HazeCompilerFunction* ParentFunction, HazeBaseBlock* ParentBlock);

	HazeBaseBlock(const HazeBaseBlock&) = delete;

	HazeBaseBlock& operator=(const HazeBaseBlock&) = delete;

	~HazeBaseBlock();

	std::shared_ptr<HazeCompilerValue> CreateAlloce(const HazeDefineVariable& Define, int Count, std::shared_ptr<HazeCompilerValue> RefValue = nullptr, 
		std::vector<std::shared_ptr<HazeCompilerValue>> ArraySize = {}, std::vector<HazeDefineType>*Vector_Param = nullptr);

	std::shared_ptr<HazeBaseBlock> GetShared() { return shared_from_this(); }

	const std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>& GetAllocaList() const { return Vector_Alloca; }

	HazeBaseBlock* GetParentBlock() const { return ParentBlock; }

	const HAZE_STRING& GetName() const { return Name; }

	uint64 GetIRCodeSize() { return Vector_IRCode.size() - 1; }

	const std::vector<HAZE_STRING>& GetIRCode() const { return Vector_IRCode; }

	bool FindLocalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	bool FindLocalVariableName(const HazeCompilerValue* Value, HAZE_STRING& OutName);

	void SetLoopEnd(HazeBaseBlock* Block) { LoopEndBlock = Block; }

	void SetLoopStep(HazeBaseBlock* Block) { LoopStepBlock = Block; }

	HazeBaseBlock* GetLoopEnd() const { return LoopEndBlock; }

	HazeBaseBlock* GetLoopStep() const { return LoopStepBlock; }

	HazeBaseBlock* FindLoopBlock();

	bool IsLoopBlock() const;

	void AddChildBlock(std::shared_ptr<HazeBaseBlock> Block);

	void GenI_Code(HAZE_STRING_STREAM& SStream);

	void ClearLocalVariable();

public:
	static std::shared_ptr<HazeBaseBlock> CreateBaseBlock(const HAZE_STRING& Name, std::shared_ptr<HazeCompilerFunction> Parent, std::shared_ptr<HazeBaseBlock> ParentBlock);

	void PushIRCode(const HAZE_STRING& Code);

private:
	HAZE_STRING Name;
	HazeCompilerFunction* ParentFunction;

	HazeBaseBlock* ParentBlock;

	HazeBaseBlock* LoopEndBlock;
	HazeBaseBlock* LoopStepBlock;
	
	std::list<std::shared_ptr<HazeBaseBlock>> List_ChildBlock;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> Vector_Alloca;

	std::vector<HAZE_STRING> Vector_IRCode;
};
