#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTBase.h"

namespace AST
{
	class ASTFunction
	{
	public:
		ASTFunction(std::string& Name, std::vector<std::unique_ptr<ASTBase>>& Return, std::vector<std::unique_ptr<ASTBase>>& Param,
			std::vector<std::unique_ptr<ASTBase>>& LocalVariable);

		~ASTFunction();

	private:
		std::string FunctionName;
		std::vector<std::unique_ptr<ASTBase>> FunctionReturn;
		std::vector<std::unique_ptr<ASTBase>> FunctionParam; //´Ó×óµ½ÓÒ
		std::vector<std::unique_ptr<ASTBase>> FunctionLocalVariable;
	};
}