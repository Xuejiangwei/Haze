#pragma once

#include <memory>
#include <vector>
#include <string>

namespace AST
{
	class ASTBase;
	class ASTFunction;

	class ASTClass
	{
	public:
		ASTClass(std::string& Name, std::vector<std::shared_ptr<ASTBase>>& Data, std::vector<std::shared_ptr<ASTFunction>> Function);
		~ASTClass();

	private:
		std::string ClassName;
		std::vector<std::shared_ptr<ASTBase>> ClassData;
		std::vector<std::shared_ptr<ASTFunction>> ClassFunction;
	};
}