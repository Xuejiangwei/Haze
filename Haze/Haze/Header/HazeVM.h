#pragma once

#include <string>

class HazeVM
{
public:
	HazeVM();
	~HazeVM();

	void DoString(const std::wstring& String);

private:
	void Parse(const std::wstring& String);
};
