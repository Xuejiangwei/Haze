#include "GarbageCollection.h"

#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeMemory.h"
#include "MemoryPage.h"
#include "MemoryBlock.h"

static GC_Array GC_Arrays;

GarbageCollection::GarbageCollection(HazeVM* VM) : VM(VM)
{
}

GarbageCollection::~GarbageCollection()
{
}

void GarbageCollection::AddToRoot(void*)
{
}

//https://juejin.cn/post/6966954993869914119
//https://juejin.cn/post/6968400262629163038

//https://zhuanlan.zhihu.com/p/41023320
//https://zhuanlan.zhihu.com/p/41398507


void GarbageCollection::MarkClassMember(std::vector<std::pair<uint64, HazeValueType>>& Vector_MarkAddressBase,
	std::vector<std::pair<uint64, ClassData*>>& Vector_MarkAddressClass, const HazeDefineType& VarType, char* BaseAddress)
{
	uint64 Address = 0;
	auto ClassData = VM->FindClass(VarType.CustomName);
	for (size_t i = 0; i < ClassData->Vector_Member.size(); i++)
	{
		auto& Member = ClassData->Vector_Member[i];
		if (Member.MemberData.Type.PrimaryType == HazeValueType::PointerBase)
		{
			memcpy(&Address, BaseAddress + Member.Offset, sizeof(Address));
			Vector_MarkAddressBase.push_back({ Address, Member.MemberData.Type.SecondaryType });
		}
		else if (VarType.PrimaryType == HazeValueType::PointerClass)
		{
			memcpy(&Address, BaseAddress + Member.Offset, sizeof(Address));
			Vector_MarkAddressClass.push_back({ Address, VM->FindClass(Member.MemberData.Type.CustomName) });
		}
		else if (VarType.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, Member.MemberData.Type, BaseAddress + Member.Offset);
		}

	}
}

void GarbageCollection::Mark()
{
	//根节点内存有 静态变量、栈、寄存器等
	std::vector<std::pair<uint64, HazeValueType>> Vector_MarkAddressBase;
	std::vector<std::pair<uint64, ClassData*>> Vector_MarkAddressClass;
	uint64 Address = 0;

	for (auto& It : VM->Vector_GlobalData)
	{
		if (It.Type.PrimaryType == HazeValueType::PointerBase)
		{
			Vector_MarkAddressBase.push_back({ (uint64)It.Value.Value.Pointer, It.Type.SecondaryType });
		}
		else if (It.Type.PrimaryType == HazeValueType::PointerClass)
		{
			Vector_MarkAddressClass.push_back({ (uint64)It.Value.Value.Pointer, VM->FindClass(It.Type.CustomName) });
		}
		else if (It.Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, It.Type, (char*)&It.Value);
		}
	}

	for (auto& It : VM->VMStack->HashMap_VirtualRegister)
	{
		if (It.second.Type.PrimaryType == HazeValueType::PointerBase)
		{
			memcpy(&Address, It.second.Data.begin()._Unwrapped(), sizeof(Address));
			Vector_MarkAddressBase.push_back({ Address, It.second.Type.SecondaryType });
		}
		else if (It.second.Type.PrimaryType == HazeValueType::PointerClass)
		{
			memcpy(&Address, It.second.Data.begin()._Unwrapped(), sizeof(Address));
			Vector_MarkAddressClass.push_back({ Address, VM->FindClass(It.second.Type.CustomName) });
		}
		else if (It.second.Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, It.second.Type, It.second.Data.begin()._Unwrapped());
		}
	}

	for (size_t i = 0; i < VM->VMStack->Stack_Frame.size(); i++)
	{
		for (auto& Var : VM->VMStack->Stack_Frame[i].FunctionInfo->Vector_Variable)
		{
			if (Var.Variable.Type.PrimaryType == HazeValueType::PointerBase)
			{
				memcpy(&Address, &VM->VMStack->Stack_Main[VM->VMStack->Stack_Frame[i].EBP + Var.Offset], sizeof(Address));
				Vector_MarkAddressBase.push_back({ Address, Var.Variable.Type.SecondaryType });
			}
			else if (Var.Variable.Type.PrimaryType == HazeValueType::PointerClass)
			{
				memcpy(&Address, &VM->VMStack->Stack_Main[VM->VMStack->Stack_Frame[i].EBP + Var.Offset], sizeof(Address));
				Vector_MarkAddressClass.push_back({ Address, VM->FindClass(Var.Variable.Type.CustomName) });
			}
			else if (Var.Variable.Type.PrimaryType == HazeValueType::Class)
			{
				MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, Var.Variable.Type, &VM->VMStack->Stack_Main[VM->VMStack->Stack_Frame[i].EBP + Var.Offset]);
			}
		}
	}

	//遍历完根节点后，再遍历 Vector_MarkAddress
	uint64 BaseIndex = 0;
	uint64 ClassIndex = 0;
	while (BaseIndex < Vector_MarkAddressBase.size() || ClassIndex < Vector_MarkAddressClass.size())
	{
		if (BaseIndex < Vector_MarkAddressBase.size())
		{
			MarkArrayBaseIndex(Vector_MarkAddressBase, Vector_MarkAddressClass, BaseIndex++);
		}

		if (ClassIndex < Vector_MarkAddressClass.size())
		{
			MarkArrayClassIndex(Vector_MarkAddressBase, Vector_MarkAddressClass, ClassIndex++);
		}
	}

	Vector_KeepMemory.clear();
	for (size_t i = 0; i < Vector_MarkAddressBase.size(); i++)
	{
		Vector_KeepMemory.push_back((void*)Vector_MarkAddressBase[i].first);
	}

	for (size_t i = 0; i < Vector_MarkAddressClass.size(); i++)
	{
		Vector_KeepMemory.push_back((void*)Vector_MarkAddressClass[i].first);
	}
}

void GarbageCollection::Sweep()
{
	std::vector<void*> KeepMemory;

	for (auto& Iter : HazeMemory::GetMemory()->HashMap_Page)
	{
		auto Page = Iter.second.get();
		while (Page)
		{
			memset(Page->PageInfo.HeadBlock->BlockInfo.MemoryKeepSignal.begin()._Unwrapped(), 0, Page->PageInfo.HeadBlock->BlockInfo.MemoryKeepSignal.size());

			for (size_t i = 0; i < Vector_KeepMemory.size(); i++)
			{
				if (Page->IsInPage(Vector_KeepMemory[i]))
				{
					uint64 Index = ((uint64)Vector_KeepMemory[i] - (uint64)Page->PageInfo.HeadBlock->GetHeadAddress()) / Page->PageInfo.HeadBlock->BlockInfo.UnitSize;
					Page->PageInfo.HeadBlock->BlockInfo.MemoryKeepSignal[Index] = 1;
				}
			}

			Page->PageInfo.HeadBlock->CollectionMemory();
			Page = Page->PageInfo.NextPage.get();
		}
	}
}

void GarbageCollection::ForceGC()
{
	Mark();
	Sweep();
}

void GarbageCollection::MarkArrayBaseIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index)
{
	
}

void GarbageCollection::MarkArrayClassIndex(std::vector<std::pair<uint64, HazeValueType>>& ArrayBase, std::vector<std::pair<uint64, ClassData*>>& ArrayClass, uint64 Index)
{
}