#pragma once

#define HAZE_BASE_LIBRARY_STREAM_NAME H_TEXT("标准流")
#define HAZE_BASE_LIBRARY_STREAM_CODE H_TEXT("静态库 标准流\n\
{\n\
	函数\n\
	{\n\
		空 打印(字符 甲, ...)\n\
		空 输入(字符 甲, ...)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_MEMORY_NAME H_TEXT("标准内存")
#define HAZE_BASE_LIBRARY_MEMORY_CODE H_TEXT("静态库 标准内存\n\
{\n\
	函数\n\
	{\n\
		空 内存复制(正整数64 目标地址, 正整数64 源地址, 正整数64 字节数)\n\
		空 多对象构造(正整数64 首地址, 正整数64 构造函数地址, 正整数64 对象大小, 正整数64 对象个数)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_FILE_NAME H_TEXT("标准文件")
#define HAZE_BASE_LIBRARY_FILE_CODE H_TEXT("静态库 标准文件\n\
{\n\
	函数\n\
	{\n\
		正整数64 打开文件(正整数64 文件路径, 整数 操作方式)\n\
		空 关闭文件(正整数64 文件指针)\n\
		整数 读取字符(正整数64 文件指针)\n\
		正整数64 读取字符串(正整数64 文件指针, 整数 最大个数, 正整数64 单个字符串)\n\
		正整数64 读取一行(正整数64 文件指针, 正整数64 单个字符串)\n\
		正整数64 读取(正整数64 文件指针, 正整数64 字节数, 正整数64 个数, 正整数64 单个字符串)\n\
		整数 写入字符(正整数64 文件指针, 正整数 单个字符)\n\
		整数 写入字符串(正整数64 文件指针, 正整数64 单个字符串)\n\
		正长整数 写入(正整数64 文件指针, 正整数64 字节数, 正整数64 个数, 正整数64 单个字符串)\n\
	}\n\
}")

#define HAZE_BASE_LIBS { HAZE_BASE_LIBRARY_STREAM_NAME, HAZE_BASE_LIBRARY_STREAM_CODE, \
						 HAZE_BASE_LIBRARY_MEMORY_NAME, HAZE_BASE_LIBRARY_MEMORY_CODE, \
						 HAZE_BASE_LIBRARY_FILE_NAME, HAZE_BASE_LIBRARY_FILE_CODE \
 }