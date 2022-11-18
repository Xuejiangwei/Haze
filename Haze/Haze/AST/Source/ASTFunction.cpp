#include "ASTFunction.h"

namespace AST
{
	ASTFunction::ASTFunction(std::string& Name, std::vector<std::unique_ptr<ASTBase>>& Return, std::vector<std::unique_ptr<ASTBase>>& Param,
		std::vector<std::unique_ptr<ASTBase>>& LocalVariable) :
		FunctionName(std::move(Name)),
		FunctionReturn(std::move(Return)),
		FunctionParam(std::move(Param)),
		FunctionLocalVariable(std::move(LocalVariable))
	{
	}

	ASTFunction::~ASTFunction()
	{
	}
}