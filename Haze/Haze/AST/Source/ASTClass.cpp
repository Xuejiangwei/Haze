#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"

#include "HazeLog.h"

ASTClass::ASTClass(HazeCompiler* compiler,/* const SourceLocation& Location,*/ HAZE_STRING& name, std::vector<HAZE_STRING>& parentClass,
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>>& data, std::unique_ptr<ASTClassFunctionSection>& functionSection)
	: m_Compiler(compiler), m_ClassName(std::move(name)), m_ParentClasses(std::move(parentClass)), m_ClassDatas(std::move(data)), 
		m_ClassFunctionSection(std::move(functionSection))
{
}

ASTClass::~ASTClass()
{
}

void ASTClass::CodeGen()
{
	std::vector<HazeCompilerClass*> parentClasses;
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

	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>> datas;

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

ASTClassDefine::ASTClassDefine(HazeCompiler* compiler, /*const SourceLocation& Location,*/ HAZE_STRING& name,
	std::vector<std::vector<std::unique_ptr<ASTBase>>>& datas, std::vector<std::unique_ptr<ASTFunctionDefine>>& functions)
	: m_Compiler(compiler), m_ClassName(std::move(name)), m_ClassDatas(std::move(datas)), m_ClassFunctions(std::move(functions))
{
}

ASTClassDefine::~ASTClassDefine()
{
}

void ASTClassDefine::CodeGen()
{
}