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
	case HazeValueType::Byte:
		value.Value.Byte = prValue.Value.Byte + 1;
		break;
	case HazeValueType::UnsignedByte:
		value.Value.UnsignedByte = prValue.Value.UnsignedByte + 1;
		break;
	case HazeValueType::Short:
		value.Value.Short = prValue.Value.Short + 1;
		break;
	case HazeValueType::UnsignedShort:
		value.Value.UnsignedShort = prValue.Value.UnsignedShort + 1;
		break;
	case HazeValueType::Int:
		value.Value.Int = prValue.Value.Int + 1;
		break;
	case HazeValueType::Long:
		value.Value.Long = prValue.Value.Long + 1;
		break;
	case HazeValueType::UnsignedInt:
		value.Value.UnsignedInt = prValue.Value.UnsignedInt + 1;
		break;
	case HazeValueType::UnsignedLong:
		value.Value.UnsignedLong = prValue.Value.UnsignedLong + 1;
		break;
	default:
		AST_ERR_W("枚举自动匹配常量值出错");
		break;
	}
}