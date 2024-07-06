#include "HazePch.h"
#include "ASTEnum.h"
#include "ASTBase.h"
#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerEnum.h"

ASTEnum::ASTEnum(HazeCompiler* compiler, const SourceLocation& location, HString& name, HazeValueType baseType,
	V_Array<Pair<HString, Unique<ASTBase>>>& enums)
	: m_Compiler(compiler), m_Location(location), m_EnumName(Move(name)), m_BaseType(baseType), m_Enums(Move(enums))
{
}

ASTEnum::~ASTEnum()
{
}

void ASTEnum::CodeGen()
{
	auto enumValue = m_Compiler->GetCurrModule()->CreateEnum(m_EnumName, m_BaseType);

	for (size_t i = 0; i < m_Enums.size(); i++)
	{
		Share<HazeCompilerValue> v = nullptr;
		if (m_Enums[i].second)
		{
			//这里需要计算出常量
			v = m_Enums[i].second->CodeGen();
			if (!v)
			{
				AST_ERR_W("枚举<%s>生成错误", m_Enums[i].first.c_str());
				return;
			}
		}
		else
		{
			HazeValue value;
			if (i > 0)
			{
				auto preValue = m_Compiler->GetEnumVariable(m_Enums[i - 1].first);
				if (preValue)
				{
					value.Value.UnsignedLong = preValue->GetValue().Value.UnsignedLong + 1;
				}
				else
				{
					AST_ERR_W("枚举<%s>生成错误, 未找到枚举<%s>", m_Enums[i].first.c_str(), m_Enums[i - 1].first.c_str());
					return;
				}
			}
			else
			{
				value.Value.UnsignedLong = 0;
			}

			v = m_Compiler->GenConstantValue(m_BaseType, value);
		}

		if (!v->IsConstant())
		{
			AST_ERR_W("枚举<%s>生成错误, 不是常量", m_Enums[i].first.c_str());
			return;
		}

		enumValue->AddEnumValue(m_Enums[i].first, v);
	}

	m_Compiler->GetCurrModule()->FinishCreateEnum();
}