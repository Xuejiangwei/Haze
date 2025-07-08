#include "HazePch.h"
#include "ASTEnum.h"
#include "ASTBase.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerValue.h"
#include "CompilerEnum.h"

ASTEnum::ASTEnum(Compiler* compiler, const SourceLocation& location, HString& name, HazeValueType baseType,
	V_Array<Pair<HString, Unique<ASTBase>>>& enums)
	: m_Compiler(compiler), m_Location(location), m_EnumName(Move(name)), m_BaseType(baseType), m_Enums(Move(enums))
{
}

ASTEnum::~ASTEnum()
{
}

void ASTEnum::CodeGen()
{
	auto enumValue = m_Compiler->GetCurrModule()->CreateEnum(m_EnumName, 0);

	for (size_t i = 0; i < m_Enums.size(); i++)
	{
		Share<CompilerValue> v = nullptr;
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
				auto preValue = m_Compiler->GetEnumVariable(m_EnumName, m_Enums[i - 1].first);
				if (preValue)
				{
					AddEnumOneValueByType(value, preValue->GetValue());
				}
				else
				{
					AST_ERR_W("枚举<%s>生成错误, 未找到枚举<%s>", m_Enums[i].first.c_str(), m_Enums[i - 1].first.c_str());
					return;
				}
			}
			else
			{
				memset(&value.Value, 0, sizeof(value.Value));
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

void ASTEnum::AddEnumOneValueByType(HazeValue& value, const HazeValue& prValue)
{
	switch (m_BaseType)
	{
	case HazeValueType::Int8:
		value.Value.Int8 = prValue.Value.Int8 + 1;
		break;
	case HazeValueType::UInt8:
		value.Value.UInt8 = prValue.Value.UInt8 + 1;
		break;
	case HazeValueType::Int16:
		value.Value.Int16 = prValue.Value.Int16 + 1;
		break;
	case HazeValueType::UInt16:
		value.Value.UInt16 = prValue.Value.UInt16 + 1;
		break;
	case HazeValueType::Int32:
		value.Value.Int32 = prValue.Value.Int32 + 1;
		break;
	case HazeValueType::UInt32:
		value.Value.UInt32 = prValue.Value.UInt32 + 1;
		break;
	case HazeValueType::Int64:
		value.Value.Int64 = prValue.Value.Int64 + 1;
		break;
	case HazeValueType::UInt64:
		value.Value.UInt64 = prValue.Value.UInt64 + 1;
		break;
	default:
		AST_ERR_W("枚举自动匹配常量值出错");
		break;
	}
}