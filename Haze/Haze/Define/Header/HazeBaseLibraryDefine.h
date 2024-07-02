#pragma once

#define HAZE_BASE_LIBRARY_STREAM_NAME HAZE_TEXT("标准流")
#define HAZE_BASE_LIBRARY_STREAM_CODE HAZE_TEXT("标准库 标准流\n\
{\n\
	函数\n\
	{\n\
		空 打印(字符* 甲, ...)\n\
		空 输入(字符* 甲, ...)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_MEMORY_NAME HAZE_TEXT("标准内存")
#define HAZE_BASE_LIBRARY_MEMORY_CODE HAZE_TEXT("标准库 标准内存\n\
{\n\
	函数\n\
	{\n\
		空 内存复制(空* 目标地址, 空* 源地址, 长整数 字节大小)\n\
		空 多对象构造(空* 首地址, 空* 构造函数地址, 长整数 对象大小, 长整数 对象个数)\n\
		长整数 获得字符个数(空* 字符首地址)\n\
	}\n\
}")