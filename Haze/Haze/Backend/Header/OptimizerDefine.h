#pragma once

enum class BackendOptimizationLevel : x_uint32
{
    None,				// 无优化
    Base,				// 基础优化
    Radical,			// 激进优化
};

// 优化器配置
struct OptimizerConfig
{
    // 优化级别
    BackendOptimizationLevel optimizationLevel = BackendOptimizationLevel::Radical;

    // 常量折叠
    bool ConstantFold = true;

    // 死代码消除
    bool DeadCodeElimination = true;

    // 循环优化
    bool LoopOptimization = true;

    // 寄存器分配
    bool RegisterAllocation = true;

    // 函数内联
    bool FunctionInline = true;

    // 窥孔优化
    bool PeeholeOptimization = true;

    // 内联阈值
    size_t max_inline_size = 50;  // 最大内联函数指令数
    size_t max_inline_depth = 3;  // 最大内联深度

    // 基础优化配置
    static OptimizerConfig GetBasicOptimizationConfig()
    {
        OptimizerConfig config;
        config.optimizationLevel = BackendOptimizationLevel::Base;
        config.ConstantFold = true;
        config.DeadCodeElimination = true;
        config.PeeholeOptimization = true;
        config.RegisterAllocation = false;
        config.FunctionInline = false;
        config.LoopOptimization = false;
        return config;
    }

    // 激进优化配置
    static OptimizerConfig GetAggressiveOptimizationConfig()
    {
        OptimizerConfig config;
        config.optimizationLevel = BackendOptimizationLevel::Radical;
        config.ConstantFold = true;
        config.DeadCodeElimination = true;
        config.PeeholeOptimization = true;
        config.RegisterAllocation = true;
        config.FunctionInline = true;
        config.LoopOptimization = true;
        config.max_inline_size = 30;  // 更保守的内联阈值
        config.max_inline_depth = 2;  // 限制内联深度
        return config;
    }

    // 调试优化配置
    static OptimizerConfig GetDebugOptimizationConfig()
    {
        OptimizerConfig config;
        config.optimizationLevel = BackendOptimizationLevel::None;  // 无优化
        config.ConstantFold = false;
        config.DeadCodeElimination = false;
        config.PeeholeOptimization = false;
        config.RegisterAllocation = false;
        config.FunctionInline = false;
        config.LoopOptimization = false;
        return config;
    }

    // 性能优化配置
    static OptimizerConfig GetPerformanceOptimizationConfig()
    {
        OptimizerConfig config;
        config.optimizationLevel = BackendOptimizationLevel::Radical;
        config.ConstantFold = true;
        config.DeadCodeElimination = true;
        config.PeeholeOptimization = true;
        config.RegisterAllocation = true;
        config.FunctionInline = true;
        config.LoopOptimization = true;
        config.max_inline_size = 100;  // 更激进的内联
        config.max_inline_depth = 5;   // 更深的内联
        return config;
    }

};