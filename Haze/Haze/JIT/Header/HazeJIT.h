#pragma once

#include "HazeDefine.h"
#include "HazeVM.h"
#include "HazeInstruction.h"
#include <unordered_map>
#include <vector>
#include <memory>

// JIT编译单元
struct JITCompilationUnit
{
    std::vector<Instruction> bytecode;     // 字节码指令
    std::vector<uint8_t> native_code;      // 生成的机器码
    size_t entry_point;                    // 入口点偏移
    size_t size;                          // 代码大小
    
    // 优化信息
    bool is_hot_path;                     // 是否为热点路径
    int execution_count;                  // 执行次数
    double execution_time;                // 执行时间
};

// JIT优化级别
enum class JITOptimizationLevel
{
    None = 0,           // 无优化
    Basic = 1,          // 基础优化
    Aggressive = 2,     // 激进优化
    ProfileGuided = 3   // 基于性能分析的优化
};

// JIT编译器
class HazeJIT
{
public:
    HazeJIT(HazeVM* vm);
    ~HazeJIT();
    
    // 编译函数
    JITCompilationUnit* compileFunction(const HString& function_name);
    
    // 执行JIT编译的代码
    void executeNativeCode(JITCompilationUnit* unit, HazeStack* stack);
    
    // 执行字节码（回退方案）
    void executeBytecode(JITCompilationUnit* unit, HazeStack* stack);
    
    // 热点检测
    bool isHotPath(const HString& function_name);
    
    // 性能分析
    void updateProfile(const HString& function_name, double execution_time);
    
    // 优化配置
    void setOptimizationLevel(JITOptimizationLevel level);
    
private:
    HazeVM* m_VM;
    std::unordered_map<HString, JITCompilationUnit*> m_compiled_units;
    std::unordered_map<HString, int> m_execution_counts;
    std::unordered_map<HString, double> m_execution_times;
    
    JITOptimizationLevel m_optimization_level;
    
    // 编译优化方法
    void basicOptimization(JITCompilationUnit* unit);
    
    // 激进优化
    void aggressiveOptimization(JITCompilationUnit* unit);
    
    // 基于性能分析的优化
    void profileGuidedOptimization(JITCompilationUnit* unit);
    
    // 循环优化
    void optimizeLoops(JITCompilationUnit* unit);
    
    // 热点路径优化
    void optimizeHotPath(JITCompilationUnit* unit);
    
    // 新增：JIT特有的优化函数
    bool isRuntimeConstant(const InstructionData& operand);
    void eliminateRuntimeDeadCode(JITCompilationUnit* unit);
    bool isRuntimeConditionalDeadCode(const Instruction& inst, size_t index, JITCompilationUnit* unit);
    bool isConfigValue(const InstructionData& operand);
    bool isConfigBasedDeadCode(const Instruction& inst, size_t index, JITCompilationUnit* unit);
    bool isDebugRelatedJump(const Instruction& inst);
    
    // 基于性能分析的优化函数
    void optimizeLoopsWithProfile(JITCompilationUnit* unit);
    void inlineHotFunctions(JITCompilationUnit* unit);
    void optimizeRegisterAllocationWithProfile(JITCompilationUnit* unit);
    void specializeDynamicTypes(JITCompilationUnit* unit);
    void optimizeBranchPredictionWithProfile(JITCompilationUnit* unit);
    void devirtualizeCalls(JITCompilationUnit* unit);
    
    // 辅助函数
    size_t findLabelIndex(JITCompilationUnit* unit, const HString& label_name);
    int calculateLoopExecutionCount(JITCompilationUnit* unit, size_t loop_start, size_t loop_end);
    void aggressiveLoopOptimization(JITCompilationUnit* unit, size_t loop_start, size_t loop_end);
    void moderateLoopOptimization(JITCompilationUnit* unit, size_t loop_start, size_t loop_end);
    void inlineFunction(JITCompilationUnit* unit, size_t call_site, JITCompilationUnit* callee_unit);
    void applyRegisterAllocation(JITCompilationUnit* unit, const std::unordered_map<HString, int>& register_assignment);
    bool isDynamicTypeOperation(const Instruction& inst);
    void specializeInstruction(JITCompilationUnit* unit, size_t index, const Instruction& inst);
    void specializeStringOperation(JITCompilationUnit* unit, size_t index, const Instruction& inst);
    void specializeArrayOperation(JITCompilationUnit* unit, size_t index, const Instruction& inst);
    double calculateBranchProbability(JITCompilationUnit* unit, size_t branch_index);
    void optimizeFrequentBranch(JITCompilationUnit* unit, size_t branch_index, bool is_frequent);
    bool isVirtualFunctionCall(const Instruction& inst);
    bool canDevirtualize(const Instruction& inst, size_t index, JITCompilationUnit* unit);
    void devirtualizeCall(JITCompilationUnit* unit, size_t index, const Instruction& inst);
    void unrollLoop(JITCompilationUnit* unit, size_t loop_start, size_t loop_end, int unroll_factor);
    void hoistInvariantCode(JITCompilationUnit* unit, size_t loop_start, size_t loop_end);
    bool isInvariantInstruction(const Instruction& inst, size_t loop_start, size_t loop_end, JITCompilationUnit* unit);
    void strengthReduction(JITCompilationUnit* unit, size_t loop_start, size_t loop_end);
    void basicStrengthReduction(JITCompilationUnit* unit, size_t loop_start, size_t loop_end);
    
    // 代码布局优化
    void optimizeCodeLayout(JITCompilationUnit* unit);
    void clusterHotCode(JITCompilationUnit* unit);
    void separateColdCode(JITCompilationUnit* unit);
    void alignBranchTargets(JITCompilationUnit* unit);
    bool isHotInstruction(const Instruction& inst);
    
    // 代码生成
    void generateNativeCode(JITCompilationUnit* unit);
    void generateInstructionCode(const Instruction& inst, std::vector<uint8_t>& code);
}; 