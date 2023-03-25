#pragma once

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <fstream>

#include "Haze.h"
#include "HazeCompilerValue.h"

class HazeCompilerModule;
class HazeCompilerClass;
class HazeBaseBlock;

class HazeCompilerFunction
{
public:
	friend class HazeCompiler;
	friend class HazeCompilerModule;

	HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param, HazeCompilerClass* Class = nullptr);
	~HazeCompilerFunction();

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& Name, const HAZE_STRING* MemberName = nullptr);

	const HAZE_STRING& GetName() const { return Name; }
	
	HazeCompilerModule* GetModule() const { return Module; }

	HazeCompilerClass* GetClass() const { return Class; }

	void FunctionFinish();

	void GenI_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream);

	std::shared_ptr<HazeBaseBlock>& GetTopBaseBlock() { return BBList.back(); }

	std::list<std::shared_ptr<HazeBaseBlock>>& GetBaseBlockList() { return BBList; }

	bool GetLocalVariableName(const std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& OutName);

	bool GetFunctionParamNameByIndex(unsigned int Index, HAZE_STRING& OutName);

private:
	void AddFunctionParam(const HazeDefineVariable& Variable);

	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(const HazeDefineVariable& Variable);

	void CreateNew(const HazeDefineData& Data);

private:
	HazeCompilerModule* Module;
	HazeCompilerClass* Class;

	HAZE_STRING Name;
	HazeDefineData Type;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorParam; //从右到左加入参数

	std::list<std::shared_ptr<HazeBaseBlock>> BBList;

};
