#pragma once

#include <unordered_map>

#include "Haze.h"

//class HazeClass;
//class HazeFunction;

/*
	Op File format

	Main Header:
	4 byte : 全局变量大小
	4 byte : main函数索引

	Op Stream:
	4 byte : 指令数量
	未知大小 ： 指令

	String Table:
	4 byte : 字符串个数
	未知大小 ： n个（ 4 byte（字符串长度） + 字符串）

	Function Table:
	4 byte : 函数个数
	未知大小： 函数

	Global Data Table:
	4 byte : 全局数据个数
	未知大小 ：数据
*/

class HazeModule
{
public:
	HazeModule(const HAZE_STRING& opFile);

	~HazeModule();

private:
	void ParseOpFile(const HAZE_STRING& opFile);

private:

	HazeValue* AddGlobalVariable();

private:
	//std::unordered_map<HAZE_STRING, HazeCompilerValue> MapGlobalVariables;
	//std::vector<HazeClass> Classes;
	//std::vector<HazeFunction> Functions;
};
