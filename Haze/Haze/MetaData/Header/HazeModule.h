#pragma once

#include <unordered_map>

#include "Haze.h"

//class HazeClass;
//class HazeFunction;

/*
	Op File format

	Main Header:
	4 byte : ȫ�ֱ�����С
	4 byte : main��������

	Op Stream:
	4 byte : ָ������
	δ֪��С �� ָ��

	String Table:
	4 byte : �ַ�������
	δ֪��С �� n���� 4 byte���ַ������ȣ� + �ַ�����

	Function Table:
	4 byte : ��������
	δ֪��С�� ����

	Global Data Table:
	4 byte : ȫ�����ݸ���
	δ֪��С ������
*/

class HazeModule
{
public:
	HazeModule(const HAZE_STRING& OpFile);
	~HazeModule();

private:
	void ParseOpFile(const HAZE_STRING& OpFile);

private:

	HazeValue* AddGlobalVariable();

private:
	//std::unordered_map<HAZE_STRING, HazeCompilerValue> MapGlobalVariables;
	//std::vector<HazeClass> Classes;
	//std::vector<HazeFunction> Functions;
};
