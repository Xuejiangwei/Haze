#include "HazePch.h"
#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"

#include "Compiler.h"
#include "CompilerHelper.h"
#include "CompilerModule.h"
#include "CompilerClass.h"
#include "CompilerValue.h"

#include "HazeLog.h"

ASTClass::ASTClass(Compiler* compiler,/* const SourceLocation& Location,*/ HString& name, V_Array<HString>& parentClass,
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>>& data, Unique<ASTClassFunctionSection>& functionSection)
	: m_Compiler(compiler), m_ClassName(Move(name)), m_ParentClasses(Move(parentClass)),
	m_ClassDatas(Move(data)), m_ClassFunctionSection(Move(functionSection))
{
}

ASTClass::~ASTClass()
{
}

void ASTClass::CodeGen()
{
	V_Array<CompilerClass*> parentClasses;
	for (x_uint64 i = 0; i < m_ParentClasses.size(); i++)
	{
		auto parentClass = m_Compiler->GetCurrModule()->GetClass(m_ParentClasses[i]);
		if (parentClass)
		{
			parentClasses.push_back(parentClass.get());
		}
		else
		{
			HAZE_LOG_ERR_W("解析创建类错误,未能找到类<%s>的父类<%s>!\n", m_ClassName.c_str(), m_ParentClasses[i].c_str());
			return;
		}
	}

	// 判断菱形继承
	if (parentClasses.size() > 1)
	{
		for (x_uint64 i = 0; i < parentClasses.size() - 1; i++)
		{
			for (x_uint64 j = i + 1; j < parentClasses.size(); j++)
			{
				if (CompilerClass::HasCommomInheritClass(parentClasses[i], parentClasses[j]))
				{
					HAZE_LOG_ERR_W("创建类<%s>错误, 父类<%s><%s>存在共同的父类!\n", m_ClassName.c_str(), 
						m_ParentClasses[i].c_str(), m_ParentClasses[j].c_str());
					return;
				}
			}
		}
	}
	
	V_Array<Pair<HString, Share<CompilerValue>>> datas;
	for (x_uint64 i = 0; i < m_ClassDatas.size(); i++)
	{
		for (x_uint64 j = 0; j < m_ClassDatas[i].second.size(); j++)
		{
			auto v = m_ClassDatas[i].second[j]->CodeGen(nullptr);
			if (v->IsRefrence())
			{
				auto m_Location = m_ClassDatas[i].second[j]->m_Location;
				AST_ERR_W("类<%s>成员不允许是引用类型", m_ClassName.c_str());
				return;
			}
			else
			{
				v->SetDataDesc(m_ClassDatas[i].first);
				datas.push_back({ m_ClassDatas[i].second[j]->GetName(), v });
			}
		}
	}

	m_Compiler->GetCurrModule()->CreateClass(m_ClassName, parentClasses, datas);

	if (m_ClassFunctionSection)
	{
		m_ClassFunctionSection->CodeGen();
	}

	m_Compiler->GetCurrModule()->FinishCreateClass();
}

ASTClassDefine::ASTClassDefine(Compiler* compiler, /*const SourceLocation& Location,*/ HString& name,
	V_Array<V_Array<Unique<ASTBase>>>& datas, V_Array<Unique<ASTFunctionDefine>>& functions)
	: m_Compiler(compiler), m_ClassName(Move(name)), m_ClassDatas(Move(datas)), 
	m_ClassFunctions(Move(functions))
{
}

ASTClassDefine::~ASTClassDefine()
{
}

void ASTClassDefine::CodeGen()
{
}