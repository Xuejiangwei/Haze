#include "HazePch.h"
#include "ASTFunction.h"

#include "HazeLogDefine.h"
#include "Compiler.h"
#include "CompilerBlock.h"
#include "HazeCompilerPointerValue.h"
#include "CompilerFunction.h"
#include "CompilerClass.h"
#include "CompilerClassValue.h"
#include "CompilerModule.h"

ASTFunction::ASTFunction(Compiler* compiler, const SourceLocation& startLocation, const SourceLocation& endLocation,
	HazeSectionSignal section, HString& name, HazeDefineType& type, V_Array<Unique<ASTBase>>& params,
	Unique<ASTBase> body, bool isVirtual, bool isPureVirtual)
	: m_Compiler(compiler), m_StartLocation(startLocation), m_EndLocation(endLocation), m_Section(section), 
	m_FunctionName(Move(name)),m_FunctionType(Move(type)),m_FunctionParams(Move(params)), m_Body(Move(body)), 
	m_IsVirtual(isVirtual), m_IsPureVirtual(isPureVirtual)
{
}

ASTFunction::~ASTFunction()
{
}

HazeValue* ASTFunction::CodeGen()
{
	auto& currModule = m_Compiler->GetCurrModule();

	Share<CompilerFunction> compilerFunction = nullptr;
	Share<CompilerClass> currClass = nullptr;

	V_Array<HazeDefineVariable> paramDefines(m_FunctionParams.size());
	for (size_t i = 0; i < m_FunctionParams.size(); i++)
	{
		paramDefines[i] = m_FunctionParams[i]->GetDefine();
	}

	if (m_Section == HazeSectionSignal::Global)
	{
		compilerFunction = currModule->CreateFunction(m_FunctionName, m_FunctionType, paramDefines);
	}
	else if (m_Section == HazeSectionSignal::Class)
	{
		currClass = currModule->GetClass(*m_FunctionParams[0]->GetDefine().Type.CustomName);
		compilerFunction = currModule->CreateFunction(currClass, m_IsVirtual ? ClassCompilerFunctionType::Virtual : m_IsPureVirtual ?
			ClassCompilerFunctionType::PureVirtual : ClassCompilerFunctionType::Normal,
			m_FunctionName, m_FunctionType, paramDefines);
	}
	compilerFunction->SetStartEndLine(m_StartLocation.Line, m_EndLocation.Line);
	m_Compiler->SetInsertBlock(compilerFunction->GetEntryBlock());

	for (int i = (int)m_FunctionParams.size() - 1; i >= 0; i--)
	{
		currModule->BeginCreateFunctionParamVariable();
		m_FunctionParams[i]->CodeGen();
		currModule->EndCreateFunctionParamVariable();
	}

	if (m_Body)
	{
		m_Body->CodeGen();
	}

	if (compilerFunction == currModule->GetCurrFunction())
	{
		currModule->FinishFunction();
	}
	else
	{
		auto& m_Location = m_EndLocation;
		AST_ERR_W("���ɺ���<%s>��������, ���ǵ�ǰģ������ĺ���<%s>", m_FunctionName.c_str(), currModule->GetCurrFunction()->GetName().c_str());
	}

	return nullptr;
}

void ASTFunction::RegisterFunction()
{
	auto& currModule = m_Compiler->GetCurrModule();

	Share<CompilerClass> currClass = nullptr;

	V_Array<HazeDefineVariable> paramDefines(m_FunctionParams.size());
	for (size_t i = 0; i < m_FunctionParams.size(); i++)
	{
		paramDefines[i] = m_FunctionParams[i]->GetDefine();
	}

	if (m_Section == HazeSectionSignal::Global)
	{
		currModule->CreateFunction(m_FunctionName, m_FunctionType, paramDefines);
	}
	else if (m_Section == HazeSectionSignal::Class)
	{
		currClass = currModule->GetClass(*m_FunctionParams[0]->GetDefine().Type.CustomName);
		currModule->CreateFunction(currClass, m_IsVirtual ? ClassCompilerFunctionType::Virtual : m_IsPureVirtual ?
			ClassCompilerFunctionType::PureVirtual : ClassCompilerFunctionType::Normal, m_FunctionName, m_FunctionType, paramDefines);
	}
}

ASTFunctionSection::ASTFunctionSection(Compiler* compiler,/* const SourceLocation& Location,*/ V_Array<Unique<ASTFunction>>& functions)
	: m_Compiler(compiler), m_Functions(Move(functions))
{
}

ASTFunctionSection::~ASTFunctionSection()
{
}

void ASTFunctionSection::CodeGen()
{
	for (auto& iter : m_Functions)
	{
		iter->RegisterFunction();
	}

	for (auto& iter : m_Functions)
	{
		iter->CodeGen();
	}
}

ASTFunctionDefine::ASTFunctionDefine(Compiler* compiler, /*const SourceLocation& Location,*/ const HString& name, 
	HazeDefineType& type, V_Array<Unique<ASTBase>>& params)
	: m_Compiler(compiler), m_FunctionName(name), m_FunctionType(type), m_FunctionParams(Move(params))
{
}

ASTFunctionDefine::~ASTFunctionDefine()
{
}

void ASTFunctionDefine::CodeGen()
{
	auto& currModule = m_Compiler->GetCurrModule();

	V_Array<HazeDefineVariable> paramDefines(m_FunctionParams.size());
	for (size_t i = 0; i < m_FunctionParams.size(); i++)
	{
		paramDefines[i] = m_FunctionParams[i]->GetDefine();
	}

	Share<CompilerFunction> CompilerFunction = currModule->CreateFunction(m_FunctionName, m_FunctionType, paramDefines);

	m_Compiler->SetInsertBlock(CompilerFunction->GetEntryBlock());

	for (auto& iter : m_FunctionParams)
	{
		iter->CodeGen();
	}
}

ASTClassFunctionSection::ASTClassFunctionSection(Compiler* compiler, /*const SourceLocation& Location,*/ 
	V_Array<Pair<HazeDataDesc, V_Array<Unique<ASTFunction>>>>& functions)
	: m_Compiler(compiler), m_Functions(Move(functions))
{
}

ASTClassFunctionSection::~ASTClassFunctionSection()
{
}

void ASTClassFunctionSection::CodeGen()
{
	auto& className = m_Compiler->GetCurrModule()->GetCurrClassName();
	for (auto& iter : m_Functions)
	{
		if (iter.first == HazeDataDesc::ClassFunction_Local_Public)
		{
			for (uint64 i = 0; i < iter.second.size(); i++)
			{
				if (iter.second[i]->GetName() == className && i != 0)
				{
					COMPILER_ERR_MODULE_W("��<%s>��Ҫ��<��>��Χ�ڵ�һ�����幹�캯��", m_Compiler->GetCurrModuleName().c_str(), className.c_str());
					return;
				}
			}
		}

		for (auto& function : iter.second)
		{
			function->RegisterFunction();
		}
	}

	for (auto& iter : m_Functions)
	{
		for (auto& function : iter.second)
		{
			function->CodeGen();
		}
	}
}