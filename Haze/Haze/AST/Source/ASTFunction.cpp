#include "ASTFunction.h"

#include "HazeCompiler.h"
#include "HazeBaseBlock.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerModule.h"

ASTFunction::ASTFunction(HazeCompiler* compiler, const SourceLocation& startLocation, const SourceLocation& endLocation,
	HazeSectionSignal section, HAZE_STRING& name, HazeDefineType& type, std::vector<std::unique_ptr<ASTBase>>& params,
	std::unique_ptr<ASTBase>& body) 
	: m_Compiler(compiler), m_StartLocation(startLocation), m_EndLocation(endLocation), m_Section(section), m_FunctionName(std::move(name)),m_FunctionType(std::move(type)),
	m_FunctionParams(std::move(params)), m_Body(std::move(body))
{
}

ASTFunction::~ASTFunction()
{
}

HazeValue* ASTFunction::CodeGen()
{
	auto& currModule = m_Compiler->GetCurrModule();

	std::shared_ptr<HazeCompilerFunction> compilerFunction = nullptr;
	std::shared_ptr<HazeCompilerClass> currClass = nullptr;

	std::vector<HazeDefineVariable> paramDefines(m_FunctionParams.size());
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
		currClass = currModule->GetClass(m_FunctionParams[0]->GetDefine().m_Type.CustomName);
		compilerFunction = currModule->CreateFunction(currClass, m_FunctionName, m_FunctionType, paramDefines);
	}
	compilerFunction->SetStartEndLine(m_StartLocation.Line, m_EndLocation.Line);
	m_Compiler->SetInsertBlock(compilerFunction->GetEntryBlock());

	for (int i = (int)m_FunctionParams.size() - 1; i >= 0; i--)
	{
		m_FunctionParams[i]->CodeGen();
	}

	if (m_Body)
	{
		m_Body->CodeGen();
	}

	if (compilerFunction)
	{
		compilerFunction->FunctionFinish();
	}

	return nullptr;
}

void ASTFunction::RegisterFunction()
{
	auto& currModule = m_Compiler->GetCurrModule();

	std::shared_ptr<HazeCompilerClass> currClass = nullptr;

	std::vector<HazeDefineVariable> paramDefines(m_FunctionParams.size());
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
		currClass = currModule->GetClass(m_FunctionParams[0]->GetDefine().m_Type.CustomName);
		currModule->CreateFunction(currClass, m_FunctionName, m_FunctionType, paramDefines);
	}
}

ASTFunctionSection::ASTFunctionSection(HazeCompiler* compiler,/* const SourceLocation& Location,*/ std::vector<std::unique_ptr<ASTFunction>>& functions)
	: m_Compiler(compiler), m_Functions(std::move(functions))
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

ASTFunctionDefine::ASTFunctionDefine(HazeCompiler* compiler, /*const SourceLocation& Location,*/ const HAZE_STRING& name, 
	HazeDefineType& type, std::vector<std::unique_ptr<ASTBase>>& params)
	: m_Compiler(compiler), m_FunctionName(name), m_FunctionType(type), m_FunctionParams(std::move(params))
{
}

ASTFunctionDefine::~ASTFunctionDefine()
{
}

void ASTFunctionDefine::CodeGen()
{
	auto& currModule = m_Compiler->GetCurrModule();

	std::vector<HazeDefineVariable> paramDefines(m_FunctionParams.size());
	for (size_t i = 0; i < m_FunctionParams.size(); i++)
	{
		paramDefines[i] = m_FunctionParams[i]->GetDefine();
	}

	std::shared_ptr<HazeCompilerFunction> CompilerFunction = currModule->CreateFunction(m_FunctionName, m_FunctionType, paramDefines);

	m_Compiler->SetInsertBlock(CompilerFunction->GetEntryBlock());

	for (auto& iter : m_FunctionParams)
	{
		iter->CodeGen();
	}
}

ASTClassFunctionSection::ASTClassFunctionSection(HazeCompiler* compiler, /*const SourceLocation& Location,*/ 
	std::vector<std::pair<HazeDataDesc, std::vector<std::unique_ptr<ASTFunction>>>>& functions)
	: m_Compiler(compiler), m_Functions(std::move(functions))
{
}

ASTClassFunctionSection::~ASTClassFunctionSection()
{
}

void ASTClassFunctionSection::CodeGen()
{
	for (auto& iter : m_Functions)
	{
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