#include "HazeCompare.h"
#include "Haze.h"
#include <unordered_map>

HazeCmpType GetHazeCmpTypeByToken(HazeToken Token)
{
	static std::unordered_map<HazeToken, HazeCmpType> HashMap = 
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

	auto Iter = HashMap.find(Token);
	if (Iter != HashMap.end())
	{
		return Iter->second;
	}
	
	return HazeCmpType::None;
}

InstructionOpCode GetInstructionOpCodeByCmpType(HazeCmpType Type)
{
	static std::unordered_map<HazeCmpType, InstructionOpCode> HashMap =
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

	auto Iter = HashMap.find(Type);
	if (Iter != HashMap.end())
	{
		return Iter->second;
	}

	return InstructionOpCode::NONE;
}

HazeOperatorAssign GetHazeOperatorAssignTypeByToken(HazeToken Token)
{
	static std::unordered_map<HazeToken, HazeOperatorAssign> HashMap =
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

	auto Iter = HashMap.find(Token);
	if (Iter != HashMap.end())
	{
		return Iter->second;
	}

	return HazeOperatorAssign::None;
}