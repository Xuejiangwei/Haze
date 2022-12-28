#include <unordered_map>
#include <functional>

#include "Haze.h"
#include "OpCodeProcessor.h"
#include "HazeVM.h"

enum OpCodeType
{
	Register,		//寄存器
	Immediate,		//立即数
};

static std::unordered_map<int64_t, OpCodeType> MapOpCode =
{
	{0, OpCodeType::Register},
	{1, OpCodeType::Immediate},
};

static void Instruction_Mov(OpCodeProcessor* Processor)
{
	//Mov (寄存器 操作数类型 操作数 操作数类型 操作数)
	//局部变量

	//寄存器
	uint64_t Code = Processor->GetNextBinary();
	HazeValue* R = Processor->GetVM()->GetVirtualRegister(Code);

	//操作数类型
	Code = Processor->GetNextBinary();

	auto It = MapOpCode.find(Code);
	if (It->second == OpCodeType::Register)
	{
		Code = Processor->GetNextBinary();
		HazeValue* R1 = Processor->GetVM()->GetVirtualRegister(Code);
		*R = *R1;
	}
	else if (It->second == OpCodeType::Immediate)
	{
		*R = Processor->GetNextBinary();
	}
}

static void Instruction_Add(OpCodeProcessor* Processor)
{
	//Add (寄存器 操作数类型 操作数)
	//局部变量

	//寄存器
	uint64_t Code = Processor->GetNextBinary();
	HazeValue* R = Processor->GetVM()->GetVirtualRegister(Code);

	//操作数类型
	Code = Processor->GetNextBinary();

	auto It = MapOpCode.find(Code);
	if (It->second == OpCodeType::Register)
	{
		Code = Processor->GetNextBinary();
		HazeValue* R1 = Processor->GetVM()->GetVirtualRegister(Code);
		*R += *R1;
	}
	else if (It->second == OpCodeType::Immediate)
	{
		//*R = Processor->GetNextBinary();
	}
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

static std::unordered_map <InstructionOpCode, std::function<void(OpCodeProcessor*)>> MapInstructInvoke =
{
	{InstructionOpCode::MOV, Instruction_Mov},
	{InstructionOpCode::ADD, Instruction_Add},
	{InstructionOpCode::SUB, Instruction_Sub},
	{InstructionOpCode::MUL, Instruction_Mul},
	{InstructionOpCode::DIV, Instruction_Div},
	{InstructionOpCode::MOD, Instruction_Mod},
	{InstructionOpCode::EXP, Instruction_Exp},
	{InstructionOpCode::NEG, Instruction_Neg},
	{InstructionOpCode::INC, Instruction_Inc},
	{InstructionOpCode::DEC, Instruction_Dec},
	{InstructionOpCode::AND, Instruction_And},
	{InstructionOpCode::OR, Instruction_Or},
	{InstructionOpCode::NOT, Instruction_Not},
	{InstructionOpCode::XOR, Instruction_XOR},
	{InstructionOpCode::SHL, Instruction_SHL},
	{InstructionOpCode::SHR, Instruction_SHR},
	{InstructionOpCode::PUSH, Instruction_Push},
	{InstructionOpCode::POP, Instruction_Pop},
	{InstructionOpCode::CALL, Instruction_Call},
};

OpCodeProcessor::OpCodeProcessor(HazeVM* VM) : VM(VM)
{
}

OpCodeProcessor::~OpCodeProcessor()
{
}