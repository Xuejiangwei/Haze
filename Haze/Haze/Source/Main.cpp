﻿#ifdef _WINDLL

#include "Haze.h"
//解析文本  --->  生成字节码   --->  用虚拟机解析字节码，并执行
int main(int ArgCount, char* ArgValue[])
{
	return HazeMain(ArgCount, ArgValue);
}

#endif // DEBUG
