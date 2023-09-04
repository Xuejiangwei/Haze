#include "MemoryPage.h"
#include "MemoryBlock.h"

#define DEFAULT_UNIT_NUM 32

MemoryPage::MemoryPage(uint64 PageByteSize, uint64 BlockUnitSize)
{
	PageInfo.PageByteSize = PageByteSize;
	PageInfo.UnitSize = BlockUnitSize;

	PageInfo.MemBlock = std::make_unique<MemoryBlock>(malloc(PageByteSize), PageByteSize, BlockUnitSize);
}

MemoryPage::~MemoryPage()
{
}

void* MemoryPage::TryAlloca(uint64 Size)
{
	void* Ret = PageInfo.MemBlock->Alloca(Size);

	if (!Ret && PageInfo.NextPage)
	{
		Ret = PageInfo.NextPage->TryAlloca(Size);
	}

	/*if (!Ret && Block)
	{
		if ((uint64)Block->GetTailAddress() < (uint64)PageInfo.HeadBlock->GetHeadAddress() + PageInfo.PageByteSize)
		{
			uint64 BlockUnitNum = DEFAULT_UNIT_NUM;

			auto NextBlock = std::make_unique<MemoryBlock>(Block->GetTailAddress(), BlockUnitNum, PageInfo.UnitSize);
			Block->SetNextBlock(NextBlock);
		}
	}*/

	return Ret;
}

MemoryBlock* MemoryPage::GetLastBlock()
{
	MemoryBlock* Ret = PageInfo.MemBlock.get();
	while (Ret->GetNextBlock())
	{
		Ret = Ret = GetLastBlock();
	}

	return Ret;
}

void MemoryPage::SetNextPage(std::unique_ptr<MemoryPage> Page)
{
	PageInfo.NextPage = std::move(Page);
}

bool MemoryPage::IsInPage(void* Address)
{
	return PageInfo.MemBlock->GetHeadAddress() <= Address && Address < PageInfo.MemBlock->GetTailAddress();
}