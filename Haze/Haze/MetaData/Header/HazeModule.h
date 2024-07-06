#pragma once

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
	HazeModule(const HString& opFile);

	~HazeModule();

private:
	void ParseOpFile(const HString& opFile);

private:

	HazeValue* AddGlobalVariable();

private:
	//HashMap<HString, HazeCompilerValue> MapGlobalVariables;
	//V_Array<HazeClass> Classes;
	//V_Array<HazeFunction> Functions;
};
