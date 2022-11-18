#include "ASTClass.h"
#include "ASTBase.h"

namespace AST
{
	ASTClass::ASTClass(std::string& Name, std::vector<std::shared_ptr<ASTBase>>& Data, std::vector<std::shared_ptr<ASTFunction>> Function) :
		ClassName(std::move(Name)), ClassData(std::move(Data)), ClassFunction(std::move(Function))
	{
	}

	ASTClass::~ASTClass()
	{
	}
}