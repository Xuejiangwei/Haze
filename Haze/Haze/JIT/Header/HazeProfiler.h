#pragma once

#include "HazeDefine.h"
#include <unordered_map>
#include <chrono>
#include <vector>

// 性能分析数据
struct ProfileData
{
    int call_count;                    // 调用次数
    double total_time;                 // 总执行时间
    double avg_time;                   // 平均执行时间
    double min_time;                   // 最小执行时间
    double max_time;                   // 最大执行时间
    
    // 热点检测
    bool is_hot;                       // 是否为热点
    double hot_threshold;              // 热点阈值
    
    // 调用栈信息
    std::vector<HString> call_stack;   // 调用栈
};

// 性能分析器
class HazeProfiler
{
public:
    HazeProfiler();
    ~HazeProfiler();
    
    // 开始性能分析
    void startProfile(const HString& function_name);
    
    // 结束性能分析
    void endProfile(const HString& function_name);
    
    // 获取性能数据
    ProfileData* getProfileData(const HString& function_name);
    
    // 检测热点函数
    std::vector<HString> getHotFunctions();
    
    // 生成性能报告
    void generateReport();
    
    // 设置热点阈值
    void setHotThreshold(double threshold);
    
private:
    std::unordered_map<HString, ProfileData> m_profile_data;
    std::unordered_map<HString, std::chrono::high_resolution_clock::time_point> m_start_times;
    std::vector<HString> m_call_stack;
    
    double m_hot_threshold;  // 热点阈值（毫秒）
    
    // 热点检测算法
    void updateHotStatus(ProfileData& data);
    
    // 性能统计
    void updateStatistics(ProfileData& data, double execution_time);
}; 