#pragma once

#include "ModuleUnit.h"
#include "HazeDefine.h"
#include "OptimizerDefine.h"

// 前向声明
class BackendParse;

// 基本块分析
struct BasicBlock
{
    std::vector<x_uint64> instructions;  // 指令索引
    std::vector<x_uint64> successors;
    
    // 数据流分析
    std::unordered_set<STDString> defs;  // 定义集合
    std::unordered_set<STDString> uses;  // 使用集合
    std::unordered_set<STDString> live_in;
    std::unordered_set<STDString> live_out;
    
    bool is_reachable = true;
};

// 控制流图
struct ControlFlowGraph
{
    std::vector<BasicBlock> blocks;
    std::unordered_map<size_t, size_t> instruction_to_block;  // 指令到块的映射
};

// 指令分析结果
struct InstructionAnalysis
{
    bool is_constant = false;
    HazeValue constant_value;
    bool is_dead = false;
    bool can_be_eliminated = false;
    std::vector<STDString> dependencies;
    std::vector<STDString> definitions;
};

class Optimizer
{
public:
    Optimizer(BackendParse* backend_parse);
    ~Optimizer();

    // 主优化入口
    void optimize(ModuleUnit& module);
    
    // 设置优化配置
    void setConfig(const OptimizerConfig& config) { m_config = config; }
    
    // 获取优化统计
    struct OptimizationStats
    {
        size_t instructions_eliminated = 0;
        size_t constants_folded = 0;
        size_t constants_propagated = 0;
        size_t functions_inlined = 0;
        size_t loops_unrolled = 0;
        size_t loop_invariant_hoisted = 0;
        size_t strength_reductions = 0;
        size_t common_subexpressions_eliminated = 0;
        size_t register_allocations = 0;
        size_t instructions_scheduled = 0;
    };
    const OptimizationStats& getStats() const { return m_stats; }

private:
    // 常量折叠优化
    void ConstantFolding(ModuleUnit& module);
    bool EvaluateConstantExpression(const ModuleUnit::FunctionInstruction& inst, HazeValue& result);
    
    // 死代码消除
    void DeadCodeElimination(ModuleUnit& module);
    void AnalyzeDataFlow(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg);
    void markDeadInstructions(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg);
    
    // 死代码优化的各个阶段
    void markDeadInstructionsByLiveness(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg);
    void markUnreachableCode(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg);
    void markDeadInstructionsWithoutSideEffects(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg);
    void markRedundantInstructions(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg);
    void markDeadLoopsAndBranches(ModuleUnit::FunctionTableData& function);
    
    // 可达性分析
    void markReachableInstructions(ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg, 
                                  size_t start_inst, std::vector<bool>& reachable);
    
    // 辅助函数
    size_t findLabelInstruction(ModuleUnit::FunctionTableData& function, const STDString& label);
    bool isInstructionWithoutSideEffects(const ModuleUnit::FunctionInstruction& inst);
    bool isRedundantInstruction(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                               ModuleUnit::FunctionTableData& function, ControlFlowGraph& cfg);
    bool isDeadLoop(const ModuleUnit::FunctionInstruction& inst, size_t index, ModuleUnit::FunctionTableData& function);
    bool isDeadBranch(const ModuleUnit::FunctionInstruction& inst, size_t index, ModuleUnit::FunctionTableData& function);
    bool isConditionAlwaysFalse(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                               ModuleUnit::FunctionTableData& function);
    bool isConditionAlwaysTrue(const ModuleUnit::FunctionInstruction& inst, size_t index, 
                              ModuleUnit::FunctionTableData& function);
    
    // 窥孔优化
    void PeepholeOptimization(ModuleUnit& module);
    bool OptimizeInstructionSequence(std::vector<ModuleUnit::FunctionInstruction>& instructions, size_t start, bool canOptNextIns);
    
    // 指令替换
    void replaceInstruction(ModuleUnit::FunctionTableData& function, size_t index, const ModuleUnit::FunctionInstruction& new_inst);
    void removeInstruction(ModuleUnit::FunctionTableData& function, size_t index);
    void insertInstruction(ModuleUnit::FunctionTableData& function, size_t index, const ModuleUnit::FunctionInstruction& inst);
    
    // 变量分析
    std::vector<STDString> getUsedVariables(const ModuleUnit::FunctionInstruction& inst);
    std::vector<STDString> getDefinedVariables(const ModuleUnit::FunctionInstruction& inst);
    bool isVariableLive(const STDString& var, size_t instruction_index, const ControlFlowGraph& cfg);
    
        // 常量传播
    void constantPropagation(ModuleUnit& module);
    void constantPropagationFunction(ModuleUnit::FunctionTableData& function);
    bool canPropagateConstant(const ModuleUnit::FunctionInstruction& inst, 
                             const std::unordered_map<STDString, HazeValue>& constant_map);
    bool propagateConstant(ModuleUnit::FunctionInstruction& inst, 
                          const std::unordered_map<STDString, HazeValue>& constant_map);
    void updateConstantMap(const ModuleUnit::FunctionInstruction& inst, 
                          std::unordered_map<STDString, HazeValue>& constant_map);
    bool canEvaluateConstant(const ModuleUnit::FunctionInstruction& inst);
    
    // 循环优化
    void loopOptimization(ModuleUnit& module);
    void loopOptimizationFunction(ModuleUnit::FunctionTableData& function);
    
    // 循环信息
    struct LoopInfo
    {
        size_t header;  // 循环头
        std::unordered_set<size_t> body;  // 循环体
    };
    
    std::vector<LoopInfo> identifyLoops(ModuleUnit::FunctionTableData& function, const ControlFlowGraph& cfg);
    void dfsFindLoops(size_t block_idx, ModuleUnit::FunctionTableData& function, const ControlFlowGraph& cfg,
                      std::vector<bool>& visited, std::vector<bool>& in_stack,
                      std::vector<size_t>& dfs_stack, std::vector<LoopInfo>& loops);
    void hoistInvariantCode(ModuleUnit::FunctionTableData& function, const LoopInfo& loop, const ControlFlowGraph& cfg);
    bool isInvariantInstruction(const ModuleUnit::FunctionInstruction& inst, const LoopInfo& loop, const ControlFlowGraph& cfg, const ModuleUnit::FunctionTableData& function);
    bool isVariableModifiedInLoop(const STDString& var_name, const LoopInfo& loop, const ControlFlowGraph& cfg, const ModuleUnit::FunctionTableData& function);
    bool isVariableDefined(const ModuleUnit::FunctionInstruction& inst, const STDString& var_name);
    bool shouldUnrollLoop(const LoopInfo& loop, const ControlFlowGraph& cfg);
    bool hasPredictableIterations(const LoopInfo& loop);
    void unrollLoop(ModuleUnit::FunctionTableData& function, const LoopInfo& loop, const ControlFlowGraph& cfg);
    
    // 强度削减
    void strengthReduction(ModuleUnit& module);
    void strengthReductionFunction(ModuleUnit::FunctionTableData& function);
    bool canConvertToAddition(const ModuleUnit::FunctionInstruction& inst);
    void convertToAddition(ModuleUnit::FunctionTableData& function, size_t inst_idx);
    
    // 函数内联
    void functionInlining(ModuleUnit& module);
    void functionInliningFunction(ModuleUnit::FunctionTableData& function, const ModuleUnit& module);
    bool shouldInlineFunction(const STDString& function_name, const ModuleUnit& module);
    bool isRecursiveFunction(const ModuleUnit::FunctionTableData& function);
    bool hasComplexControlFlow(const ModuleUnit::FunctionTableData& function);
    void inlineFunctionCall(ModuleUnit::FunctionTableData& function, size_t call_site, const STDString& callee_name, const ModuleUnit& module);
    void renameLocalVariables(ModuleUnit::FunctionInstruction& inst, const STDString& caller_name);
    bool isLocalVariable(const STDString& var_name);
    
    // 公共子表达式消除
    void commonSubexpressionElimination(ModuleUnit& module);
    void commonSubexpressionEliminationFunction(ModuleUnit::FunctionTableData& function);
    STDString generateExpressionKey(const ModuleUnit::FunctionInstruction& inst);
    bool areOperandsAvailable(const ModuleUnit::FunctionInstruction& prev_inst, size_t current_inst_idx);
    
    // 寄存器分配
    void registerAllocation(ModuleUnit& module);
    void registerAllocationFunction(ModuleUnit::FunctionTableData& function);
    
    // 干扰图
    using InterferenceGraph = std::unordered_map<STDString, std::unordered_set<STDString>>;
    
    void buildInterferenceGraph(ModuleUnit::FunctionTableData& function, const ControlFlowGraph& cfg,
                               InterferenceGraph& interference_graph);
    bool colorGraph(const InterferenceGraph& interference_graph,
                   std::unordered_map<STDString, int>& register_assignment);
    void applyRegisterAllocation(ModuleUnit::FunctionTableData& function,
                                const std::unordered_map<STDString, int>& register_assignment);
    
    // 指令调度
    void instructionScheduling(ModuleUnit& module);
    void instructionSchedulingFunction(ModuleUnit::FunctionTableData& function);
    void buildInstructionDependencies(ModuleUnit::FunctionTableData& function, 
                                     std::vector<std::vector<size_t>>& dependencies);
    bool hasDependency(const ModuleUnit::FunctionInstruction& inst1, const ModuleUnit::FunctionInstruction& inst2);
    bool topologicalSort(const std::vector<std::vector<size_t>>& dependencies,
                        std::vector<size_t>& schedule);
    void applyInstructionSchedule(ModuleUnit::FunctionTableData& function, const std::vector<size_t>& schedule);

private:
    BackendParse* m_backend_parse;
    OptimizerConfig m_config;
    OptimizationStats m_stats;
    
    // 缓存
    std::unordered_map<STDString, HazeValue> m_constant_cache;
    std::unordered_map<STDString, ModuleUnit::FunctionTableData*> m_function_cache;
    
    // 分析结果
    std::unordered_map<size_t, InstructionAnalysis> m_instruction_analysis;
    
    // 当前分析状态
    ModuleUnit::FunctionTableData* m_current_function = nullptr;
    ControlFlowGraph m_cfg;
    std::unordered_map<STDString, std::shared_ptr<ModuleUnit>> m_modules;
};