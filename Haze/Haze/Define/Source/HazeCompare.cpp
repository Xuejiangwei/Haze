#include "HazePch.h"
#include "HazeCompare.h"
#include <unordered_map>

HazeCmpType GetHazeCmpTypeByToken(HazeToken token)
{
	static HashMap<HazeToken, HazeCmpType> s_HashMap =
	{
		{HazeToken::Equal, HazeCmpType::Equal},
		{HazeToken::NotEqual, HazeCmpType::NotEqual},
		{HazeToken::Greater, HazeCmpType::Greater},
		{HazeToken::GreaterEqual, HazeCmpType::GreaterEqual},
		{HazeToken::Less, HazeCmpType::Less},
		{HazeToken::LessEqual, HazeCmpType::LessEqual},

		{HazeToken::And, HazeCmpType::And},
		{HazeToken::Or, HazeCmpType::Or},
	};

	auto iter = s_HashMap.find(token);
	if (iter != s_HashMap.end())
	{
		return iter->second;
	}

	return HazeCmpType::None;
}

InstructionOpCode GetInstructionOpCodeByCmpType(HazeCmpType type)
{
	static HashMap<HazeCmpType, InstructionOpCode> s_HashMap =
	{
		{HazeCmpType::Equal, InstructionOpCode::JE},
		{HazeCmpType::NotEqual, InstructionOpCode::JNE},
		{HazeCmpType::Greater, InstructionOpCode::JG},
		{HazeCmpType::GreaterEqual, InstructionOpCode::JNL},
		{HazeCmpType::Less, InstructionOpCode::JL},
		{HazeCmpType::LessEqual, InstructionOpCode::JNG},

		{HazeCmpType::And, InstructionOpCode::JE},
		{HazeCmpType::Or, InstructionOpCode::JE},
	};

	auto iter = s_HashMap.find(type);
	if (iter != s_HashMap.end())
	{
		return iter->second;
	}

	return InstructionOpCode::NONE;
}

HazeOperatorAssign GetHazeOperatorAssignTypeByToken(HazeToken token)
{
	static HashMap<HazeToken, HazeOperatorAssign> s_HashMap =
	{
		{HazeToken::AddAssign, HazeOperatorAssign::AddAssign},
		{HazeToken::SubAssign, HazeOperatorAssign::SubAssign},
		{HazeToken::MulAssign, HazeOperatorAssign::MulAssign},
		{HazeToken::DivAssign, HazeOperatorAssign::DivAssign},
		{HazeToken::ModAssign, HazeOperatorAssign::ModAssign},
		{HazeToken::BitAndAssign, HazeOperatorAssign::BitAndAssign},
		{HazeToken::BitOrAssign, HazeOperatorAssign::BitOrAssign},
		{HazeToken::BitXorAssign, HazeOperatorAssign::BitXorAssign},
		{HazeToken::ShlAssign, HazeOperatorAssign::ShlAssign},
		{HazeToken::ShrAssign, HazeOperatorAssign::ShrAssign},
	};

	auto iter = s_HashMap.find(token);
	if (iter != s_HashMap.end())
	{
		return iter->second;
	}

	return HazeOperatorAssign::None;
}