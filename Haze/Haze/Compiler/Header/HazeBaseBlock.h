#pragma once

class HazeCompilerValue;
class HazeCompilerFunction;

class HazeBaseBlock : public std::enable_shared_from_this<HazeBaseBlock>
{
public:
	explicit HazeBaseBlock(const HString& name, HazeCompilerFunction* parentFunction, HazeBaseBlock* parentBlock);

	~HazeBaseBlock();

	HazeBaseBlock(const HazeBaseBlock&) = delete;

	HazeBaseBlock& operator=(const HazeBaseBlock&) = delete;

	Share<HazeCompilerValue> CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, 
		Share<HazeCompilerValue> refValue = nullptr, V_Array<Share<HazeCompilerValue>> m_ArraySize = {},
		V_Array<HazeDefineType>* Params = nullptr);

	Share<HazeBaseBlock> GetShared() { return shared_from_this(); }

	const V_Array<Pair<HString, Share<HazeCompilerValue>>>& GetAllocaList() const { return m_Allocas; }

	HazeBaseBlock* GetParentBlock() const { return m_ParentBlock; }

	const HString& GetName() const { return m_Name; }

	uint64 GetIRCodeSize() { return m_IRCodes.size() - 1; }

	const V_Array<HString>& GetIRCode() const { return m_IRCodes; }

	bool FindLocalVariableName(const Share<HazeCompilerValue>& value, HString& outName);

	bool FindLocalVariableName(const HazeCompilerValue* value, HString& outName, bool getOffset = false, V_Array<uint64>* offsets = nullptr);

	void SetLoopEnd(HazeBaseBlock* block) { m_LoopEndBlock = block; }

	void SetLoopStep(HazeBaseBlock* block) { m_LoopStepBlock = block; }

	HazeBaseBlock* GetLoopEnd() const { return m_LoopEndBlock; }

	HazeBaseBlock* GetLoopStep() const { return m_LoopStepBlock; }

	HazeBaseBlock* FindLoopBlock();

	bool IsLoopBlock() const;

	void AddChildBlock(Share<HazeBaseBlock> block);

	void GenI_Code(HAZE_STRING_STREAM& hss);

	void ClearLocalVariable();

public:
	static Share<HazeBaseBlock> CreateBaseBlock(const HString& name,
		Share<HazeCompilerFunction> parent, Share<HazeBaseBlock> parentBlock);

	void PushIRCode(const HString& code);

private:
	HString m_Name;
	HazeCompilerFunction* m_ParentFunction;

	HazeBaseBlock* m_ParentBlock;
	HazeBaseBlock* m_LoopEndBlock;
	HazeBaseBlock* m_LoopStepBlock;

	List<Share<HazeBaseBlock>> m_ChildBlocks;

	V_Array<Pair<HString, Share<HazeCompilerValue>>> m_Allocas;

	V_Array<HString> m_IRCodes;
};
