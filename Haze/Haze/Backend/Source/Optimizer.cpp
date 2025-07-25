#include "HazePch.h"
#include "Optimizer.h"
#include "BackendParse.h"
#include "HazeInstruction.h"
#include "HazeLog.h"
#include <algorithm>
#include <queue>
#include <stack>

Optimizer::Optimizer(BackendParse* backend_parse) 
    : m_backend_parse(backend_parse)
{
    m_stats = OptimizationStats();
}

Optimizer::~Optimizer()
{
}

void Optimizer::optimize(ModuleUnit& module)
{
    HAZE_LOG_INFO_W("开始优化模块: %s\n", module.m_Name.c_str());
    
    // 重置统计信息
    m_stats = OptimizationStats();
    
    // 按优化级别执行优化
    if (m_config.optimizationLevel >= BackendOptimizationLevel::Base)
    {
        // 基础优化
        if (m_config.ConstantFold)
        {
            ConstantFolding(module);
        }
        
        if (m_config.PeeholeOptimization)
        {
            PeepholeOptimization(module);
        }
        
        if (m_config.DeadCodeElimination)
        {
           //DeadCodeElimination(module);
        }
    }
    
    if (m_config.optimizationLevel >= BackendOptimizationLevel::Radical)
    {
        // 激进优化
        if (m_config.FunctionInline)
        {
            functionInlining(module);
        }
        
        if (m_config.LoopOptimization)
        {
            loopOptimization(module);
        }
    }
    
    if (m_config.RegisterAllocation)
    {
        registerAllocation(module);
    }
    
    HAZE_LOG_INFO_W("优化完成 - 消除指令: %d, 常量折叠: %d, 函数内联: %d\n", 
        m_stats.instructions_eliminated, m_stats.constants_folded, m_stats.functions_inlined);
}

void Optimizer::ConstantFolding(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        for (size_t i = 0; i < function.Instructions.size(); ++i)
        {
            auto& inst = function.Instructions[i];

            // 检查是否为算术运算且操作数都是常量
            if (IsArithmeticOpCode(inst.InsCode) && inst.Operator.size() > 2)
            {
                bool allConstants = true;
                for (size_t i = 1; i < inst.Operator.size(); i++)
                {
                    if (!IsConstant(inst.Operator[i].Desc))
                    {
                        allConstants = false;
                        break;
                    }
                }

                if (allConstants)
                {
                    HazeValue result;
                    if (EvaluateConstantExpression(inst, result))
                    {
                        // 创建常量赋值指令
                        ModuleUnit::FunctionInstruction const_inst;
                        const_inst.InsCode = InstructionOpCode::MOV;
                        const_inst.Operator.resize(2);

                        // 目标操作数
                        const_inst.Operator[0] = inst.Operator[0];
                        const_inst.Operator[1] = inst.Operator[1];

                        const_inst.Operator[1].Variable.Name = HazeValueNumberToString(inst.Operator[1].Variable.Type.BaseType, result);
                        const_inst.Operator[1].Extra.RuntimeDynamicValue = result;

                        replaceInstruction(function, i, const_inst);
                        m_stats.constants_folded++;
                    }
                }
            }
        }
    }
}

bool Optimizer::EvaluateConstantExpression(const ModuleUnit::FunctionInstruction& inst, HazeValue& result)
{
    if (inst.Operator.size() < 2)
    {
        return false;
    }
    
    CalculateValueByType(inst.Operator[0].Variable.Type.BaseType, inst.InsCode, result, inst.Operator[1].Extra.RuntimeDynamicValue, inst.Operator[2].Extra.RuntimeDynamicValue);
    return false;
}

// ==================== 死代码消除 ====================

void Optimizer::DeadCodeElimination(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        if (function.Instructions.size() > 0)
        {
            ControlFlowGraph cfg;
            AnalyzeDataFlow(function, cfg);
            markDeadInstructions(function, cfg);

            // 移除死指令
            HAZE_LOG_INFO_W("开始消除<%s>\n", function.Name.c_str());
            for (size_t i = 0; i < function.Instructions.size(); i++)
            {
                if (m_instruction_analysis[i].is_dead)
                {
                    HAZE_LOG_INFO_W("消除<%s><d>\n", GetInstructionString(function.Instructions.at(i).InsCode), i);
                }
            }

            for (int i = static_cast<int>(function.Instructions.size()) - 1; i >= 0; --i)
            {
                if (m_instruction_analysis[i].is_dead)
                {
                    removeInstruction(function, i);
                    m_stats.instructions_eliminated++;


                    // 需要重新设置Block的数据
                    for (x_uint64 j = 0; j < function.Blocks.size(); j++)
                    {
                        if (i >= function.Blocks[j].StartAddress && i < function.Blocks[j].StartAddress + function.Blocks[j].InstructionNum)
                        {
                            function.Blocks[j].InstructionNum--;

                            for (x_uint64 k = j + 1; k < function.Blocks.size(); k++)
                            {
                                function.Blocks[k].StartAddress--;
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
}

void Optimizer::AnalyzeDataFlow(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    // 为每个块计算def和use集合
    for (auto& block : cfg.blocks)
    {
        block.defs.clear();
        block.uses.clear();
        
        for (size_t inst_idx : block.instructions)
        {
            auto& inst = function.Instructions[inst_idx];
            
            // 获取定义的变量
            auto defs = getDefinedVariables(inst);
            for (const auto& def : defs)
            {
                block.defs.insert(def);
            }
            
            // 获取使用的变量
            auto uses = getUsedVariables(inst);
            for (const auto& use : uses)
            {
                if (block.defs.find(use) == block.defs.end())
                {
                    block.uses.insert(use);
                }
            }
        }
    }
    
    // 计算活跃变量
    bool changed = true;
    while (changed)
    {
        changed = false;
        
        for (auto& block : cfg.blocks)
        {
            std::unordered_set<HString> old_live_out = block.live_out;
            
            // live_out[B] = union(live_in[S] for S in successors(B))
            block.live_out.clear();
            for (size_t succ_idx : block.successors)
            {
                for (const auto& var : cfg.blocks[succ_idx].live_in)
                {
                    block.live_out.insert(var);
                }
            }
            
            // live_in[B] = use[B] union (live_out[B] - def[B])
            block.live_in.clear();
            for (const auto& use : block.uses)
            {
                block.live_in.insert(use);
            }
            
            for (const auto& var : block.live_out)
            {
                if (block.defs.find(var) == block.defs.end())
                {
                    block.live_in.insert(var);
                }
            }
            
            if (block.live_out != old_live_out)
            {
                changed = true;
            }
        }
    }
}

void Optimizer::markDeadInstructions(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    m_instruction_analysis.clear();
    
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        m_instruction_analysis[i] = InstructionAnalysis();
    }
    
    // 第一遍：标记基于变量活跃性的死指令
    markDeadInstructionsByLiveness(function, cfg);
    
    // 第二遍：标记不可达代码
    markUnreachableCode(function, cfg);
    
    // 第三遍：标记无副作用的死指令
    markDeadInstructionsWithoutSideEffects(function, cfg);
    
    // 第四遍：标记冗余指令
    markRedundantInstructions(function, cfg);
    
    // 第五遍：标记死循环和死分支
    markDeadLoopsAndBranches(function, cfg);
}

void Optimizer::markDeadInstructionsByLiveness(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    // 标记死指令
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        auto& analysis = m_instruction_analysis[i];
        
        // 获取定义的变量
        auto defs = getDefinedVariables(inst);
        
        // 检查是否为死指令
        bool is_dead = true;
        for (const auto& def : defs)
        {
            if (isVariableLive(def, i, cfg))
            {
                is_dead = false;
                break;
            }
        }
        
        // 特殊指令不能删除
        if (IsJmpOpCode(inst.InsCode) || inst.InsCode == InstructionOpCode::RET || 
            inst.InsCode == InstructionOpCode::CALL || inst.InsCode == InstructionOpCode::LINE)
        {
            is_dead = false;
        }
        
        analysis.is_dead = is_dead;
    }
}

void Optimizer::markUnreachableCode(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    // 标记不可达的代码块
    std::vector<bool> reachable_instructions(function.Instructions.size(), false);
    
    // 从入口点开始标记可达指令
    markReachableInstructions(function, cfg, 0, reachable_instructions);
    
    // 标记不可达指令为死代码
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        if (!reachable_instructions[i])
        {
            auto& inst = function.Instructions[i];
            
            // 某些指令即使不可达也不能删除（如标签）
            if (inst.InsCode != InstructionOpCode::LINE)
            {
                m_instruction_analysis[i].is_dead = true;
                m_instruction_analysis[i].can_be_eliminated = true;
            }
        }
    }
}

void Optimizer::markReachableInstructions(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg, 
                                         size_t start_inst, std::vector<bool>& reachable)
{
    if (start_inst >= function.Instructions.size() || reachable[start_inst])
    {
        return;
    }
    
    reachable[start_inst] = true;
    
    auto& inst = function.Instructions[start_inst];
    
    // 如果是跳转指令，递归标记目标
    if (IsJmpOpCode(inst.InsCode))
    {
        if (inst.InsCode == InstructionOpCode::JMP)
        {
            // 无条件跳转
            HString target = inst.Operator[0].Variable.Name;
            size_t target_inst = findLabelInstruction(function, target);
            if (target_inst < function.Instructions.size())
            {
                markReachableInstructions(function, cfg, target_inst, reachable);
            }
        }
        else
        {
            // 条件跳转 - 两个分支都可能执行
            if (inst.Operator.size() >= 1)
            {
                HString true_target = inst.Operator[0].Variable.Name;
                size_t true_inst = findLabelInstruction(function, true_target);
                if (true_inst < function.Instructions.size())
                {
                    markReachableInstructions(function, cfg, true_inst, reachable);
                }
            }
            
            if (inst.Operator.size() >= 2)
            {
                HString false_target = inst.Operator[1].Variable.Name;
                size_t false_inst = findLabelInstruction(function, false_target);
                if (false_inst < function.Instructions.size())
                {
                    markReachableInstructions(function, cfg, false_inst, reachable);
                }
            }
            
            // fall-through
            if (start_inst + 1 < function.Instructions.size())
            {
                markReachableInstructions(function, cfg, start_inst + 1, reachable);
            }
        }
    }
    else if (inst.InsCode != InstructionOpCode::RET)
    {
        // 顺序执行
        if (start_inst + 1 < function.Instructions.size())
        {
            markReachableInstructions(function, cfg, start_inst + 1, reachable);
        }
    }
}

void Optimizer::markDeadInstructionsWithoutSideEffects(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        auto& analysis = m_instruction_analysis[i];
        
        // 如果指令已经被标记为死代码，跳过
        if (analysis.is_dead)
        {
            continue;
        }
        
        // 检查是否为无副作用的指令
        if (isInstructionWithoutSideEffects(inst))
        {
            // 检查结果是否被使用
            auto defs = getDefinedVariables(inst);
            bool has_used_result = false;
            
            for (const auto& def : defs)
            {
                if (isVariableLive(def, i, cfg))
                {
                    has_used_result = true;
                    break;
                }
            }
            
            if (!has_used_result)
            {
                analysis.is_dead = true;
                analysis.can_be_eliminated = true;
            }
        }
    }
}

void Optimizer::markRedundantInstructions(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        auto& analysis = m_instruction_analysis[i];
        
        // 如果指令已经被标记为死代码，跳过
        if (analysis.is_dead)
        {
            continue;
        }
        
        // 检查冗余指令
        if (isRedundantInstruction(inst, i, function, cfg))
        {
            analysis.is_dead = true;
            analysis.can_be_eliminated = true;
        }
    }
}

void Optimizer::markDeadLoopsAndBranches(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        auto& analysis = m_instruction_analysis[i];
        
        // 如果指令已经被标记为死代码，跳过
        if (analysis.is_dead)
        {
            continue;
        }
        
        // 检查死循环
        if (isDeadLoop(inst, i, function, cfg))
        {
            analysis.is_dead = true;
            analysis.can_be_eliminated = true;
        }
        
        // 检查死分支
        if (isDeadBranch(inst, i, function, cfg))
        {
            analysis.is_dead = true;
            analysis.can_be_eliminated = true;
        }
    }
}

// 辅助函数
size_t Optimizer::findLabelInstruction(ModuleUnit::FunctionTableData& function, const HString& label)
{
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        if (inst.InsCode == InstructionOpCode::LINE)
        {
            if (i + 1 < function.Instructions.size())
            {
                auto& next_inst = function.Instructions[i + 1];
                if (next_inst.Operator.size() > 0 && next_inst.Operator[0].Variable.Name == label)
                {
                    return i + 1;
                }
            }
        }
    }
    return function.Instructions.size(); // 未找到
}

bool Optimizer::isInstructionWithoutSideEffects(const ModuleUnit::FunctionInstruction& inst)
{
    // 纯计算指令，无副作用
    switch (inst.InsCode)
    {
    case InstructionOpCode::ADD:
    case InstructionOpCode::SUB:
    case InstructionOpCode::MUL:
    case InstructionOpCode::DIV:
    case InstructionOpCode::MOD:
    case InstructionOpCode::NEG:
    case InstructionOpCode::NOT:
    case InstructionOpCode::BIT_AND:
    case InstructionOpCode::BIT_OR:
    case InstructionOpCode::BIT_XOR:
    case InstructionOpCode::BIT_NEG:
    case InstructionOpCode::SHL:
    case InstructionOpCode::SHR:
    case InstructionOpCode::CMP:
        return true;
    default:
        return false;
    }
}

bool Optimizer::isRedundantInstruction(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                                      ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    // 检查冗余的MOV指令
    if (inst.InsCode == InstructionOpCode::MOV && inst.Operator.size() >= 2)
    {
        HString dest = inst.Operator[0].Variable.Name;
        HString src = inst.Operator[1].Variable.Name;
        
        // MOV x, x 是冗余的
        if (dest == src)
        {
            return true;
        }
        
        // 检查是否覆盖了之前的定义
        for (int i = static_cast<int>(index) - 1; i >= 0; --i)
        {
            auto& prev_inst = function.Instructions[i];
            auto prev_defs = getDefinedVariables(prev_inst);
            
            for (const auto& prev_def : prev_defs)
            {
                if (prev_def == dest)
                {
                    // 找到了之前的定义，检查是否被使用
                    if (!isVariableLive(prev_def, i, cfg))
                    {
                        return true; // 冗余的MOV
                    }
                    break;
                }
            }
        }
    }
    
    return false;
}

bool Optimizer::isDeadLoop(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                          ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    // 检查死循环：条件永远为false的循环
    if (IsJmpOpCode(inst.InsCode) && inst.InsCode != InstructionOpCode::JMP)
    {
        // 检查条件是否永远为false
        if (isConditionAlwaysFalse(inst, index, function))
        {
            return true;
        }
    }
    
    return false;
}

bool Optimizer::isDeadBranch(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                            ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg)
{
    // 检查死分支：条件永远为true的分支
    if (IsJmpOpCode(inst.InsCode) && inst.InsCode != InstructionOpCode::JMP)
    {
        // 检查条件是否永远为true
        if (isConditionAlwaysTrue(inst, index, function))
        {
            return true;
        }
    }
    
    return false;
}

bool Optimizer::isConditionAlwaysFalse(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                                      ModuleUnit::FunctionTableData& function)
{
    // 简化实现：检查一些明显的永远为false的条件
    // 实际实现需要更复杂的常量传播和条件分析
    
    // 例如：CMP 0, 0; JE target (永远为true)
    if (index > 0)
    {
        auto& prev_inst = function.Instructions[index - 1];
        if (prev_inst.InsCode == InstructionOpCode::CMP)
        {
            // 检查比较的两个操作数是否相等
            if (prev_inst.Operator.size() >= 2)
            {
                auto& op1 = prev_inst.Operator[0];
                auto& op2 = prev_inst.Operator[1];
                
                if (op1.Desc == HazeDataDesc::Constant && op2.Desc == HazeDataDesc::Constant)
                {
                    // 简化：检查是否为相同的常量
                    if (op1.Variable.Name == op2.Variable.Name)
                    {
                        // CMP相同值，然后JE永远为true，JNE永远为false
                        if (inst.InsCode == InstructionOpCode::JNE)
                        {
                            return true; // JNE永远为false
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

bool Optimizer::isConditionAlwaysTrue(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                                     ModuleUnit::FunctionTableData& function)
{
    // 类似isConditionAlwaysFalse的逻辑
    if (index > 0)
    {
        auto& prev_inst = function.Instructions[index - 1];
        if (prev_inst.InsCode == InstructionOpCode::CMP)
        {
            if (prev_inst.Operator.size() >= 2)
            {
                auto& op1 = prev_inst.Operator[0];
                auto& op2 = prev_inst.Operator[1];
                
                if (op1.Desc == HazeDataDesc::Constant && op2.Desc == HazeDataDesc::Constant)
                {
                    if (op1.Variable.Name == op2.Variable.Name)
                    {
                        if (inst.InsCode == InstructionOpCode::JE)
                        {
                            return true; // JE永远为true
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

// ==================== 窥孔优化 ====================

void Optimizer::PeepholeOptimization(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        if (function.Instructions.size() <= 1)
        {
            continue;
        }

        V_Array<x_uint64> blockEndInsIndex;
        for (x_uint64 i = 0; i < function.Blocks.size(); i++)
        {
            blockEndInsIndex.push_back(function.Blocks[i].InstructionNum + function.Blocks[i].StartAddress - 1);
        }

        // 单次遍历，应用所有可能的优化
        for (x_uint64 i = 0; i < function.Instructions.size() - 1; ++i)
        {
            bool canNext = true;
            for (auto& index : blockEndInsIndex)
            {
                if (index == i)
                {
                    canNext = false;
                    break;
                }
            }

            if (OptimizeInstructionSequence(function.Instructions, i, canNext))
            {
                m_stats.instructions_eliminated++;

                // 需要重新设置Block的数据
                for (x_uint64 j = 0; j < function.Blocks.size(); j++)
                {
                    if (i >= function.Blocks[j].StartAddress && i < function.Blocks[j].StartAddress + function.Blocks[j].InstructionNum)
                    {
                        function.Blocks[j].InstructionNum--;
                        
                        for (x_uint64 k = j + 1; k < function.Blocks.size(); k++)
                        {
                            function.Blocks[k].StartAddress--;
                        }

                        for (auto& endIndex : blockEndInsIndex)
                        {
                            if (endIndex >= i)
                            {
                                endIndex--;
                            }
                        }
                        
                        break;
                    }
                }

                // 由于删除了指令，需要重新检查当前位置
                if (i > 0)
                {
                    i--;
                }
            }
        }
    }
}

bool Optimizer::OptimizeInstructionSequence(std::vector<ModuleUnit::FunctionInstruction>& instructions, size_t start, bool canOptNextIns)
{
    bool tryOptTwo = canOptNextIns;
    if (start + 1 >= instructions.size())
    {
        tryOptTwo = false;
    }
    
    auto& inst1 = instructions[start];
    if (tryOptTwo)
    {
        auto& inst2 = instructions[start + 1];

        // 优化1: MOV x, y; MOV y, x -> MOV x, y
        if (inst1.InsCode == InstructionOpCode::MOV && inst2.InsCode == InstructionOpCode::MOV)
        {
            if (inst1.Operator.size() >= 2 && inst2.Operator.size() >= 2)
            {
                if (inst1.Operator[0].Variable.Name == inst2.Operator[1].Variable.Name &&
                    inst1.Operator[1].Variable.Name == inst2.Operator[0].Variable.Name)
                {
                    // 移除第二个MOV指令
                    instructions.erase(instructions.begin() + start + 1);
                    return true;
                }
            }
        }

        // 优化4: 连续的跳转优化
        if (IsJmpOpCode(inst1.InsCode) && IsJmpOpCode(inst2.InsCode))
        {
            if (inst1.InsCode == InstructionOpCode::JMP)
            {
                // 无条件跳转后的指令不可达
                instructions.erase(instructions.begin() + start + 1);
                return true;
            }
        }
    }
    
    // 优化2: ADD或SUB的第三个操作数为0
    if ((inst1.InsCode == InstructionOpCode::ADD || inst1.InsCode == InstructionOpCode::SUB) && inst1.Operator[0].Variable.Name == inst1.Operator[1].Variable.Name)
    {
        auto& oper2 = inst1.Operator[2];
        if (IsConstant(oper2.Desc) && IsNumberZero(oper2.Variable.Type.BaseType, oper2.Extra.RuntimeDynamicValue))
        {
            instructions.erase(instructions.begin() + start);
            return true;
        }
    }
    
    // 优化3: MUL或DIV的第三个操作数为1
    if ((inst1.InsCode == InstructionOpCode::MUL || inst1.InsCode == InstructionOpCode::DIV) && inst1.Operator[0].Variable.Name == inst1.Operator[1].Variable.Name)
    {
        auto& oper2 = inst1.Operator[2];
        if (IsConstant(oper2.Desc) && IsNumberOne(oper2.Variable.Type.BaseType, oper2.Extra.RuntimeDynamicValue))
        {
            instructions.erase(instructions.begin() + start);
            return true;
        }
    }

    return false;
}

std::vector<HString> Optimizer::getUsedVariables(const ModuleUnit::FunctionInstruction& inst)
{
    std::vector<HString> used;
    
    for (const auto& operand : inst.Operator)
    {
        if (operand.Desc != HazeDataDesc::Constant && 
            operand.Desc != HazeDataDesc::ConstantString &&
            operand.Desc != HazeDataDesc::NullPtr)
        {
            used.push_back(operand.Variable.Name);
        }
    }
    
    return used;
}

std::vector<HString> Optimizer::getDefinedVariables(const ModuleUnit::FunctionInstruction& inst)
{
    std::vector<HString> defined;
    
    if (!inst.Operator.empty())
    {
        defined.push_back(inst.Operator[0].Variable.Name);
    }
    
    return defined;
}

bool Optimizer::isVariableLive(const HString& var, size_t instruction_index, const ControlFlowGraph& cfg)
{
    auto it = cfg.instruction_to_block.find(instruction_index);
    if (it == cfg.instruction_to_block.end()) return false;
    
    size_t block_idx = it->second;
    if (block_idx >= cfg.blocks.size()) return false;
    
    const auto& block = cfg.blocks[block_idx];
    
    // 检查变量是否在后续指令中使用
    for (size_t inst_idx : block.instructions)
    {
        if (inst_idx > instruction_index)
        {
            // 这里需要检查指令是否使用了该变量
            // 简化实现，实际应该分析指令的use集合
            return true;
        }
    }
    
    // 检查是否在后续块中活跃
    for (size_t succ_idx : block.successors)
    {
        if (cfg.blocks[succ_idx].live_in.find(var) != cfg.blocks[succ_idx].live_in.end())
        {
            return true;
        }
    }
    
    return false;
}

void Optimizer::replaceInstruction(ModuleUnit::FunctionTableData& function, size_t index, const ModuleUnit::FunctionInstruction& new_inst)
{
    if (index < function.Instructions.size())
    {
        function.Instructions[index] = new_inst;
    }
}

void Optimizer::removeInstruction(ModuleUnit::FunctionTableData& function, size_t index)
{
    if (index < function.Instructions.size())
    {
        function.Instructions.erase(function.Instructions.begin() + index);
    }
}

void Optimizer::insertInstruction(ModuleUnit::FunctionTableData& function, size_t index, const ModuleUnit::FunctionInstruction& inst)
{
    if (index <= function.Instructions.size())
    {
        function.Instructions.insert(function.Instructions.begin() + index, inst);
    }
}

// ==================== 常量传播优化 ====================

void Optimizer::constantPropagation(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        constantPropagationFunction(function);
    }
}

void Optimizer::constantPropagationFunction(ModuleUnit::FunctionTableData& function)
{
    // 构建控制流图
    ControlFlowGraph cfg;
    //buildControlFlowGraph(function, cfg);
    
    // 初始化常量映射
    std::unordered_map<HString, HazeValue> constant_map;
    
    // 前向传播常量
    bool changed = true;
    while (changed)
    {
        changed = false;
        
        for (size_t i = 0; i < function.Instructions.size(); ++i)
        {
            auto& inst = function.Instructions[i];
            
            // 检查是否可以传播常量
            if (canPropagateConstant(inst, constant_map))
            {
                if (propagateConstant(inst, constant_map))
                {
                    changed = true;
                    m_stats.constants_propagated++;
                }
            }
            
            // 更新常量映射
            updateConstantMap(inst, constant_map);
        }
    }
}

bool Optimizer::canPropagateConstant(const ModuleUnit::FunctionInstruction& inst, 
                                    const std::unordered_map<HString, HazeValue>& constant_map)
{
    // 检查指令的操作数是否都是已知常量
    for (int i = 1; i < inst.Operator.size(); ++i)
    {
        if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
            inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
            inst.Operator[i].Desc != HazeDataDesc::NullPtr)
        {
            HString var_name = inst.Operator[i].Variable.Name;
            if (constant_map.find(var_name) == constant_map.end())
            {
                return false;
            }
        }
    }
    return true;
}

bool Optimizer::propagateConstant(ModuleUnit::FunctionInstruction& inst, 
                                 const std::unordered_map<HString, HazeValue>& constant_map)
{
    // 将变量替换为常量值
    for (int i = 1; i < inst.Operator.size(); ++i)
    {
        if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
            inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
            inst.Operator[i].Desc != HazeDataDesc::NullPtr)
        {
            HString var_name = inst.Operator[i].Variable.Name;
            auto it = constant_map.find(var_name);
            if (it != constant_map.end())
            {
                inst.Operator[i].Desc = HazeDataDesc::Constant;
                inst.Operator[i].Variable.Name = HazeValueNumberToString(inst.Operator[i].Variable.Type.BaseType, it->second);
                return true;
            }
        }
    }
    return false;
}

void Optimizer::updateConstantMap(const ModuleUnit::FunctionInstruction& inst, 
                                 std::unordered_map<HString, HazeValue>& constant_map)
{
    // 更新常量映射
    if (inst.Operator[0].Desc != HazeDataDesc::Constant && 
        inst.Operator[0].Desc != HazeDataDesc::ConstantString &&
        inst.Operator[0].Desc != HazeDataDesc::NullPtr)
    {
        HString var_name = inst.Operator[0].Variable.Name;
        
        if (inst.InsCode == InstructionOpCode::MOV && 
            inst.Operator[1].Desc == HazeDataDesc::Constant)
        {
            // 常量赋值
            HazeValue value = inst.Operator[1].Extra.RuntimeDynamicValue;
            constant_map[var_name] = value;
        }
        else if (IsArithmeticOpCode(inst.InsCode) && canEvaluateConstant(inst))
        {
            // 常量表达式
            HazeValue result;
            if (EvaluateConstantExpression(inst, result))
            {
                constant_map[var_name] = result;
            }
        }
        else
        {
            // 变量被重新定义，移除常量信息
            constant_map.erase(var_name);
        }
    }
}

bool Optimizer::canEvaluateConstant(const ModuleUnit::FunctionInstruction& inst)
{
    // 检查是否可以计算常量值
    for (int i = 1; i < inst.Operator.size(); ++i)
    {
        if (inst.Operator[i].Desc != HazeDataDesc::Constant)
        {
            return false;
        }
    }
    return true;
}

// ==================== 循环优化 ====================

void Optimizer::loopOptimization(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        loopOptimizationFunction(function);
    }
}

void Optimizer::loopOptimizationFunction(ModuleUnit::FunctionTableData& function)
{
    if (function.Instructions.size() <= 0)
    {
        return;
    }

    ControlFlowGraph cfg;
    //buildControlFlowGraph(function, cfg);
    
    // 识别循环
    std::vector<LoopInfo> loops = identifyLoops(function, cfg);
    
    for (auto& loop : loops)
    {
        // 循环不变代码外提
        hoistInvariantCode(function, loop, cfg);
        
        // 循环展开
        if (shouldUnrollLoop(loop, cfg))
        {
            unrollLoop(function, loop, cfg);
        }
        
        // 强度削弱 - 这里应该调用针对单个函数的强度削弱
        // strengthReduction是针对整个模块的，不适合在这里调用
    }
}

std::vector<Optimizer::LoopInfo> Optimizer::identifyLoops(ModuleUnit::FunctionTableData& function, const ControlFlowGraph& cfg)
{
    std::vector<LoopInfo> loops;
    
    // 使用深度优先搜索识别回边
    std::vector<bool> visited(cfg.blocks.size(), false);
    std::vector<bool> in_stack(cfg.blocks.size(), false);
    std::vector<size_t> dfs_stack;
    
    for (size_t i = 0; i < cfg.blocks.size(); ++i)
    {
        if (!visited[i])
        {
            dfsFindLoops(i, function, cfg, visited, in_stack, dfs_stack, loops);
        }
    }
    
    return loops;
}

void Optimizer::dfsFindLoops(size_t block_idx, ModuleUnit::FunctionTableData& function, const ControlFlowGraph& cfg,
                            std::vector<bool>& visited, std::vector<bool>& in_stack,
                            std::vector<size_t>& dfs_stack, std::vector<LoopInfo>& loops)
{
    visited[block_idx] = true;
    in_stack[block_idx] = true;
    dfs_stack.push_back(block_idx);
    
    for (size_t succ : cfg.blocks[block_idx].successors)
    {
        if (!visited[succ])
        {
            dfsFindLoops(succ, function, cfg, visited, in_stack, dfs_stack, loops);
        }
        else if (in_stack[succ])
        {
            // 找到回边，识别循环
            LoopInfo loop;
            loop.header = succ;
            loop.body.clear();
            
            // 从栈中找到循环体
            for (int i = dfs_stack.size() - 1; i >= 0; --i)
            {
                size_t current = dfs_stack[i];
                loop.body.insert(current);
                if (current == succ) break;
            }
            
            loops.push_back(loop);
        }
    }
    
    in_stack[block_idx] = false;
    dfs_stack.pop_back();
}

void Optimizer::hoistInvariantCode(ModuleUnit::FunctionTableData& function, const LoopInfo& loop, const ControlFlowGraph& cfg)
{
    // 识别循环不变代码
    std::vector<size_t> invariant_instructions;
    
    for (size_t block_idx : loop.body)
    {
        const auto& block = cfg.blocks[block_idx];
        for (size_t inst_idx : block.instructions)
        {
            auto& inst = function.Instructions[inst_idx];
            
            if (isInvariantInstruction(inst, loop, cfg, function))
            {
                invariant_instructions.push_back(inst_idx);
            }
        }
    }
    
    // 将不变代码外提到循环头
    for (size_t inst_idx : invariant_instructions)
    {
        auto inst = function.Instructions[inst_idx];
        
        // 插入到循环头之前
        insertInstruction(function, loop.header, inst);
        
        // 从原位置删除
        removeInstruction(function, inst_idx);
        
        m_stats.loop_invariant_hoisted++;
    }
}

bool Optimizer::isInvariantInstruction(const ModuleUnit::FunctionInstruction& inst, const LoopInfo& loop, const ControlFlowGraph& cfg, const ModuleUnit::FunctionTableData& function)
{
    // 检查指令的操作数是否都是循环不变的
    for (int i = 1; i < inst.Operator.size(); ++i)
    {
        if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
            inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
            inst.Operator[i].Desc != HazeDataDesc::NullPtr)
        {
            HString var_name = inst.Operator[i].Variable.Name;
            if (isVariableModifiedInLoop(var_name, loop, cfg, function))
            {
                return false;
            }
        }
    }
    return true;
}

bool Optimizer::isVariableModifiedInLoop(const HString& var_name, const LoopInfo& loop, const ControlFlowGraph& cfg, const ModuleUnit::FunctionTableData& function)
{
    for (size_t block_idx : loop.body)
    {
        const auto& block = cfg.blocks[block_idx];
        for (size_t inst_idx : block.instructions)
        {
            auto& inst = function.Instructions[inst_idx];
            if (isVariableDefined(inst, var_name))
            {
                return true;
            }
        }
    }
    return false;
}

bool Optimizer::isVariableDefined(const ModuleUnit::FunctionInstruction& inst, const HString& var_name)
{
    return inst.Operator[0].Desc != HazeDataDesc::Constant && 
           inst.Operator[0].Desc != HazeDataDesc::ConstantString &&
           inst.Operator[0].Desc != HazeDataDesc::NullPtr &&
           inst.Operator[0].Variable.Name == var_name;
}

bool Optimizer::shouldUnrollLoop(const LoopInfo& loop, const ControlFlowGraph& cfg)
{
    // 简单的循环展开判断
    int loop_size = 0;
    for (size_t block_idx : loop.body)
    {
        loop_size += cfg.blocks[block_idx].instructions.size();
    }
    
    // 如果循环体较小且迭代次数可预测，则展开
    return loop_size <= 10 && hasPredictableIterations(loop);
}

bool Optimizer::hasPredictableIterations(const LoopInfo& loop)
{
    // 检查循环是否有可预测的迭代次数
    // 这里简化实现，实际需要更复杂的分析
    return true;
}

void Optimizer::unrollLoop(ModuleUnit::FunctionTableData& function, const LoopInfo& loop, const ControlFlowGraph& cfg)
{
    // 循环展开实现
    // 将循环体复制多次，减少循环开销
    
    std::vector<ModuleUnit::FunctionInstruction> unrolled_instructions;
    
    // 假设展开因子为4
    const int unroll_factor = 4;
    
    for (int i = 0; i < unroll_factor; ++i)
    {
        for (size_t block_idx : loop.body)
        {
            const auto& block = cfg.blocks[block_idx];
            for (size_t inst_idx : block.instructions)
            {
                auto inst = function.Instructions[inst_idx];
                unrolled_instructions.push_back(inst);
            }
        }
    }
    
    // 替换原循环
    // 这里需要更复杂的实现来处理循环控制逻辑
    
    m_stats.loops_unrolled++;
}

// ==================== 强度削弱 ====================

void Optimizer::strengthReduction(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        strengthReductionFunction(function);
    }
}

void Optimizer::strengthReductionFunction(ModuleUnit::FunctionTableData& function)
{
    // 强度削弱：将乘法转换为加法
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        
        if (inst.InsCode == InstructionOpCode::MUL)
        {
            // 检查是否可以转换为加法
            if (canConvertToAddition(inst))
            {
                convertToAddition(function, i);
                m_stats.strength_reductions++;
            }
        }
    }
}

bool Optimizer::canConvertToAddition(const ModuleUnit::FunctionInstruction& inst)
{
    // 检查乘法是否可以转换为加法
    // 例如：x * 2 可以转换为 x + x
    if (inst.Operator[2].Desc == HazeDataDesc::Constant)
    {
        int constant = StringToStandardType<int>(inst.Operator[2].Variable.Name);
        return constant == 2 || constant == 4 || constant == 8;
    }
    return false;
}

void Optimizer::convertToAddition(ModuleUnit::FunctionTableData& function, size_t inst_idx)
{
    auto& inst = function.Instructions[inst_idx];
    int constant = StringToStandardType<int>(inst.Operator[2].Variable.Name);
    
    if (constant == 2)
    {
        // x * 2 -> x + x
        inst.InsCode = InstructionOpCode::ADD;
        inst.Operator[2] = inst.Operator[1]; // 复制操作数
    }
    else if (constant == 4)
    {
        // x * 4 -> (x + x) + (x + x)
        // 需要插入额外的指令
        ModuleUnit::FunctionInstruction add_inst1, add_inst2;
        add_inst1.InsCode = InstructionOpCode::ADD;
        add_inst1.Operator[0] = inst.Operator[0];
        add_inst1.Operator[1] = inst.Operator[1];
        add_inst1.Operator[2] = inst.Operator[1];
        
        add_inst2.InsCode = InstructionOpCode::ADD;
        add_inst2.Operator[0] = inst.Operator[0];
        add_inst2.Operator[1] = inst.Operator[0];
        add_inst2.Operator[2] = inst.Operator[0];
        
        insertInstruction(function, inst_idx, add_inst1);
        insertInstruction(function, inst_idx + 1, add_inst2);
        removeInstruction(function, inst_idx + 2);
    }
}

// ==================== 函数内联 ====================

void Optimizer::functionInlining(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        functionInliningFunction(function, module);
    }
}

void Optimizer::functionInliningFunction(ModuleUnit::FunctionTableData& function, const ModuleUnit& module)
{
    std::vector<size_t> call_sites;
    
    // 找到所有函数调用点
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        if (IsCallOpCode(inst.InsCode))
        {
            call_sites.push_back(i);
        }
    }
    
    // 从后往前处理，避免索引变化
    for (int i = call_sites.size() - 1; i >= 0; --i)
    {
        size_t call_site = call_sites[i];
        auto& inst = function.Instructions[call_site];
        
        HString callee_name = inst.Operator[0].Variable.Name;
        
        // 检查是否可以内联
        if (shouldInlineFunction(callee_name, module))
        {
            inlineFunctionCall(function, call_site, callee_name, module);
        }
    }
}

bool Optimizer::shouldInlineFunction(const HString& function_name, const ModuleUnit& module)
{
    // 查找被调用函数
    const ModuleUnit::FunctionTableData* callee = nullptr;
    for (const auto& func : module.m_FunctionTable.m_Functions)
    {
        if (func.Name == function_name)
        {
            callee = &func;
            break;
        }
    }
    
    if (!callee) return false;
    
    // 内联条件：
    // 1. 函数体较小（指令数 <= 阈值）
    // 2. 不是递归函数
    // 3. 没有复杂的控制流
    
    return callee->Instructions.size() <= m_config.max_inline_size &&
           !isRecursiveFunction(*callee) &&
           !hasComplexControlFlow(*callee);
}

bool Optimizer::isRecursiveFunction(const ModuleUnit::FunctionTableData& function)
{
    // 检查函数是否递归调用自己
    for (const auto& inst : function.Instructions)
    {
        if (IsCallOpCode(inst.InsCode) && inst.Operator[0].Variable.Name == function.Name)
        {
            return true;
        }
    }
    return false;
}

bool Optimizer::hasComplexControlFlow(const ModuleUnit::FunctionTableData& function)
{
    // 检查是否有复杂的控制流（多个跳转、循环等）
    int jump_count = 0;
    for (const auto& inst : function.Instructions)
    {
        if (IsJmpOpCode(inst.InsCode))
        {
            jump_count++;
        }
    }
    return jump_count > 3; // 超过3个跳转认为复杂
}

void Optimizer::inlineFunctionCall(ModuleUnit::FunctionTableData& function, size_t call_site, const HString& callee_name, const ModuleUnit& module)
{
    // 获取被调用函数
    const ModuleUnit::FunctionTableData* callee = nullptr;
    for (const auto& func : module.m_FunctionTable.m_Functions)
    {
        if (func.Name == callee_name)
        {
            callee = &func;
            break;
        }
    }
    
    if (!callee) return;
    
    // 创建内联后的指令序列
    std::vector<ModuleUnit::FunctionInstruction> inlined_instructions;
    
    // 1. 参数传递
    // 这里需要根据实际的参数传递机制来实现
    
    // 2. 复制函数体
    for (const auto& inst : callee->Instructions)
    {
        if (inst.InsCode != InstructionOpCode::LINE) // 跳过行号指令
        {
            ModuleUnit::FunctionInstruction new_inst = inst;
            // 重命名局部变量以避免冲突
            renameLocalVariables(new_inst, function.Name);
            inlined_instructions.push_back(new_inst);
        }
    }
    
    // 3. 替换函数调用
    removeInstruction(function, call_site);
    for (size_t i = 0; i < inlined_instructions.size(); ++i)
    {
        insertInstruction(function, call_site + i, inlined_instructions[i]);
    }
    
    m_stats.functions_inlined++;
}

void Optimizer::renameLocalVariables(ModuleUnit::FunctionInstruction& inst, const HString& caller_name)
{
    // 重命名局部变量以避免冲突
    for (int i = 0; i < inst.Operator.size(); ++i)
    {
        if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
            inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
            inst.Operator[i].Desc != HazeDataDesc::NullPtr)
        {
            HString old_name = inst.Operator[i].Variable.Name;
            if (isLocalVariable(old_name))
            {
                HString new_name = caller_name + H_TEXT("_") + old_name;
                inst.Operator[i].Variable.Name = new_name;
            }
        }
    }
}

bool Optimizer::isLocalVariable(const HString& var_name)
{
    // 检查是否为局部变量
    // 这里简化实现，实际需要更复杂的分析
    return !var_name.empty() && var_name[0] != '@';
}

// ==================== 公共子表达式消除 ====================

void Optimizer::commonSubexpressionElimination(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        commonSubexpressionEliminationFunction(function);
    }
}

void Optimizer::commonSubexpressionEliminationFunction(ModuleUnit::FunctionTableData& function)
{
    std::unordered_map<HString, size_t> expression_cache;
    
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        
        if (IsArithmeticOpCode(inst.InsCode) || IsComparisonOpCode(inst.InsCode))
        {
            HString expression_key = generateExpressionKey(inst);
            
            auto it = expression_cache.find(expression_key);
            if (it != expression_cache.end())
            {
                // 找到相同的表达式，可以消除
                size_t prev_inst_idx = it->second;
                auto& prev_inst = function.Instructions[prev_inst_idx];
                
                // 检查操作数是否仍然有效
                if (areOperandsAvailable(prev_inst, i))
                {
                    // 替换为MOV指令
                    ModuleUnit::FunctionInstruction mov_inst;
                    mov_inst.InsCode = InstructionOpCode::MOV;
                    mov_inst.Operator[0] = inst.Operator[0];
                    mov_inst.Operator[1] = prev_inst.Operator[0];
                    mov_inst.Operator[1].Desc = HazeDataDesc::RegisterTemp;
                    
                    replaceInstruction(function, i, mov_inst);
                    m_stats.common_subexpressions_eliminated++;
                }
            }
            else
            {
                expression_cache[expression_key] = i;
            }
        }
    }
}

HString Optimizer::generateExpressionKey(const ModuleUnit::FunctionInstruction& inst)
{
    // 生成表达式的唯一键
    HString key = ToHazeString(static_cast<int>(inst.InsCode));
    
    for (int i = 1; i < inst.Operator.size(); ++i)
    {
        if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
            inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
            inst.Operator[i].Desc != HazeDataDesc::NullPtr)
        {
            key += H_TEXT("_") + inst.Operator[i].Variable.Name;
        }
        else if (inst.Operator[i].Desc == HazeDataDesc::Constant)
        {
            key += H_TEXT("_C") + inst.Operator[i].Variable.Name;
        }
    }
    
    return key;
}

bool Optimizer::areOperandsAvailable(const ModuleUnit::FunctionInstruction& prev_inst, size_t current_inst_idx)
{
    // 检查之前的表达式结果是否仍然可用
    // 这里简化实现，实际需要更复杂的活跃变量分析
    return true;
}

// ==================== 寄存器分配优化 ====================

void Optimizer::registerAllocation(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        registerAllocationFunction(function);
    }
}

void Optimizer::registerAllocationFunction(ModuleUnit::FunctionTableData& function)
{
    if (function.Instructions.size() <= 0)
    {
        return;
    }

    // 构建活跃变量分析
    ControlFlowGraph cfg;
    //buildControlFlowGraph(function, cfg);
    AnalyzeDataFlow(function, cfg);
    
    // 构建干扰图
    InterferenceGraph interference_graph;
    buildInterferenceGraph(function, cfg, interference_graph);
    
    // 图着色分配寄存器
    std::unordered_map<HString, int> register_assignment;
    if (colorGraph(interference_graph, register_assignment))
    {
        // 应用寄存器分配
        applyRegisterAllocation(function, register_assignment);
        m_stats.register_allocations++;
    }
}

void Optimizer::buildInterferenceGraph(ModuleUnit::FunctionTableData& function, const ControlFlowGraph& cfg,
                                      InterferenceGraph& interference_graph)
{
    interference_graph.clear();
    
    // 为每个变量创建节点
    std::unordered_set<HString> all_variables;
    for (const auto& inst : function.Instructions)
    {
        for (int i = 0; i < inst.Operator.size(); ++i)
        {
            if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
                inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
                inst.Operator[i].Desc != HazeDataDesc::NullPtr)
            {
                all_variables.insert(inst.Operator[i].Variable.Name);
            }
        }
    }
    
    for (const auto& var : all_variables)
    {
        interference_graph[var] = std::unordered_set<HString>();
    }
    
    // 构建干扰边
    for (const auto& block : cfg.blocks)
    {
        std::vector<HString> live_vars;
        
        for (size_t inst_idx : block.instructions)
        {
            auto& inst = function.Instructions[inst_idx];
            
            // 添加新定义的变量到活跃变量列表
            if (inst.Operator[0].Desc != HazeDataDesc::Constant && 
                inst.Operator[0].Desc != HazeDataDesc::ConstantString &&
                inst.Operator[0].Desc != HazeDataDesc::NullPtr)
            {
                HString def_var = inst.Operator[0].Variable.Name;
                live_vars.push_back(def_var);
            }
            
            // 为所有活跃变量对添加干扰边
            for (size_t i = 0; i < live_vars.size(); ++i)
            {
                for (size_t j = i + 1; j < live_vars.size(); ++j)
                {
                    interference_graph[live_vars[i]].insert(live_vars[j]);
                    interference_graph[live_vars[j]].insert(live_vars[i]);
                }
            }
            
            // 移除不再活跃的变量
            for (int i = 0; i < inst.Operator.size(); ++i)
            {
                if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
                    inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
                    inst.Operator[i].Desc != HazeDataDesc::NullPtr)
                {
                    HString use_var = inst.Operator[i].Variable.Name;
                    auto it = std::find(live_vars.begin(), live_vars.end(), use_var);
                    if (it != live_vars.end())
                    {
                        live_vars.erase(it);
                    }
                }
            }
        }
    }
}

bool Optimizer::colorGraph(const InterferenceGraph& interference_graph,
                          std::unordered_map<HString, int>& register_assignment)
{
    // 简化的图着色算法
    std::vector<HString> variables;
    for (const auto& pair : interference_graph)
    {
        variables.push_back(pair.first);
    }
    
    // 按度数排序（启发式）
    std::sort(variables.begin(), variables.end(),
              [&](const HString& a, const HString& b) {
                  return interference_graph.at(a).size() > interference_graph.at(b).size();
              });
    
    const int max_registers = 16; // 假设有16个寄存器
    
    for (const auto& var : variables)
    {
        std::vector<bool> used_colors(max_registers, false);
        
        // 检查相邻节点的颜色
        for (const auto& neighbor : interference_graph.at(var))
        {
            auto it = register_assignment.find(neighbor);
            if (it != register_assignment.end())
            {
                used_colors[it->second] = true;
            }
        }
        
        // 分配第一个可用颜色
        int color = 0;
        while (color < max_registers && used_colors[color])
        {
            color++;
        }
        
        if (color >= max_registers)
        {
            // 无法分配寄存器，需要溢出到内存
            return false;
        }
        
        register_assignment[var] = color;
    }
    
    return true;
}

void Optimizer::applyRegisterAllocation(ModuleUnit::FunctionTableData& function,
                                       const std::unordered_map<HString, int>& register_assignment)
{
    // 应用寄存器分配结果
    for (auto& inst : function.Instructions)
    {
        for (int i = 0; i < inst.Operator.size(); ++i)
        {
            if (inst.Operator[i].Desc != HazeDataDesc::Constant && 
                inst.Operator[i].Desc != HazeDataDesc::ConstantString &&
                inst.Operator[i].Desc != HazeDataDesc::NullPtr)
            {
                HString var_name = inst.Operator[i].Variable.Name;
                auto it = register_assignment.find(var_name);
                if (it != register_assignment.end())
                {
                    // 将变量替换为寄存器
                    inst.Operator[i].Variable.Name = H_TEXT("R") + ToHazeString(it->second);
                }
            }
        }
    }
}

// ==================== 指令调度优化 ====================

void Optimizer::instructionScheduling(ModuleUnit& module)
{
    for (auto& function : module.m_FunctionTable.m_Functions)
    {
        instructionSchedulingFunction(function);
    }
}

void Optimizer::instructionSchedulingFunction(ModuleUnit::FunctionTableData& function)
{
    // 指令调度：重新排列指令以提高流水线效率
    
    // 1. 构建依赖图
    std::vector<std::vector<size_t>> dependencies(function.Instructions.size());
    buildInstructionDependencies(function, dependencies);
    
    // 2. 拓扑排序
    std::vector<size_t> schedule;
    if (topologicalSort(dependencies, schedule))
    {
        // 3. 应用调度结果
        applyInstructionSchedule(function, schedule);
        m_stats.instructions_scheduled++;
    }
}

void Optimizer::buildInstructionDependencies(ModuleUnit::FunctionTableData& function, 
                                            std::vector<std::vector<size_t>>& dependencies)
{
    // 构建指令间的依赖关系
    for (size_t i = 0; i < function.Instructions.size(); ++i)
    {
        auto& inst = function.Instructions[i];
        
        // 检查与后续指令的依赖
        for (size_t j = i + 1; j < function.Instructions.size(); ++j)
        {
            auto& next_inst = function.Instructions[j];
            
            if (hasDependency(inst, next_inst))
            {
                dependencies[i].push_back(j);
            }
        }
    }
}

bool Optimizer::hasDependency(const ModuleUnit::FunctionInstruction& inst1, const ModuleUnit::FunctionInstruction& inst2)
{
    // 检查两条指令之间是否有依赖关系
    
    // 1. 数据依赖：inst1定义变量，inst2使用该变量
    if (inst1.Operator[0].Desc != HazeDataDesc::Constant && 
        inst1.Operator[0].Desc != HazeDataDesc::ConstantString &&
        inst1.Operator[0].Desc != HazeDataDesc::NullPtr)
    {
        HString def_var = inst1.Operator[0].Variable.Name;
        
        for (int i = 1; i < inst2.Operator.size(); ++i)
        {
            if (inst2.Operator[i].Desc != HazeDataDesc::Constant && 
                inst2.Operator[i].Desc != HazeDataDesc::ConstantString &&
                inst2.Operator[i].Desc != HazeDataDesc::NullPtr &&
                inst2.Operator[i].Variable.Name == def_var)
            {
                return true;
            }
        }
    }
    
    // 2. 控制依赖：inst1是跳转指令
    if (IsJmpOpCode(inst1.InsCode))
    {
        return true;
    }
    
    return false;
}

bool Optimizer::topologicalSort(const std::vector<std::vector<size_t>>& dependencies,
                               std::vector<size_t>& schedule)
{
    // 拓扑排序
    std::vector<int> in_degree(dependencies.size(), 0);
    
    // 计算入度
    for (const auto& deps : dependencies)
    {
        for (size_t dep : deps)
        {
            in_degree[dep]++;
        }
    }
    
    // 使用队列进行拓扑排序
    std::queue<size_t> q;
    for (size_t i = 0; i < in_degree.size(); ++i)
    {
        if (in_degree[i] == 0)
        {
            q.push(i);
        }
    }
    
    while (!q.empty())
    {
        size_t current = q.front();
        q.pop();
        schedule.push_back(current);
        
        for (size_t dep : dependencies[current])
        {
            in_degree[dep]--;
            if (in_degree[dep] == 0)
            {
                q.push(dep);
            }
        }
    }
    
    return schedule.size() == dependencies.size();
}

void Optimizer::applyInstructionSchedule(ModuleUnit::FunctionTableData& function, const std::vector<size_t>& schedule)
{
    // 应用指令调度结果
    std::vector<ModuleUnit::FunctionInstruction> new_instructions;
    new_instructions.reserve(function.Instructions.size());
    
    for (size_t index : schedule)
    {
        new_instructions.push_back(function.Instructions[index]);
    }
    
    function.Instructions = new_instructions;
}