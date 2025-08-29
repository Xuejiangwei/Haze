#pragma once

class CompilerValue;
class CompilerFunction;

//考虑将块的引用及行数信息写入到符号表中
class CompilerBlock : public std::enable_shared_from_this<CompilerBlock>
{
public:
	explicit CompilerBlock(STDString&& name, CompilerFunction* parentFunction, CompilerBlock* parentBlock);

	~CompilerBlock();

	CompilerBlock(const CompilerBlock&) = delete;

	CompilerBlock& operator=(const CompilerBlock&) = delete;

	Share<CompilerValue> CreateAlloce(const HazeDefineVariable& defineVar, int line, int count, HazeVariableScope scope,
		Share<CompilerValue> refValue = nullptr, TemplateDefineTypes* Params = nullptr);

	Share<CompilerBlock> GetShared() { return shared_from_this(); }

	const V_Array<Pair<STDString, Share<CompilerValue>>>& GetAllocaList() const { return m_Allocas; }

	void AddClosureRefValue(Share<CompilerValue> refValue, const STDString& name);

	CompilerBlock* GetParentBlock() const { return m_ParentBlock; }

	const STDString& GetName() const { return m_Name; }

	x_uint64 GetIRCodeSize() { return m_IRCodes.size() - 1; }

	const V_Array<STDString>& GetIRCode() const { return m_IRCodes; }

	bool FindLocalVariableName(const Share<CompilerValue>& value, HStringView& outName);

	void SetLoopEnd(CompilerBlock* block) { m_LoopEndBlock = block; }

	void SetLoopStep(CompilerBlock* block) { m_LoopStepBlock = block; }

	CompilerBlock* GetLoopEnd() const { return m_LoopEndBlock; }

	CompilerBlock* GetLoopStep() const { return m_LoopStepBlock; }

	CompilerBlock* FindLoopBlock();

	bool IsLoopBlock() const;

	void AddChildBlock(Share<CompilerBlock> block);

	void AddPredecessor(Share<CompilerBlock> block);
	void AddSuccessor(Share<CompilerBlock> block1, Share<CompilerBlock> block2 = nullptr);

	void GenI_Code(HAZE_STRING_STREAM& hss, HashMap<CompilerBlock*, x_uint64>& blockIndex);
	void GenI_Code_FlowGraph(HAZE_STRING_STREAM& hss, HashMap<CompilerBlock*, x_uint64>& blockIndex);

	void ClearLocalVariable();

public:
	static Share<CompilerBlock> CreateBaseBlock(STDString&& name, Share<CompilerFunction> parent, Share<CompilerBlock> parentBlock);
	static Share<CompilerBlock> CreateBaseBlock(STDString&& name, CompilerFunction* parent, CompilerBlock* parentBlock);

	void PushIRCode(const STDString& code);

private:
	STDString m_Name;
	CompilerFunction* m_ParentFunction;

	CompilerBlock* m_ParentBlock;
	CompilerBlock* m_LoopEndBlock;
	CompilerBlock* m_LoopStepBlock;

	List<Share<CompilerBlock>> m_ChildBlocks;
	List<Share<CompilerBlock>> m_Predecessors;
	List<Share<CompilerBlock>> m_Successors;

	V_Array<Pair<STDString, Share<CompilerValue>>> m_Allocas;

	V_Array<STDString> m_IRCodes;
};
