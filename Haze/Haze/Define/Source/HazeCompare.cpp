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
	};

	auto Iter = HashMap.find(Token);
	if (Iter != HashMap.end())
	{
		return Iter->second;
	}
	
	return HazeCmpType::None;
}

const HAZE_CHAR* GetInstructionStringByCmpType(HazeCmpType Type)
{
	static std::unordered_map<HazeCmpType, InstructionOpCode> HashMap =
	{
		{HazeCmpType::Equal, InstructionOpCode::JE},
		{HazeCmpType::NotEqual, InstructionOpCode::JNE},
		{HazeCmpType::Greater, InstructionOpCode::JG},
		{HazeCmpType::GreaterEqual, InstructionOpCode::JNL},
		{HazeCmpType::Less, InstructionOpCode::JL},
		{HazeCmpType::LessEqual, InstructionOpCode::JNG},
	};

	auto Iter = HashMap.find(Type);
	if (Iter != HashMap.end())
	{
		return GetInstructionString(Iter->second);
	}

	return nullptr;
}
