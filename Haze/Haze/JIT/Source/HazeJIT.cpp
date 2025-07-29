#include "HazePch.h"
#include "HazeJIT.h"
#include "HazeLog.h"
#include <algorithm>
#include <chrono>

HazeJIT::HazeJIT(HazeVM* vm) 
    : m_VM(vm), m_optimization_level(JITOptimizationLevel::Basic)
{
    HAZE_LOG_INFO_W("JIT编译器初始化完成\\n");
}

HazeJIT::~HazeJIT()
{
    // 清理编译单元
    for (auto& pair : m_compiled_units)
    {
        delete pair.second;
    }
}

JITCompilationUnit* HazeJIT::compileFunction(const HString& function_name)
{
    // 检查是否已编译
    auto it = m_compiled_units.find(function_name);
    if (it != m_compiled_units.end())
    {
        return it->second;
    }

    // 获取函数字节码
    auto function = m_VM->GetFunctionDataByName(function_name);
    if (!function)
    {
        HAZE_LOG_ERR_W("函数 %s 不存在\\n", function_name.c_str());
        return nullptr;
    }

    // 创建编译单元
    JITCompilationUnit* unit = new JITCompilationUnit();

    auto insBegin = m_VM->GetInstruction().begin();
    unit->bytecode.clear();
    unit->bytecode.insert(unit->bytecode.end(), insBegin + function->FunctionDescData.InstructionStartAddress, insBegin + function->FunctionDescData.InstructionStartAddress + function->InstructionNum);
    unit->is_hot_path = isHotPath(function_name);
    unit->execution_count = m_execution_counts[function_name];
    unit->execution_time = m_execution_times[function_name];
    
    // 根据优化级别执行优化
    switch (m_optimization_level)
    {
        case JITOptimizationLevel::None:
            break;
        case JITOptimizationLevel::Basic:
            basicOptimization(unit);
            break;
        case JITOptimizationLevel::Aggressive:
            aggressiveOptimization(unit);
            break;
        case JITOptimizationLevel::ProfileGuided:
            profileGuidedOptimization(unit);
            break;
    }
    
    // 生成机器码
    generateNativeCode(unit);
    
    // 缓存编译单元
    m_compiled_units[function_name] = unit;
    
    HAZE_LOG_INFO_W("JIT编译函数 %s 完成，代码大小: %zu 字节\\n", function_name.c_str(), unit->size);
    
    return unit;
}

void HazeJIT::basicOptimization(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行基础JIT优化\\n");
    
    // JIT特有的优化：运行时常量折叠
    // 只处理后端优化无法处理的运行时常量
    for (auto& inst : unit->bytecode) {
        if (inst.InsCode == InstructionOpCode::ADD && 
            inst.Operator[1].Desc == HazeDataDesc::Constant &&
            inst.Operator[2].Desc == HazeDataDesc::Constant) {
            
            // 检查是否为运行时确定的常量（后端优化时未知）
            if (isRuntimeConstant(inst.Operator[1]) || isRuntimeConstant(inst.Operator[2])) {
                // 编译时计算常量加法
                int val1 = StringToStandardType<int>(inst.Operator[1].Variable.Name);
                int val2 = StringToStandardType<int>(inst.Operator[2].Variable.Name);
                int result = val1 + val2;
                
                // 替换为MOV指令
                inst.InsCode = InstructionOpCode::MOV;
                inst.Operator[1].Variable.Name = ToHazeString(result);
                inst.Operator[1].Desc = HazeDataDesc::Constant;
                inst.Operator[2] = InstructionData(); // 清空第二个操作数
            }
        }
    }
    
    // JIT特有的优化：基于执行频率的死代码消除
    // 只处理运行时才知道的死代码
    if (unit->execution_count > 100) {  // 热点函数才进行死代码消除
        eliminateRuntimeDeadCode(unit);
    }
}

void HazeJIT::aggressiveOptimization(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行激进JIT优化\\n");
    
    // 1. 基于执行统计的循环优化
    if (unit->execution_count > 1000) {
        optimizeLoopsWithProfile(unit);
    }
    
    // 2. 热点函数内联
    if (unit->is_hot_path) {
        inlineHotFunctions(unit);
    }
    
    // 3. 基于实际执行路径的寄存器分配
    optimizeRegisterAllocationWithProfile(unit);
    
    // 4. 动态类型特化
    specializeDynamicTypes(unit);
}

void HazeJIT::profileGuidedOptimization(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行基于性能分析的JIT优化\\n");
    
    // 1. 热点路径优化（后端优化无法做到）
    if (unit->is_hot_path) {
        optimizeHotPath(unit);
    }
    
    // 2. 基于实际分支概率的分支预测优化
    optimizeBranchPredictionWithProfile(unit);
    
    // 3. 缓存友好的代码布局
    optimizeCodeLayout(unit);
    
    // 4. 动态去虚拟化
    devirtualizeCalls(unit);
}

void HazeJIT::optimizeLoops(JITCompilationUnit* unit)
{
    // 识别循环结构
    std::vector<std::pair<size_t, size_t>> loops;
    
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        
        if (inst.InsCode == InstructionOpCode::JMP) {
            // 查找循环
            if (inst.Operator.size() >= 1) {
                HString target_name = inst.Operator[0].Variable.Name;
                size_t target = findLabelIndex(unit, target_name);
                if (target < i) {
                    loops.push_back({target, i});
                }
            }
        }
    }
    
    // 对每个循环进行优化
    for (auto& loop : loops) {
        // 1. 循环不变代码外提
        hoistInvariantCode(unit, loop.first, loop.second);
        
        // 2. 循环展开
        if (unit->execution_count > 1000) {
            unrollLoop(unit, loop.first, loop.second, 1);
        }
    }
}

void HazeJIT::optimizeHotPath(JITCompilationUnit* unit)
{
    // 热点路径特殊优化
    // 1. 更激进的指令重排序
    // 2. 更多的寄存器分配
    // 3. 更少的边界检查
}

void HazeJIT::generateNativeCode(JITCompilationUnit* unit)
{
    // 简化的x86-64代码生成示例
    unit->native_code.clear();
    
    for (const auto& inst : unit->bytecode) {
        generateInstructionCode(inst, unit->native_code);
    }
    
    unit->size = unit->native_code.size();
    unit->entry_point = 0;
}

void HazeJIT::generateInstructionCode(const Instruction& inst, std::vector<uint8_t>& code)
{
    // 简化的指令到机器码转换
    switch (inst.InsCode) {
        case InstructionOpCode::MOV:
            // MOV指令: 0x89 + 操作数
            code.push_back(0x89);
            // 这里需要更复杂的操作数编码
            break;
            
        case InstructionOpCode::ADD:
            // ADD指令: 0x01 + 操作数
            code.push_back(0x01);
            break;
            
        case InstructionOpCode::JMP:
            // JMP指令: 0xE9 + 偏移量
            code.push_back(0xE9);
            // 需要计算跳转偏移量
            break;
            
        default:
            // 其他指令的代码生成
            break;
    }
}

void HazeJIT::executeNativeCode(JITCompilationUnit* unit, HazeStack* stack)
{
    if (unit && !unit->native_code.empty()) {
        HAZE_LOG_INFO_W("执行JIT编译的本地代码\\n");
        
        // 这里应该执行生成的机器码
        // 简化实现：回退到字节码执行
        executeBytecode(unit, stack);
    } else {
        HAZE_LOG_ERR_W("JIT编译单元无效或未生成机器码\\n");
    }
}

void HazeJIT::executeBytecode(JITCompilationUnit* unit, HazeStack* stack)
{
    // 执行字节码（简化实现）
    for (const auto& inst : unit->bytecode) {
        // 调用VM的指令处理器
        if (m_VM && stack) {
            // 这里应该调用VM的指令执行器
            // 简化实现：记录执行
            HAZE_LOG_INFO_W("执行指令: %d\\n", (int)inst.InsCode);
        }
    }
}

bool HazeJIT::isHotPath(const HString& function_name)
{
    // 检查是否为热点路径
    auto it = m_execution_counts.find(function_name);
    if (it != m_execution_counts.end()) {
        return it->second > 1000; // 执行超过1000次认为是热点
    }
    
    auto time_it = m_execution_times.find(function_name);
    if (time_it != m_execution_times.end()) {
        return time_it->second > 1.0; // 执行时间超过1秒认为是热点
    }
    
    return false;
}

void HazeJIT::updateProfile(const HString& function_name, double execution_time)
{
    m_execution_counts[function_name]++;
    m_execution_times[function_name] += execution_time;
}

void HazeJIT::setOptimizationLevel(JITOptimizationLevel level)
{
    m_optimization_level = level;
    HAZE_LOG_INFO_W("JIT优化级别设置为: %d\\n", (int)level);
} 

// 新增：检查是否为运行时常量
bool HazeJIT::isRuntimeConstant(const InstructionData& operand)
{
    // 检查是否为运行时才知道的常量
    // 例如：配置值、环境变量、用户输入等
    if (operand.Desc == HazeDataDesc::Constant) {
        // 检查常量名称是否包含运行时信息
        HString const_name = operand.Variable.Name;
        return const_name.find(H_TEXT("runtime_")) != HString::npos ||
               const_name.find(H_TEXT("config_")) != HString::npos ||
               const_name.find(H_TEXT("env_")) != HString::npos;
    }
    return false;
}

// 新增：基于执行频率的死代码消除
void HazeJIT::eliminateRuntimeDeadCode(JITCompilationUnit* unit)
{
    // 只处理运行时才知道的死代码
    // 例如：基于用户配置的条件分支
    std::vector<bool> is_dead(unit->bytecode.size(), false);
    
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        
        // 检查是否为基于运行时条件的死代码
        if (isRuntimeConditionalDeadCode(inst, i, unit)) {
            is_dead[i] = true;
        }
    }
    
    // 移除死代码
    std::vector<Instruction> optimized_code;
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        if (!is_dead[i]) {
            optimized_code.push_back(unit->bytecode[i]);
        }
    }
    unit->bytecode = optimized_code;
}

// 新增：检查是否为运行时条件死代码
bool HazeJIT::isRuntimeConditionalDeadCode(const Instruction& inst, size_t index, JITCompilationUnit* unit)
{
    // 检查是否为基于运行时条件的死代码
    // 例如：调试模式检查、特性开关等
    
    if (inst.InsCode == InstructionOpCode::CMP) {
        // 检查是否为运行时配置比较
        if (inst.Operator.size() >= 2) {
            auto& op1 = inst.Operator[0];
            auto& op2 = inst.Operator[1];
            
            // 检查是否为配置值比较
            if (isConfigValue(op1) || isConfigValue(op2)) {
                // 基于实际配置值判断是否为死代码
                return isConfigBasedDeadCode(inst, index, unit);
            }
        }
    }
    
    return false;
}

// 新增：检查是否为配置值
bool HazeJIT::isConfigValue(const InstructionData& operand)
{
    if (operand.Desc == HazeDataDesc::Constant) {
        HString const_name = operand.Variable.Name;
        return const_name.find(H_TEXT("debug_")) != HString::npos ||
               const_name.find(H_TEXT("feature_")) != HString::npos ||
               const_name.find(H_TEXT("mode_")) != HString::npos;
    }
    return false;
}

// 新增：基于配置的死代码检查
bool HazeJIT::isConfigBasedDeadCode(const Instruction& inst, size_t index, JITCompilationUnit* unit)
{
    // 简化实现：检查一些常见的配置模式
    // 实际实现需要访问运行时配置
    
    // 例如：如果debug模式关闭，相关的调试代码就是死代码
    if (index + 1 < unit->bytecode.size()) {
        auto& next_inst = unit->bytecode[index + 1];
        if (IsJmpOpCode(next_inst.InsCode)) {
            // 检查是否为调试相关的跳转
            return isDebugRelatedJump(next_inst);
        }
    }
    
    return false;
}

// 新增：检查是否为调试相关跳转
bool HazeJIT::isDebugRelatedJump(const Instruction& inst)
{
    // 检查跳转目标是否为调试相关代码
    if (inst.Operator.size() >= 1) {
        HString target = inst.Operator[0].Variable.Name;
        return target.find(H_TEXT("debug_")) != HString::npos ||
               target.find(H_TEXT("log_")) != HString::npos ||
               target.find(H_TEXT("assert_")) != HString::npos;
    }
    return false;
} 

// 基于性能分析的优化函数实现

void HazeJIT::optimizeLoopsWithProfile(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行基于性能分析的循环优化\\n");
    
    // 识别循环结构
    std::vector<std::pair<size_t, size_t>> loops;
    
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        
        if (inst.InsCode == InstructionOpCode::JMP) {
            // 查找循环
            if (inst.Operator.size() >= 1) {
                HString target_name = inst.Operator[0].Variable.Name;
                size_t target = findLabelIndex(unit, target_name);
                if (target < i) {
                    loops.push_back({target, i});
                }
            }
        }
    }
    
    // 对每个循环进行基于性能分析的优化
    for (auto& loop : loops) {
        size_t loop_start = loop.first;
        size_t loop_end = loop.second;
        
        // 计算循环执行频率
        int loop_execution_count = calculateLoopExecutionCount(unit, loop_start, loop_end);
        
        if (loop_execution_count > 1000) {
            // 高频循环：进行激进优化
            aggressiveLoopOptimization(unit, loop_start, loop_end);
        } else if (loop_execution_count > 100) {
            // 中频循环：进行适度优化
            moderateLoopOptimization(unit, loop_start, loop_end);
        }
    }
}

void HazeJIT::inlineHotFunctions(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行热点函数内联\\n");
    
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        
        if (inst.InsCode == InstructionOpCode::CALL) {
            if (inst.Operator.size() >= 1) {
                HString callee_name = inst.Operator[0].Variable.Name;
                
                // 检查是否为热点函数
                if (isHotPath(callee_name)) {
                    // 检查函数大小是否适合内联
                    auto callee_unit = m_compiled_units.find(callee_name);
                    if (callee_unit != m_compiled_units.end()) {
                        if (callee_unit->second->bytecode.size() <= 20) { // 小函数内联
                            inlineFunction(unit, i, callee_unit->second);
                        }
                    }
                }
            }
        }
    }
}

void HazeJIT::optimizeRegisterAllocationWithProfile(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行基于性能分析的寄存器分配优化\\n");
    
    // 分析变量使用频率
    std::unordered_map<HString, int> variable_usage_count;
    
    for (const auto& inst : unit->bytecode) {
        // 统计变量使用次数
        for (const auto& operand : inst.Operator) {
            if (operand.Desc != HazeDataDesc::Constant && 
                operand.Desc != HazeDataDesc::ConstantString &&
                operand.Desc != HazeDataDesc::NullPtr) {
                variable_usage_count[operand.Variable.Name]++;
            }
        }
    }
    
    // 为高频变量分配寄存器
    std::vector<std::pair<HString, int>> sorted_variables;
    for (const auto& pair : variable_usage_count) {
        sorted_variables.push_back(pair);
    }
    
    // 按使用频率排序
    std::sort(sorted_variables.begin(), sorted_variables.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });
    
    // 分配寄存器（简化实现）
    std::unordered_map<HString, int> register_assignment;
    int available_registers = 8; // 假设有8个可用寄存器
    
    for (size_t i = 0; i < sorted_variables.size() && i < available_registers; ++i) {
        register_assignment[sorted_variables[i].first] = i;
    }
    
    // 应用寄存器分配
    applyRegisterAllocation(unit, register_assignment);
}

void HazeJIT::specializeDynamicTypes(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行动态类型特化\\n");
    
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        
        // 检查是否为动态类型操作
        if (isDynamicTypeOperation(inst)) {
            // 基于运行时类型信息生成特化代码
            specializeInstruction(unit, i, inst);
        }
    }
}

void HazeJIT::optimizeBranchPredictionWithProfile(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行基于性能分析的分支预测优化\\n");
    
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        
        if (IsJmpOpCode(inst.InsCode) && inst.InsCode != InstructionOpCode::JMP) {
            // 条件跳转
            double branch_probability = calculateBranchProbability(unit, i);
            
            if (branch_probability > 0.9) {
                // 几乎总是执行的分支，优化为无条件跳转
                optimizeFrequentBranch(unit, i, true);
            } else if (branch_probability < 0.1) {
                // 几乎从不执行的分支，优化为死代码
                optimizeFrequentBranch(unit, i, false);
            }
        }
    }
}

void HazeJIT::devirtualizeCalls(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行动态去虚拟化\\n");
    
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        
        if (inst.InsCode == InstructionOpCode::CALL) {
            // 检查是否为虚函数调用
            if (isVirtualFunctionCall(inst)) {
                // 尝试去虚拟化
                if (canDevirtualize(inst, i, unit)) {
                    devirtualizeCall(unit, i, inst);
                }
            }
        }
    }
}

// 辅助函数实现

size_t HazeJIT::findLabelIndex(JITCompilationUnit* unit, const HString& label_name)
{
    for (size_t i = 0; i < unit->bytecode.size(); ++i) {
        auto& inst = unit->bytecode[i];
        if (inst.InsCode == InstructionOpCode::LINE) {
            if (i + 1 < unit->bytecode.size()) {
                auto& next_inst = unit->bytecode[i + 1];
                if (next_inst.Operator.size() > 0 && 
                    next_inst.Operator[0].Variable.Name == label_name) {
                    return i + 1;
                }
            }
        }
    }
    return unit->bytecode.size(); // 未找到
}

int HazeJIT::calculateLoopExecutionCount(JITCompilationUnit* unit, size_t loop_start, size_t loop_end)
{
    // 简化实现：基于函数执行次数估算循环执行次数
    return unit->execution_count * 10; // 假设平均每次函数调用循环执行10次
}

void HazeJIT::aggressiveLoopOptimization(JITCompilationUnit* unit, size_t loop_start, size_t loop_end)
{
    // 激进循环优化
    // 1. 循环展开
    unrollLoop(unit, loop_start, loop_end, 4); // 展开4次
    
    // 2. 循环不变代码外提
    hoistInvariantCode(unit, loop_start, loop_end);
    
    // 3. 强度削弱
    strengthReduction(unit, loop_start, loop_end);
}

void HazeJIT::moderateLoopOptimization(JITCompilationUnit* unit, size_t loop_start, size_t loop_end)
{
    // 适度循环优化
    // 1. 循环不变代码外提
    hoistInvariantCode(unit, loop_start, loop_end);
    
    // 2. 基本强度削弱
    basicStrengthReduction(unit, loop_start, loop_end);
}

void HazeJIT::inlineFunction(JITCompilationUnit* unit, size_t call_site, JITCompilationUnit* callee_unit)
{
    // 函数内联实现
    std::vector<Instruction> new_bytecode;
    
    // 复制调用点之前的指令
    for (size_t i = 0; i < call_site; ++i) {
        new_bytecode.push_back(unit->bytecode[i]);
    }
    
    // 内联被调用函数的指令
    for (const auto& inst : callee_unit->bytecode) {
        new_bytecode.push_back(inst);
    }
    
    // 复制调用点之后的指令
    for (size_t i = call_site + 1; i < unit->bytecode.size(); ++i) {
        new_bytecode.push_back(unit->bytecode[i]);
    }
    
    unit->bytecode = new_bytecode;
}

void HazeJIT::applyRegisterAllocation(JITCompilationUnit* unit, const std::unordered_map<HString, int>& register_assignment)
{
    // 应用寄存器分配
    for (auto& inst : unit->bytecode) {
        for (auto& operand : inst.Operator) {
            if (operand.Desc != HazeDataDesc::Constant && 
                operand.Desc != HazeDataDesc::ConstantString &&
                operand.Desc != HazeDataDesc::NullPtr) {
                
                auto it = register_assignment.find(operand.Variable.Name);
                if (it != register_assignment.end()) {
                    // 替换为寄存器引用
                    operand.Desc = HazeDataDesc::RegisterTemp;
                    operand.Variable.Name = H_TEXT("R") + ToHazeString(it->second);
                }
            }
        }
    }
}

bool HazeJIT::isDynamicTypeOperation(const Instruction& inst)
{
    // 检查是否为动态类型操作
    switch (inst.InsCode) {
    case InstructionOpCode::CALL:
        // 函数调用可能是动态类型
        return true;
    case InstructionOpCode::MOV:
        // 某些MOV操作可能涉及动态类型
        return inst.Operator.size() >= 2 && 
               inst.Operator[1].Variable.Type.BaseType == HazeValueType::ObjectFunction;
    default:
        return false;
    }
}

void HazeJIT::specializeInstruction(JITCompilationUnit* unit, size_t index, const Instruction& inst)
{
    // 指令特化实现
    // 这里简化实现，实际需要根据运行时类型信息生成特化代码
    
    if (inst.InsCode == InstructionOpCode::CALL) {
        // 函数调用特化
        HString function_name = inst.Operator[0].Variable.Name;
        
        // 检查是否为已知的特定类型函数
        if (function_name.find(H_TEXT("string_")) != HString::npos) {
            // 字符串操作特化
            specializeStringOperation(unit, index, inst);
        } else if (function_name.find(H_TEXT("array_")) != HString::npos) {
            // 数组操作特化
            specializeArrayOperation(unit, index, inst);
        }
    }
}

void HazeJIT::specializeStringOperation(JITCompilationUnit* unit, size_t index, const Instruction& inst)
{
    // 字符串操作特化
    // 生成针对字符串类型的优化代码
    HAZE_LOG_INFO_W("特化字符串操作\\n");
}

void HazeJIT::specializeArrayOperation(JITCompilationUnit* unit, size_t index, const Instruction& inst)
{
    // 数组操作特化
    // 生成针对数组类型的优化代码
    HAZE_LOG_INFO_W("特化数组操作\\n");
}

double HazeJIT::calculateBranchProbability(JITCompilationUnit* unit, size_t branch_index)
{
    // 计算分支概率
    // 简化实现：基于执行次数估算
    return 0.5; // 默认50%概率
}

void HazeJIT::optimizeFrequentBranch(JITCompilationUnit* unit, size_t branch_index, bool is_frequent)
{
    // 优化频繁分支
    auto& inst = unit->bytecode[branch_index];
    
    if (is_frequent) {
        // 频繁分支：转换为无条件跳转
        inst.InsCode = InstructionOpCode::JMP;
    } else {
        // 不频繁分支：标记为死代码
        // 这里简化处理，实际应该删除相关代码块
        HAZE_LOG_INFO_W("标记不频繁分支为死代码\\n");
    }
}

bool HazeJIT::isVirtualFunctionCall(const Instruction& inst)
{
    // 检查是否为虚函数调用
    if (inst.InsCode == InstructionOpCode::CALL) {
        HString function_name = inst.Operator[0].Variable.Name;
        return function_name.find(H_TEXT("virtual_")) != HString::npos ||
               function_name.find(H_TEXT("polymorphic_")) != HString::npos;
    }
    return false;
}

bool HazeJIT::canDevirtualize(const Instruction& inst, size_t index, JITCompilationUnit* unit)
{
    // 检查是否可以去虚拟化
    // 简化实现：检查是否为单态调用
    return true; // 假设都可以去虚拟化
}

void HazeJIT::devirtualizeCall(JITCompilationUnit* unit, size_t index, const Instruction& inst)
{
    // 去虚拟化实现
    HAZE_LOG_INFO_W("去虚拟化函数调用\\n");
    
    // 将虚函数调用替换为直接函数调用
    auto& call_inst = unit->bytecode[index];
    HString virtual_function = call_inst.Operator[0].Variable.Name;
    
    // 简化的去虚拟化：替换为直接调用
    HString direct_function = virtual_function + H_TEXT("_direct");
    call_inst.Operator[0].Variable.Name = direct_function;
}

void HazeJIT::unrollLoop(JITCompilationUnit* unit, size_t loop_start, size_t loop_end, int unroll_factor)
{
    // 循环展开实现
    HAZE_LOG_INFO_W("循环展开，展开因子: %d\\n", unroll_factor);
    
    std::vector<Instruction> new_bytecode;
    
    // 复制循环前的指令
    for (size_t i = 0; i < loop_start; ++i) {
        new_bytecode.push_back(unit->bytecode[i]);
    }
    
    // 展开循环体
    for (int j = 0; j < unroll_factor; ++j) {
        for (size_t i = loop_start; i < loop_end; ++i) {
            new_bytecode.push_back(unit->bytecode[i]);
        }
    }
    
    // 复制循环后的指令
    for (size_t i = loop_end; i < unit->bytecode.size(); ++i) {
        new_bytecode.push_back(unit->bytecode[i]);
    }
    
    unit->bytecode = new_bytecode;
}

void HazeJIT::hoistInvariantCode(JITCompilationUnit* unit, size_t loop_start, size_t loop_end)
{
    // 循环不变代码外提
    HAZE_LOG_INFO_W("循环不变代码外提\\n");
    
    std::vector<Instruction> invariant_code;
    std::vector<Instruction> loop_code;
    
    // 分离不变代码和循环代码
    for (size_t i = loop_start; i < loop_end; ++i) {
        auto& inst = unit->bytecode[i];
        if (isInvariantInstruction(inst, loop_start, loop_end, unit)) {
            invariant_code.push_back(inst);
        } else {
            loop_code.push_back(inst);
        }
    }
    
    // 重新组织代码：不变代码在前，循环代码在后
    std::vector<Instruction> new_bytecode;
    
    // 复制循环前的指令
    for (size_t i = 0; i < loop_start; ++i) {
        new_bytecode.push_back(unit->bytecode[i]);
    }
    
    // 添加不变代码
    for (const auto& inst : invariant_code) {
        new_bytecode.push_back(inst);
    }
    
    // 添加循环代码
    for (const auto& inst : loop_code) {
        new_bytecode.push_back(inst);
    }
    
    // 复制循环后的指令
    for (size_t i = loop_end; i < unit->bytecode.size(); ++i) {
        new_bytecode.push_back(unit->bytecode[i]);
    }
    
    unit->bytecode = new_bytecode;
}

bool HazeJIT::isInvariantInstruction(const Instruction& inst, size_t loop_start, size_t loop_end, JITCompilationUnit* unit)
{
    // 检查指令是否为循环不变
    // 简化实现：检查是否只使用常量
    for (const auto& operand : inst.Operator) {
        if (operand.Desc != HazeDataDesc::Constant && 
            operand.Desc != HazeDataDesc::ConstantString &&
            operand.Desc != HazeDataDesc::NullPtr) {
            return false; // 使用了变量，不是不变指令
        }
    }
    return true;
}

void HazeJIT::strengthReduction(JITCompilationUnit* unit, size_t loop_start, size_t loop_end)
{
    // 强度削弱
    HAZE_LOG_INFO_W("强度削弱优化\\n");
    
    for (size_t i = loop_start; i < loop_end; ++i) {
        auto& inst = unit->bytecode[i];
        
        // 将乘法转换为加法
        if (inst.InsCode == InstructionOpCode::MUL) {
            if (inst.Operator.size() >= 3) {
                auto& op2 = inst.Operator[2];
                if (op2.Desc == HazeDataDesc::Constant) {
                    int constant = StringToStandardType<int>(op2.Variable.Name);
                    if (constant == 2) {
                        // x * 2 -> x + x
                        inst.InsCode = InstructionOpCode::ADD;
                        inst.Operator[2] = inst.Operator[1]; // 复制操作数
                    }
                }
            }
        }
    }
}

void HazeJIT::basicStrengthReduction(JITCompilationUnit* unit, size_t loop_start, size_t loop_end)
{
    // 基本强度削弱
    HAZE_LOG_INFO_W("基本强度削弱优化\\n");
    
    // 简化实现：只处理明显的强度削弱
    for (size_t i = loop_start; i < loop_end; ++i) {
        auto& inst = unit->bytecode[i];
        
        if (inst.InsCode == InstructionOpCode::MUL) {
            if (inst.Operator.size() >= 3) {
                auto& op2 = inst.Operator[2];
                if (op2.Desc == HazeDataDesc::Constant) {
                    int constant = StringToStandardType<int>(op2.Variable.Name);
                    if (constant == 1) {
                        // x * 1 -> x (删除指令)
                        inst.InsCode = InstructionOpCode::NONE;
                    }
                }
            }
        }
    }
} 

void HazeJIT::optimizeCodeLayout(JITCompilationUnit* unit)
{
    HAZE_LOG_INFO_W("执行缓存友好的代码布局优化\\n");
    
    // 1. 热点代码聚集
    clusterHotCode(unit);
    
    // 2. 冷代码分离
    separateColdCode(unit);
    
    // 3. 分支目标对齐
    alignBranchTargets(unit);
}

void HazeJIT::clusterHotCode(JITCompilationUnit* unit)
{
    // 将热点代码聚集在一起
    std::vector<Instruction> hot_code;
    std::vector<Instruction> cold_code;
    
    for (const auto& inst : unit->bytecode) {
        if (isHotInstruction(inst)) {
            hot_code.push_back(inst);
        } else {
            cold_code.push_back(inst);
        }
    }
    
    // 重新组织代码：热点代码在前
    unit->bytecode.clear();
    unit->bytecode.insert(unit->bytecode.end(), hot_code.begin(), hot_code.end());
    unit->bytecode.insert(unit->bytecode.end(), cold_code.begin(), cold_code.end());
}

void HazeJIT::separateColdCode(JITCompilationUnit* unit)
{
    // 将冷代码移到代码段末尾
    // 这里简化实现，实际需要更复杂的分析
    HAZE_LOG_INFO_W("分离冷代码\\n");
}

void HazeJIT::alignBranchTargets(JITCompilationUnit* unit)
{
    // 对齐分支目标以提高分支预测性能
    // 这里简化实现，实际需要插入NOP指令进行对齐
    HAZE_LOG_INFO_W("对齐分支目标\\n");
}

bool HazeJIT::isHotInstruction(const Instruction& inst)
{
    // 检查是否为热点指令
    // 简化实现：基于指令类型判断
    switch (inst.InsCode) {
    case InstructionOpCode::CALL:
    case InstructionOpCode::JMP:
    case InstructionOpCode::ADD:
    case InstructionOpCode::MOV:
        return true; // 这些指令通常是热点
    default:
        return false;
    }
} 