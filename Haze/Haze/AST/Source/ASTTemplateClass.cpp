#include "HazePch.h"
#include "ASTTemplateClass.h"
#include "ASTFunction.h"
#include "HazeLog.h"

#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerClass.h"
#include "HazeCompilerTemplateClass.h"

extern int g_ClassInheritLimit;
extern int g_ClassInheritLevelLimit;

ASTTemplateClass::ASTTemplateClass(Compiler* compiler, HString& name, V_Array<HString>& parentClass,
	V_Array<HString>& templateTypes,
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTBase>>>>& data,
	Unique<ASTClassFunctionSection>& functionSection)
	: ASTTemplateBase(templateTypes),  m_Compiler(compiler), m_ClassName(Move(name)),
	m_ParentClasses(Move(parentClass)), m_ClassDatas(Move(data)),
	m_ClassFunctionSection(Move(functionSection))
{
}

ASTTemplateClass::~ASTTemplateClass()
{
}

void ASTTemplateClass::CodeGen()
{
	/*if (m_ParentClasses.size() > g_ClassInheritLimit)
	{
		HAZE_LOG_ERR_W("��������ģ�������,��<%s>�̳����������<%d>!\n", m_ClassName.c_str(), g_ClassInheritLimit);
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
			HAZE_LOG_ERR_W("�������������,δ���ҵ���<%s>�ĸ���<%s>!\n", m_ClassName.c_str(), m_ParentClasses[i].c_str());
			return;
		}
	}

	for (auto& parentClass : parentClasses)
	{
		if (parentClass->GetClassInheritLevel() > g_ClassInheritLevelLimit)
		{
			HAZE_LOG_ERR_W("�������������,��<%s>�̳в㼶����<%d>!\n", m_ClassName.c_str(), g_ClassInheritLevelLimit);
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
					HAZE_LOG_ERR_W("�������������,��<%s>�д����븸��<%s>��ͬ�����ĳ�Ա����<%s>!\n", m_ClassName.c_str(), Class->GetName().c_str(),
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

	m_Compiler->GetCurrModule()->FinishCreateClass();*/
}
