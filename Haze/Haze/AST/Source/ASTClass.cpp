#include "HazePch.h"
#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"

#include "HazeLog.h"

extern int g_ClassInheritLimit;
extern int g_ClassInheritLevelLimit;

ASTClass::ASTClass(HazeCompiler* compiler,/* const SourceLocation& Location,*/ HString& name, V_Array<HString>& parentClass,
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>>& data, Unique<ASTClassFunctionSection>& functionSection)
	: m_Compiler(compiler), m_ClassName(std::move(name)), m_ParentClasses(std::move(parentClass)), m_ClassDatas(std::move(data)), 
		m_ClassFunctionSection(std::move(functionSection))
{
}

ASTClass::~ASTClass()
{
}

void ASTClass::CodeGen()
{
	if (m_ParentClasses.size() > g_ClassInheritLimit)
	{
		HAZE_LOG_ERR_W("解析创建类错误,类<%s>继承类个数超过<%d>!\n", m_ClassName.c_str(), g_ClassInheritLimit);
		return;
	}

	V_Array<HazeCompilerClass*> parentClasses;
	for (size_t i = 0; i < m_ParentClasses.size(); i++)
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

	for (auto& parentClass : parentClasses)
	{
		if (parentClass->GetClassInheritLevel() > g_ClassInheritLevelLimit)
		{
			HAZE_LOG_ERR_W("解析创建类错误,类<%s>继承层级超过<%d>!\n", m_ClassName.c_str(), g_ClassInheritLevelLimit);
			return;
		}
	}

	V_Array<Pair<HazeDataDesc, V_Array<Pair<HString, Share<HazeCompilerValue>>>>> datas;

	for (size_t i = 0; i < m_ClassDatas.size(); i++)
	{
		datas.push_back({ m_ClassDatas[i].first, {} });
		for (size_t j = 0; j < m_ClassDatas[i].second.size(); j++)
		{
			for (auto& Class : parentClasses)
			{
				if (Class->GetMemberIndex(m_ClassDatas[i].second[j]->GetName()) >= 0)
				{
					HAZE_LOG_ERR_W("解析创建类错误,类<%s>中存在与父类<%s>相同命名的成员变量<%s>!\n", m_ClassName.c_str(), Class->GetName().c_str(),
						m_ClassDatas[i].second[j]->GetName());
					return;
				}
			}

			datas.back().second.push_back({ m_ClassDatas[i].second[j]->GetName(), m_ClassDatas[i].second[j]->CodeGen() });
		}
	}

	m_Compiler->GetCurrModule()->CreateClass(m_ClassName, parentClasses, datas);

	if (m_ClassFunctionSection)
	{
		m_ClassFunctionSection->CodeGen();
	}

	m_Compiler->GetCurrModule()->FinishCreateClass();
}

ASTClassDefine::ASTClassDefine(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HString& name,
	V_Array<V_Array<Unique<ASTBase>>>& datas, V_Array<Unique<ASTFunctionDefine>>& functions)
	: m_Compiler(compiler), m_ClassName(std::move(name)), m_ClassDatas(std::move(datas)), m_ClassFunctions(std::move(functions))
{
}

ASTClassDefine::~ASTClassDefine()
{
}

void ASTClassDefine::CodeGen()
{
}