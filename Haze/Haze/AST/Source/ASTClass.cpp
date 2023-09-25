#include "ASTBase.h"
#include "ASTFunction.h"
#include "ASTClass.h"

#include "HazeCompiler.h"
#include "HazeCompilerHelper.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"

#include "HazeLog.h"

ASTClass::ASTClass(HazeCompiler* m_Compiler,/* const SourceLocation& Location,*/ HAZE_STRING& m_Name, std::vector<HAZE_STRING>& ParentClass,
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTBase>>>>& Data, std::unique_ptr<ASTClassFunctionSection>& FunctionSection)
	: m_Compiler(m_Compiler), ClassName(std::move(m_Name)), ParentClass(std::move(ParentClass)), Vector_ClassData(std::move(Data)), ClassFunctionSection(std::move(FunctionSection))
{
}

ASTClass::~ASTClass()
{
}

void ASTClass::CodeGen()
{
	std::vector<HazeCompilerClass*> Vector_ParentClass;
	for (size_t i = 0; i < ParentClass.size(); i++)
	{
		auto Class = m_Compiler->GetCurrModule()->GetClass(ParentClass[i]);
		if (Class)
		{
			Vector_ParentClass.push_back(Class.get());
		}
		else
		{
			HAZE_LOG_ERR_W("解析创建类错误,未能找到类<%s>的父类<%s>!\n", ClassName.c_str(), ParentClass[i].c_str());
			return;
		}
	}

	std::vector<std::pair<HazeDataDesc, std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>>>> Data;

	for (size_t i = 0; i < Vector_ClassData.size(); i++)
	{
		Data.push_back({ Vector_ClassData[i].first, {} });
		for (size_t j = 0; j < Vector_ClassData[i].second.size(); j++)
		{
			for (auto& Class : Vector_ParentClass)
			{
				if (Class->GetMemberIndex(Vector_ClassData[i].second[j]->GetName()) >= 0)
				{
					HAZE_LOG_ERR_W("解析创建类错误,类<%s>中存在与父类<%s>相同命名的成员变量<%s>!\n", ClassName.c_str(), Class->GetName().c_str(),
						Vector_ClassData[i].second[j]->GetName());
					return;
				}
			}

			Data.back().second.push_back({ Vector_ClassData[i].second[j]->GetName(), Vector_ClassData[i].second[j]->CodeGen() });
		}
	}

	m_Compiler->GetCurrModule()->CreateClass(ClassName, Vector_ParentClass, Data);

	if (ClassFunctionSection)
	{
		ClassFunctionSection->CodeGen();
	}

	m_Compiler->GetCurrModule()->FinishCreateClass();
}

ASTClassDefine::ASTClassDefine(HazeCompiler* m_Compiler, /*const SourceLocation& Location,*/ HAZE_STRING& m_Name, std::vector<std::vector<std::unique_ptr<ASTBase>>>& Data, std::vector<std::unique_ptr<ASTFunctionDefine>>& Function)
	: m_Compiler(m_Compiler), ClassName(std::move(m_Name)), ClassData(std::move(Data)), ClassFunction(std::move(Function))
{
}

ASTClassDefine::~ASTClassDefine()
{
}

void ASTClassDefine::CodeGen()
{
}