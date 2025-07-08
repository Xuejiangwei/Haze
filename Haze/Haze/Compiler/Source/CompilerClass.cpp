#include "HazePch.h"
#include "CompilerClass.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerFunction.h"
#include "CompilerHelper.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerClassValue.h"

CompilerClass::CompilerClass(CompilerModule* compilerModule, const HString& name, V_Array<CompilerClass*>& parentClass,
	V_Array<Pair<HString, Share<CompilerValue>>>& data)
	: m_Module(compilerModule), m_Name(name), m_ParentClass(Move(parentClass)), m_Data(Move(data))
{
	m_DataSize = 0;
	m_MemberCount = (x_uint32)m_Data.size();
	for (size_t i = 0; i < m_ParentClass.size(); i++)
	{
		m_DataSize += m_ParentClass[i]->GetDataSize();
		m_MemberCount += m_ParentClass[i]->GetMemberCount();
	}

	x_uint32 memberNum = 0;
	Share<CompilerValue> lastMember = nullptr;

	for (size_t i = 0; i < m_Data.size(); i++)
	{
		memberNum++;
		lastMember = m_Data[i].second;
	}

	MemoryAlign(memberNum);
	x_uint32 alignSize = GetAlignSize();
	alignSize = alignSize > HAZE_ALIGN_BYTE ? alignSize : HAZE_ALIGN_BYTE;
	m_DataSize = lastMember ? HAZE_ALIGN(m_Offsets.back() + lastMember->GetSize(), alignSize) + m_DataSize : m_DataSize;
	HAZE_LOG_INFO(H_TEXT("类<%s> DataSize %d\n"), name.c_str(), m_DataSize);
}

CompilerClass::~CompilerClass()
{
}

Share<CompilerFunction> CompilerClass::FindFunction(const HString& functionName, const HString* nameSpace)
{
	if (nameSpace && !nameSpace->empty() && *nameSpace != m_Name)
	{
		for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
		{
			if (m_ParentClass[i]->GetName() == *nameSpace)
			{
				return m_ParentClass[i]->FindFunction(functionName, nameSpace);
			}
		}

		for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
		{
			auto func = m_ParentClass[i]->FindFunction(functionName, nameSpace);
			if (func)
			{
				return func;
			}
		}
	}
	else
	{
		auto iter = m_HashMap_Functions.find(functionName);
		if (iter != m_HashMap_Functions.end())
		{
			return m_Functions[iter->second];
		}

		if (nameSpace && *nameSpace == m_Name)
		{
			return nullptr;
		}

		for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
		{
			auto func = m_ParentClass[i]->FindFunction(functionName, nameSpace);
			if (func)
			{
				return func;
			}
		}
	}

	return nullptr;
}

Share<CompilerFunction> CompilerClass::AddFunction(Share<CompilerFunction>& function)
{
	auto iter = m_HashMap_Functions.find(function->GetName());
	if (iter == m_HashMap_Functions.end())
	{
		m_Functions.push_back(function);
		m_HashMap_Functions[function->GetName()] = (unsigned int)m_Functions.size() - 1;

		return function;
	}

	return m_Functions[iter->second];
}

int CompilerClass::GetMemberIndex(const HString& memberName, const HString* nameSpace)
{
	bool find = false;
	int index = 0;
	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		if (nameSpace && !nameSpace->empty() && *nameSpace != m_ParentClass[i]->GetName())
		{
			index += m_ParentClass[i]->GetMemberCount();
			continue;
		}

		auto idx = m_ParentClass[i]->GetMemberIndex(memberName, nameSpace);
		if (idx >= 0)
		{
			find = true;
			index += idx;
			break;
		}

		index += m_ParentClass[i]->GetMemberCount();
	}

	if (!find)
	{
		for (int i = 0; i < m_Data.size(); i++)
		{
			if (m_Data[i].first == memberName)
			{
				find = true;
				index += i;
			}
		}
	}

	if (!find)
	{
		index = -1;
	}

	return index;
}

int CompilerClass::GetMemberIndex(const V_Array<HString>& classNames, const HString& memberName)
{
	bool find = false;
	int index = 0;

	CompilerClass* findClass = this;
	for (x_uint64 i = 0; i < classNames.size(); i++)
	{
		auto& name = classNames[i];
		for (x_uint64 j = 0; j < findClass->m_ParentClass.size(); j++)
		{
			if (findClass->m_ParentClass[i]->GetName() == name)
			{
				findClass = findClass->m_ParentClass[i];
				find = i == classNames.size() - 1;
				break;
			}

			index += findClass->m_ParentClass[i]->GetMemberCount();
		}
	}
	

	if (find)
	{
		index += findClass->GetMemberIndex(memberName, nullptr);
	}
	else
	{
		HString s;
		for (x_uint64 i = 0; i < classNames.size(); i++)
		{
			s += classNames[i];

			if (i < classNames.size() - 1)
			{
				s += H_TEXT(" ");
			}
		}

		COMPILER_ERR_MODULE_W("未能在类<%s>中找到<%s>成员", s.c_str(), memberName.c_str());
	}

	return index;
}

//int CompilerClass::GetMemberIndex(CompilerValue* value)
//{
//	uint64 index = 0;
//	for (uint64 i = 0; i < m_Data.size(); i++)
//	{
//		for (uint64 j = 0; j < m_Data[i].second.size(); j++)
//		{
//			if (m_Data[i].second[j].second.get() == value)
//			{
//				return (int)index;
//			}
//
//			index++;
//		}
//	}
//
//	return -1;
//}

//bool HazeCompilerClass::GetThisMemberName(const HazeCompilerValue* value, HString& outName, bool getOffset, V_Array<uint64>* offsets)
//{
//	uint32 count = 0;
//	for (size_t i = 0; i < m_ThisClassValue->m_Data.size(); i++)
//	{
//		for (size_t j = 0; j < m_ThisClassValue->m_Data[i].second.size(); j++)
//		{
//			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, m_ThisClassValue->m_Data[i].second[j] },
//				value, outName, getOffset, offsets))
//			{
//				if (getOffset)
//				{
//					offsets->push_back(m_Offsets[count]);
//				}
//				return true;
//			}
//
//			count++;
//		}
//	}
//
//	return false;
//}

//bool CompilerClass::GetMemberName(CompilerClassValue* classValue, const CompilerValue* value, HString& outName, bool getOffset, 
//	V_Array<Pair<uint64, CompilerValue*>>* offsets)
//{
//	uint32 count = 0;
//	for (size_t i = 0; i < classValue->m_Data.size(); i++)
//	{
//		for (size_t j = 0; j < classValue->m_Data[i].second.size(); j++)
//		{
//			if (TrtGetVariableName(nullptr, { m_Data[i].second[j].first, classValue->m_Data[i].second[j] }, value, outName, getOffset, offsets))
//			{
//				if (getOffset)
//				{
//					offsets->push_back({ m_Offsets[count], classValue->m_Data[i].second[j].get() });
//				}
//				return true;
//			}
//
//			count++;
//		}
//	}
//
//	return false;
//}

//const CompilerValue* CompilerClass::GetMemberValue(uint64 index)
//{
//	uint64 counter = 0;
//	for (uint64 i = 0; i < m_Data.size(); i++)
//	{
//		for (uint64 j = 0; j < m_Data[i].second.size(); j++)
//		{
//			if (index == counter)
//			{
//				return m_Data[i].second[j].second.get();
//			}
//
//			counter++;
//		}
//	}
//
//	return nullptr;
//}

void CompilerClass::ParseIntermediateClass(HAZE_IFSTREAM& stream, CompilerModule* m, V_Array<CompilerClass*>& parents)
{
	for (int i = 0; i < parents.size(); i++)
	{
		auto& parentClass = parents[i];
		ParseIntermediateClass(stream, m, parentClass->m_ParentClass);

		HString str;
		HazeVariableType valueType;
		x_uint32 ui32;
		x_uint64 ui64;
		for (x_uint64 j = 0; j < parentClass->m_Data.size(); ++j)
		{
			stream >> str;
			HazeVariableType::StringStreamFrom<Compiler>(stream, m->GetCompiler(), &Compiler::GetSymbolTableNameAddress);
			stream >> ui32 >> ui64;
		}
	}
}

void CompilerClass::GenClassData_I_Code(HAZE_STRING_STREAM& hss)
{
	hss << GetClassLabelHeader() << " " << m_Name << " " << m_DataSize << HAZE_ENDL;

	hss << m_ParentClass.size() << HAZE_ENDL;
	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		hss << m_ParentClass[i]->GetName() << HAZE_ENDL;
	}
	
	hss << m_MemberCount << HAZE_ENDL;

	x_uint32 offset = 0;
	GenClassData_I_CodeToHss(hss, offset);
}


void CompilerClass::GenClassData_I_CodeToHss(HAZE_STRING_STREAM& hss, x_uint32& offset)
{
	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		m_ParentClass[i]->GenClassData_I_CodeToHss(hss, offset);
	}

	//uint32 index = 0;

	x_uint32 startOffset = offset;
	for (x_uint64 i = 0; i < m_Data.size(); ++i)
	{
		hss << m_Data[i].first << " " << CAST_DESC(m_Data[i].second->GetVariableDesc()) << " ";
		m_Data[i].second->GetVariableType().StringStreamTo(hss);

		if (m_Data[i].second->IsArray())
		{

		}
		else if (m_Data[i].second->IsFunction())
		{

		}

		hss << " " << m_Offsets[i] + startOffset << " " << m_Data[i].second->GetSize() << HAZE_ENDL;
		offset += m_Data[i].second->GetSize();
		//index++;
	}
}

void CompilerClass::GenClassFunction_I_Code(HAZE_STRING_STREAM& hss)
{
	for (auto& iter : m_Functions)
	{
		iter->GenI_Code(hss);
	}
}

x_uint32 CompilerClass::GetAlignSize()
{
	x_uint32 maxMemberSize = 0;

	for (size_t i = 0; i < m_ParentClass.size(); i++)
	{
		x_uint32 parentAlignSize = m_ParentClass[i]->GetAlignSize();
		if (parentAlignSize > maxMemberSize)
		{
			maxMemberSize = parentAlignSize;
		}
	}

	for (auto& iter : m_Data)
	{
		if (iter.second->IsClass())
		{
			x_uint32 classAlignSize = DynamicCast<CompilerClassValue>(iter.second)->GetOwnerClass()->GetAlignSize();
			if (classAlignSize > maxMemberSize)
			{
				maxMemberSize = classAlignSize;
			}
		}
		else
		{
			if (iter.second->GetSize() > maxMemberSize)
			{
				maxMemberSize = iter.second->GetSize();
			}
		}
	}

	return maxMemberSize;
}

x_uint32 CompilerClass::GetOffset(x_uint32 index, Share<CompilerValue> member)
{
	return index * member->GetSize();
}

V_Array<Share<CompilerValue>> CompilerClass::CreateVariableCopyClassMember(CompilerModule* compilerModule, HazeVariableScope scope)
{
	V_Array<Share<CompilerValue>> members;
	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		auto m = m_ParentClass[i]->CreateVariableCopyClassMember(compilerModule, scope);
		members.insert(members.end(), m.begin(), m.end());
	}

	members.reserve(members.size() + m_Data.size());
	for (auto& it : m_Data)
	{
		members.push_back(CreateVariableCopyVar(compilerModule, scope, it.second));
	}

	return members;
}

bool CompilerClass::IsParentClass(CompilerClass* c) const
{
	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		if (m_ParentClass[i]->IsParentClass(c))
		{
			return true;
		}
	}

	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		if (m_ParentClass[i] == c)
		{
			return true;
		}
	}

	return false;
}

bool CompilerClass::IsParentClass(const HazeVariableType type) const
{
	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		if (m_ParentClass[i]->IsParentClass(type))
		{
			return true;
		}
	}

	for (x_uint64 i = 0; i < m_ParentClass.size(); i++)
	{
		if (m_ParentClass[i]->GetTypeId() == type.TypeId)
		{
			return true;
		}
	}

	return false;
}

bool CompilerClass::HasCommomInheritClass(CompilerClass* c1, CompilerClass* c2)
{
	if (c1 == c2 || c1->IsParentClass(c2) || c2->IsParentClass(c1))
	{
		return true;
	}
	/*else
	{
		for (uint64 i = 0; i < c1->m_ParentClass.size(); i++)
		{
			if (HasCommomInheritClass(c1->m_ParentClass[i], c2))
			{
				return true;
			}
		}
	}*/

	return false;
}

//int CompilerClass::GetClassInheritLevel() const
//{
//	uint32 maxLevel = 0;
//	uint32 inheritLevel = 0;
//	for (auto& parentClass : m_ParentClass)
//	{
//		inheritLevel = parentClass->GetClassInheritLevel() + 1;
//		if (inheritLevel > maxLevel)
//		{
//			maxLevel = inheritLevel;
//		}
//	}
//
//	return maxLevel;
//}

void CompilerClass::MemoryAlign(x_uint32 memberNum)
{
	m_Offsets.resize(memberNum);
	x_uint32 align = GetAlignSize();
	x_uint32 index = 0;

	x_uint32 size = 0;
	/*for (size_t i = 0; i < m_ParentClass.size(); i++)
	{
		size += m_ParentClass[i]->GetDataSize();
	}*/

	for (auto& iter : m_Data)
	{
		x_uint32 modSize = size % align;

		if (modSize + iter.second->GetSize() <= align || iter.second->IsClass())
		{
			x_uint32 modValue = modSize % iter.second->GetSize();
			if (modValue == 0)
			{
				m_Offsets[index] = size;
				size += iter.second->GetSize();
			}
			else
			{
				m_Offsets[index] = size + modValue;
				size += (modValue + iter.second->GetSize());
			}
		}
		else
		{
			m_Offsets[index] = size + (align - modSize);
			size += ((align - modSize) + iter.second->GetSize());

		}
		index++;
	}
}