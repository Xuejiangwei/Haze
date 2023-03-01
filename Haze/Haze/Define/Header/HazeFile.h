#pragma once

//字节码文件头部数据格式定义(模仿linux程序结构 堆区、栈区、全局数据区、只读数据区等)
enum HazeFileFormat
{
	None,
	GlobalData,
	GlobalFunction,
};