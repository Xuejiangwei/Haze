#include "HazeLog.h"

#include "HazeCompilerFunction.h"

HazeCompilerFunction::HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineType& Type, std::vector<HazeDefineVariable>& VectorParam)
	: Module(Module), Name(Name), Type(Type)
{
	for (auto it : VectorParam)
	{
	}
}

HazeCompilerFunction::~HazeCompilerFunction()
{
}

std::shared_ptr<HazeCompilerValue> HazeCompilerFunction::AddLocalVariable(const HazeDefineVariable& Variable)
{
	auto It = MapLocalVariable.find(Variable.second);
	if (It != MapLocalVariable.end())
	{
		HazeLog::LogInfo(HazeLog::Error, HAZE_TEXT("���������� ��Ӿֲ������ظ�"));
		return nullptr;
	}

	MapLocalVariable[Variable.second] = std::make_shared<HazeCompilerValue>(Module, Variable.first);

	return MapLocalVariable[Variable.second];
}

bool HazeCompilerFunction::GeneratorOpCode()
{
	//���ɺ���label

	//Push���в��������ҵ���
	for (auto& it : VectorParam)
	{
	}

	//Push���ص�ַ

	//Push��ʱ����

	return true;
}