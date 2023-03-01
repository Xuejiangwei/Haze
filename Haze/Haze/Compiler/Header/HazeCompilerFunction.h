#pragma once

#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <fstream>

#include "Haze.h"
#include "HazeCompilerValue.h"
#include "HazeCompilerFunctionStack.h"

class HazeCompilerModule;
class HazeBaseBlock;

class HazeCompilerFunction
{
public:
	friend class HazeCompiler;
	friend class HazeCompilerModule;

	HazeCompilerFunction(HazeCompilerModule* Module, HAZE_STRING& Name, HazeDefineData& Type, std::vector<HazeDefineVariable>& Param);
	~HazeCompilerFunction();

	std::shared_ptr<HazeCompilerValue> GetLocalVariable(const HAZE_STRING& Name);

	const HAZE_STRING& GetName() const { return Name; }
	
	HazeCompilerModule* GetModule() const { return Module; }

	void FunctionFinish();

	void GenI_Code(HazeCompilerModule* Module, HAZE_OFSTREAM& OFStream);

	std::shared_ptr<HazeBaseBlock>& GetTopBaseBlock() { return BBList.back(); }

	std::list<std::shared_ptr<HazeBaseBlock>>& GetBaseBlockList() { return BBList; }

	bool GetLocalVariableName(std::shared_ptr<HazeCompilerValue>& Value, HAZE_STRING& NameOut);

private:
	void AddFunctionParam(const HazeDefineVariable& Variable);

	std::shared_ptr<HazeCompilerValue> CreateLocalVariable(const HazeDefineVariable& Variable);

private:
	HazeCompilerModule* Module;

	HAZE_STRING Name;
	HazeDefineData Type;

	std::vector<std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>> VectorParam; //从右到左加入参数

	HazeCompilerFunctionStack StackFrame;

	std::list<std::shared_ptr<HazeBaseBlock>> BBList;

};
