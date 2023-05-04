#include "GarbageCollection.h"

#include "HazeVM.h"
#include "HazeStack.h"

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
	std::vector<std::pair<uint64, ClassData*>>& Vector_MarkAddressClass, const HazeDefineVariable& Var, int Offset)
{
	uint64 Address = 0;
	//int Offset = Offset;
	auto ClassData = VM->FindClass(Var.Type.CustomName);
	for (size_t i = 0; i < ClassData->Vector_Member.size(); i++)
	{
		if (ClassData->Vector_Member[i].Type.PrimaryType == HazeValueType::PointerBase)
		{
			memcpy(&Address, &VM->VMStack->Stack_Main[VM->VMStack->Stack_EBP[i + 1] + Offset], sizeof(Address));
			Vector_MarkAddressBase.push_back({ Address, ClassData->Vector_Member[i].Type.SecondaryType });
		}
		else if (Var.Type.PrimaryType == HazeValueType::PointerClass)
		{
			memcpy(&Address, &VM->VMStack->Stack_Main[VM->VMStack->Stack_EBP[i + 1] + Offset], sizeof(Address));
			Vector_MarkAddressClass.push_back({ Address, VM->FindClass(ClassData->Vector_Member[i].Type.CustomName) });
		}
		else if (Var.Type.PrimaryType == HazeValueType::Class)
		{
			MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, ClassData->Vector_Member[i], Offset);
		}

		Offset += GetSizeByType(ClassData->Vector_Member[i].Type, VM);
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
		if (!It.GetType().CustomName.empty())
		{
			
		}
		else if (It.GetType().PrimaryType == HazeValueType::PointerBase)
		{
			
		}
	}

	for (size_t i = 0; i < VM->VMStack->Stack_Frame.size(); i++)
	{
		for (auto& Var : VM->VMStack->Stack_Frame[i].FunctionInfo->Vector_Variable)
		{
			if (Var.Variable.Type.PrimaryType == HazeValueType::PointerBase)
			{
				memcpy(&Address, &VM->VMStack->Stack_Main[VM->VMStack->Stack_EBP[i + 1] + Var.Offset], sizeof(Address));
				Vector_MarkAddressBase.push_back({ Address, Var.Variable.Type.SecondaryType });
			}
			else if (Var.Variable.Type.PrimaryType == HazeValueType::PointerClass)
			{
				memcpy(&Address, &VM->VMStack->Stack_Main[VM->VMStack->Stack_EBP[i + 1] + Var.Offset], sizeof(Address));
				Vector_MarkAddressClass.push_back({ Address, VM->FindClass(Var.Variable.Type.CustomName) });
			}
			else if (Var.Variable.Type.PrimaryType == HazeValueType::Class)
			{
				std::vector<std::pair<uint64, HazeValueType>> Vector_MarkAddressBase;
				MarkClassMember(Vector_MarkAddressBase, Vector_MarkAddressClass, Var.Variable, Var.Offset);
			}
		}
	}

	//遍历完根节点后，再遍历 Vector_MarkAddress
	uint64 Index = 0;
	while (Index < Vector_MarkAddressBase.size())
	{
		//MarkArrayIndex(Vector_MarkAddressBase, Index++);
	}
}

void GarbageCollection::Sweep()
{
	
}

void GarbageCollection::MarkArrayIndex(std::vector<uint64>& Array, uint64 Index)
{

}
