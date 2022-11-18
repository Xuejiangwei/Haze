#include <unordered_map>
#include <functional>

#include "OpCodeProcessor.h"
#include "HazeVM.h"

static void Instruction_Mov(OpCodeProcessor* Processor)
{
	enum MovOpCodeType
	{
		Register,		//寄存器
		Immediate,		//立即数
	};
	static std::unordered_map<int64_t, MovOpCodeType> MapMovOpCode =
	{
		{0, MovOpCodeType::Register},
		{1, MovOpCodeType::Immediate},
	};

	//Mov (寄存器 操作数类型 操作数)
	//局部变量

	//寄存器
	uint64_t Code = Processor->GetNextBinary();
	VirtualRegister* R = Processor->GetVM()->GetVirtualRegister(Code);

	//操作数类型
	Code = Processor->GetNextBinary();

	auto It = MapMovOpCode.find(Code);
	if (It->second == MovOpCodeType::Register)
	{
		VirtualRegister* R1 = Processor->GetVM()->GetVirtualRegister(Processor->GetNextBinary());
		R->Data = R1->Data;
	}
	else if (It->second == MovOpCodeType::Immediate)
	{
		R->Data = Processor->GetNextBinary();
	}
}

static void Instruction_Add(OpCodeProcessor* Processor)
{
}

static void Instruction_Sub(OpCodeProcessor* Processor)
{
}

static void Instruction_Mul(OpCodeProcessor* Processor)
{
}

static void Instruction_Div(OpCodeProcessor* Processor)
{
}

static void Instruction_Mod(OpCodeProcessor* Processor)
{
}

static void Instruction_Exp(OpCodeProcessor* Processor)
{
}

static void Instruction_Neg(OpCodeProcessor* Processor)
{
}

static void Instruction_Inc(OpCodeProcessor* Processor)
{
}

static void Instruction_Dec(OpCodeProcessor* Processor)
{
}

static void Instruction_And(OpCodeProcessor* Processor)
{
}

static void Instruction_Or(OpCodeProcessor* Processor)
{
}

static void Instruction_Not(OpCodeProcessor* Processor)
{
}

static void Instruction_XOR(OpCodeProcessor* Processor)
{
}

static void Instruction_SHL(OpCodeProcessor* Processor)
{
}

static void Instruction_SHR(OpCodeProcessor* Processor)
{
}

static void Instruction_Push(OpCodeProcessor* Processor)
{
}

static void Instruction_Pop(OpCodeProcessor* Processor)
{
}

static void Instruction_Call(OpCodeProcessor* Processor)
{
}

static std::unordered_map <InstructCode, std::function<void(OpCodeProcessor*)>> MapInstructInvoke =
{
	{InstructCode::MOV, Instruction_Mov},
	{InstructCode::ADD, Instruction_Add},
	{InstructCode::SUB, Instruction_Sub},
	{InstructCode::MUL, Instruction_Mul},
	{InstructCode::DIV, Instruction_Div},
	{InstructCode::MOD, Instruction_Mod},
	{InstructCode::EXP, Instruction_Exp},
	{InstructCode::NEG, Instruction_Neg},
	{InstructCode::INC, Instruction_Inc},
	{InstructCode::DEC, Instruction_Dec},
	{InstructCode::AND, Instruction_And},
	{InstructCode::OR, Instruction_Or},
	{InstructCode::NOT, Instruction_Not},
	{InstructCode::XOR, Instruction_XOR},
	{InstructCode::SHL, Instruction_SHL},
	{InstructCode::SHR, Instruction_SHR},
	{InstructCode::PUSH, Instruction_Push},
	{InstructCode::POP, Instruction_Pop},
	{InstructCode::CALL, Instruction_Call},
};

OpCodeProcessor::OpCodeProcessor(HazeVM* VM) : VM(VM)
{
}

OpCodeProcessor::~OpCodeProcessor()
{
}