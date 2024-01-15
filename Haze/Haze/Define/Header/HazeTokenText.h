#pragma once

#define HAZE_SINGLE_COMMENT				HAZE_TEXT("//")
#define HAZE_MULTI_COMMENT_START		HAZE_TEXT("/*")
#define HAZE_MULTI_COMMENT_END			HAZE_TEXT("*/")

#define TOKEN_VOID						HAZE_TEXT("空")
#define TOKEN_BOOL						HAZE_TEXT("布尔")
#define TOKEN_CHAR						HAZE_TEXT("字符")
#define TOKEN_BYTE						HAZE_TEXT("字节")
#define TOKEN_SHORT						HAZE_TEXT("双字节")
#define TOKEN_INT						HAZE_TEXT("整数")
#define TOKEN_FLOAT						HAZE_TEXT("小数")
#define TOKEN_LONG						HAZE_TEXT("长整数")
#define TOKEN_DOUBLE					HAZE_TEXT("长小数")

#define TOKEN_UNSIGNED_BYTE				HAZE_TEXT("正字节")
#define TOKEN_UNSIGNED_SHORT			HAZE_TEXT("正双字节")
#define TOKEN_UNSIGNED_INT				HAZE_TEXT("正整数")
#define TOKEN_UNSIGNED_LONG				HAZE_TEXT("正长整数")

#define TOKEN_ARRAY						HAZE_TEXT("数组")
#define TOKEN_ARRAY_START				HAZE_TEXT("[")
#define TOKEN_ARRAY_END					HAZE_TEXT("]")

#define TOKEN_STRING_MATCH				HAZE_TEXT("\"")

#define TOKEN_CLASS						HAZE_TEXT("类")
#define TOKEN_CLASS_DATA				HAZE_TEXT("数据")
#define TOKEN_CLASS_DATA_PUBLIC			HAZE_TEXT("公")
#define TOKEN_CLASS_DATA_PRIVATE		HAZE_TEXT("私")
#define TOKEN_CLASS_DATA_PROTECTED		HAZE_TEXT("共")

#define TOKEN_FUNCTION					HAZE_TEXT("函数")

#define TOKEN_MAIN_FUNCTION				HAZE_MAIN_FUNCTION_TEXT

#define TOKEN_ENUM						HAZE_TEXT("枚举")

#define TOKEN_TRUE						HAZE_TEXT("真")
#define TOKEN_FALSE						HAZE_TEXT("假")

#define TOKEN_ADD						HAZE_TEXT("+")
#define TOKEN_SUB						HAZE_TEXT("-")
#define TOKEN_MUL						HAZE_TEXT("*")
#define TOKEN_DIV						HAZE_TEXT("/")
#define TOKEN_MOD						HAZE_TEXT("%")

#define TOKEN_AND						HAZE_TEXT("且")		//&&
#define TOKEN_OR						HAZE_TEXT("或")		//||
#define TOKEN_NOT						HAZE_TEXT("非")		//!

#define TOKEN_BIT_AND					HAZE_TEXT("&")
#define TOKEN_BIT_OR					HAZE_TEXT("|")
#define TOKEN_BIT_NEG					HAZE_TEXT("~")
#define TOKEN_BIT_XOR					HAZE_TEXT("^")

#define TOKEN_LEFT_MOVE					HAZE_TEXT("<<")
#define TOKEN_RIGHT_MOVE				HAZE_TEXT(">>")

#define TOKEN_ASSIGN					HAZE_TEXT("=")
#define TOKEN_EQUAL						HAZE_TEXT("==")
#define TOKEN_NOT_EQUAL					HAZE_TEXT("!=")
#define TOKEN_GREATER					HAZE_TEXT(">")
#define TOKEN_GREATER_EQUAL				HAZE_TEXT(">=")
#define TOKEN_LESS						HAZE_TEXT("<")
#define TOKEN_LESS_EQUAL				HAZE_TEXT("<=")

#define TOKEN_INC						HAZE_TEXT("++")
#define TOKEN_DEC						HAZE_TEXT("--")

#define TOKEN_ADD_ASSIGN				HAZE_TEXT("+=")
#define TOKEN_SUB_ASSIGN				HAZE_TEXT("-=")
#define TOKEN_MUL_ASSIGN				HAZE_TEXT("*=")
#define TOKEN_DIV_ASSIGN				HAZE_TEXT("/=")
#define TOKEN_MOD_ASSIGN				HAZE_TEXT("%=")
#define TOKEN_SHL_ASSIGN				HAZE_TEXT("<<=")
#define TOKEN_SHR_ASSIGN				HAZE_TEXT(">>=")
#define	TOKEN_BIT_AND_ASSIGN			HAZE_TEXT("&=")
#define	TOKEN_BIT_OR_ASSIGN				HAZE_TEXT("|=")
#define	TOKEN_BIT_XOR_ASSIGN			HAZE_TEXT("^=")

#define TOKEN_LEFT_PARENTHESES			HAZE_TEXT("(")
#define TOKEN_RIGHT_PARENTHESES			HAZE_TEXT(")")

#define TOKEN_COMMA						HAZE_TEXT(",")

#define TOKEN_LEFT_BRACE				HAZE_TEXT("{")
#define TOKEN_RIGHT_BRACE				HAZE_TEXT("}")

#define TOKEN_QUESTION_MARK				HAZE_TEXT("?")
#define TOKEN_QUESTIOB_COLON			HAZE_TEXT(":")

#define TOKEN_IF						HAZE_TEXT("若")
#define TOKEN_ELSE						HAZE_TEXT("否则")

#define TOKEN_FOR						HAZE_TEXT("循环")
#define TOKEN_FOR_STEP					HAZE_TEXT("步进")

#define TOKEN_BREAK						HAZE_TEXT("跳出")
#define TOKEN_CONTINUE					HAZE_TEXT("跳过")
#define TOKEN_RETURN					HAZE_TEXT("返")

#define TOKEN_WHILE						HAZE_TEXT("当")

#define TOKEN_CAST						HAZE_TEXT("转")

#define TOKEN_VIRTUAL					HAZE_TEXT("重载")
#define TOKEN_PUREVIRTUAL				HAZE_TEXT("必重载")

#define TOKEN_DEFINE					HAZE_TEXT("定义")

#define TOKEN_STANDARD_LIBRARY			HAZE_TEXT("标准库")
#define TOKEN_DLL_LIBRARY				HAZE_TEXT("动态库")
#define TOKEN_IMPORT_MODULE				HAZE_TEXT("引")

#define TOKEN_MULTI_VARIABLE			HAZE_TEXT("...")

#define TOKEN_NEW						HAZE_TEXT("生成")

#define TOKEN_NULL_PTR					HAZE_TEXT("空指针")

#define TOKEN_TEMPLATE					HAZE_TEXT("模板")
