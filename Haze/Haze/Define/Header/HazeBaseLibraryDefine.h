#pragma once

#define HAZE_BASE_LIBRARY_STREAM_NAME H_TEXT("标准流")
#define HAZE_BASE_LIBRARY_STREAM_CODE H_TEXT("标准库 标准流\n\
{\n\
	函数\n\
	{\n\
		空 打印(字符* 甲, ...)\n\
		空 输入(字符* 甲, ...)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_MEMORY_NAME H_TEXT("标准内存")
#define HAZE_BASE_LIBRARY_MEMORY_CODE H_TEXT("标准库 标准内存\n\
{\n\
	函数\n\
	{\n\
		空 内存复制(空* 目标地址, 空* 源地址, 正长整数 字节数)\n\
		空 多对象构造(空* 首地址, 空* 构造函数地址, 正长整数 对象大小, 正长整数 对象个数)\n\
		长整数 获得字符个数(空* 字符首地址)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_FILE_NAME H_TEXT("标准文件")
#define HAZE_BASE_LIBRARY_FILE_CODE H_TEXT("标准库 标准文件\n\
{\n\
	函数\n\
	{\n\
		空* 打开文件(字符* 文件路径, 整数 操作方式)\n\
		空 关闭文件(空* 文件指针)\n\
		整数 读取字符(空* 文件指针)\n\
		字符* 读取字符串(空* 文件指针, 整数 最大个数, 字符* 单个字符串)\n\
		字符* 读取一行(空* 文件指针, 字符* 单个字符串)\n\
		正长整数 读取(空* 文件指针, 正长整数 字节数, 正长整数 个数, 字符* 单个字符串)\n\
		整数 写入字符(空* 文件指针, 字符 单个字符)\n\
		整数 写入字符串(空* 文件指针, 字符* 单个字符串)\n\
		正长整数 写入(空* 文件指针, 正长整数 字节数, 正长整数 个数, 字符* 单个字符串)\n\
	}\n\
}")

#define HAZE_BASE_LIBS { HAZE_BASE_LIBRARY_STREAM_NAME, HAZE_BASE_LIBRARY_STREAM_CODE, \
						 HAZE_BASE_LIBRARY_MEMORY_NAME, HAZE_BASE_LIBRARY_MEMORY_CODE, \
						 HAZE_BASE_LIBRARY_FILE_NAME, HAZE_BASE_LIBRARY_FILE_CODE \
 }