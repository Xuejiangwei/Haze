#pragma once

class CompilerValue;
class CompilerFunction;

//考虑将块的引用及行数信息写入到符号表中
class CompilerBlock : public std::enable_shared_from_this<CompilerBlock>
{
public:
	explicit CompilerBlock(const HString& name, CompilerFunction* parentFunction, CompilerBlock* parentBlock);

	~CompilerBlock();

	CompilerBlock(const CompilerBlock&) = delete;

	CompilerBlock& operator=(const CompilerBlock&) = delete;

	Share<CompilerValue> CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, HazeVariableScope scope,
		Share<CompilerValue> refValue = nullptr, x_uint64 arrayDimension = 0, V_Array<HazeDefineType>* Params = nullptr);

	Share<CompilerBlock> GetShared() { return shared_from_this(); }

	const V_Array<Pair<HString, Share<CompilerValue>>>& GetAllocaList() const { return m_Allocas; }

	CompilerBlock* GetParentBlock() const { return m_ParentBlock; }

	const HString& GetName() const { return m_Name; }

	x_uint64 GetIRCodeSize() { return m_IRCodes.size() - 1; }

	const V_Array<HString>& GetIRCode() const { return m_IRCodes; }

	bool FindLocalVariableName(const Share<CompilerValue>& value, HString& outName);

	void SetLoopEnd(CompilerBlock* block) { m_LoopEndBlock = block; }

	void SetLoopStep(CompilerBlock* block) { m_LoopStepBlock = block; }

	CompilerBlock* GetLoopEnd() const { return m_LoopEndBlock; }

	CompilerBlock* GetLoopStep() const { return m_LoopStepBlock; }

	CompilerBlock* FindLoopBlock();

	bool IsLoopBlock() const;

	void AddChildBlock(Share<CompilerBlock> block);

	void GenI_Code(HAZE_STRING_STREAM& hss);

	void ClearLocalVariable();

public:
	static Share<CompilerBlock> CreateBaseBlock(const HString& name,
		Share<CompilerFunction> parent, Share<CompilerBlock> parentBlock);

	void PushIRCode(const HString& code);

private:
	HString m_Name;
	CompilerFunction* m_ParentFunction;

	CompilerBlock* m_ParentBlock;
	CompilerBlock* m_LoopEndBlock;
	CompilerBlock* m_LoopStepBlock;

	List<Share<CompilerBlock>> m_ChildBlocks;

	V_Array<Pair<HString, Share<CompilerValue>>> m_Allocas;

	V_Array<HString> m_IRCodes;
};
