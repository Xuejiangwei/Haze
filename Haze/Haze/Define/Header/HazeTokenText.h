#pragma once

#define HAZE_SINGLE_COMMENT				H_TEXT("//")
#define HAZE_MULTI_COMMENT_START		H_TEXT("/*")
#define HAZE_MULTI_COMMENT_END			H_TEXT("*/")

#define TOKEN_VOID						H_TEXT("空")
#define TOKEN_BOOL						H_TEXT("布尔")
#define TOKEN_CHAR						H_TEXT("字符")
#define TOKEN_BYTE						H_TEXT("字节")
#define TOKEN_SHORT						H_TEXT("双字节")
#define TOKEN_INT						H_TEXT("整数")
#define TOKEN_FLOAT						H_TEXT("小数")
#define TOKEN_LONG						H_TEXT("长整数")
#define TOKEN_DOUBLE					H_TEXT("长小数")

#define TOKEN_UNSIGNED_BYTE				H_TEXT("正字节")
#define TOKEN_UNSIGNED_SHORT			H_TEXT("正双字节")
#define TOKEN_UNSIGNED_INT				H_TEXT("正整数")
#define TOKEN_UNSIGNED_LONG				H_TEXT("正长整数")

#define TOKEN_ARRAY_START				H_TEXT("[")
#define TOKEN_ARRAY_END					H_TEXT("]")

#define TOKEN_STRING_MATCH				H_TEXT("\"")

#define TOKEN_CLASS						H_TEXT("类")
#define TOKEN_CLASS_DATA				H_TEXT("数据")
#define TOKEN_CLASS_DATA_PUBLIC			H_TEXT("公")
#define TOKEN_CLASS_DATA_PRIVATE		H_TEXT("私")
#define TOKEN_CLASS_DATA_PROTECTED		H_TEXT("共")

#define TOKEN_FUNCTION					H_TEXT("函数")

#define TOKEN_ENUM						H_TEXT("枚举")

#define TOKEN_TRUE						H_TEXT("真")
#define TOKEN_FALSE						H_TEXT("假")

#define TOKEN_ADD						H_TEXT("+")
#define TOKEN_SUB						H_TEXT("-")
#define TOKEN_MUL						H_TEXT("*")
#define TOKEN_DIV						H_TEXT("/")
#define TOKEN_MOD						H_TEXT("%")

#define TOKEN_AND						H_TEXT("且")		//&&
#define TOKEN_OR						H_TEXT("或")		//||
#define TOKEN_NOT						H_TEXT("非")		//!

#define TOKEN_BIT_AND					H_TEXT("&")
#define TOKEN_BIT_OR					H_TEXT("|")
#define TOKEN_BIT_NEG					H_TEXT("~")
#define TOKEN_BIT_XOR					H_TEXT("^")

#define TOKEN_LEFT_MOVE					H_TEXT("<<")
#define TOKEN_RIGHT_MOVE				H_TEXT(">>")

#define TOKEN_ASSIGN					H_TEXT("=")
#define TOKEN_EQUAL						H_TEXT("==")
#define TOKEN_NOT_EQUAL					H_TEXT("!=")
#define TOKEN_GREATER					H_TEXT(">")
#define TOKEN_GREATER_EQUAL				H_TEXT(">=")
#define TOKEN_LESS						H_TEXT("<")
#define TOKEN_LESS_EQUAL				H_TEXT("<=")

#define TOKEN_INC						H_TEXT("++")
#define TOKEN_DEC						H_TEXT("--")

#define TOKEN_ADD_ASSIGN				H_TEXT("+=")
#define TOKEN_SUB_ASSIGN				H_TEXT("-=")
#define TOKEN_MUL_ASSIGN				H_TEXT("*=")
#define TOKEN_DIV_ASSIGN				H_TEXT("/=")
#define TOKEN_MOD_ASSIGN				H_TEXT("%=")
#define TOKEN_SHL_ASSIGN				H_TEXT("<<=")
#define TOKEN_SHR_ASSIGN				H_TEXT(">>=")
#define	TOKEN_BIT_AND_ASSIGN			H_TEXT("&=")
#define	TOKEN_BIT_OR_ASSIGN				H_TEXT("|=")
#define	TOKEN_BIT_XOR_ASSIGN			H_TEXT("^=")

#define TOKEN_LEFT_PARENTHESES			H_TEXT("(")
#define TOKEN_RIGHT_PARENTHESES			H_TEXT(")")

#define TOKEN_COMMA						H_TEXT(",")

#define TOKEN_LEFT_BRACE				H_TEXT("{")
#define TOKEN_RIGHT_BRACE				H_TEXT("}")

#define TOKEN_QUESTION_MARK				H_TEXT("?")
#define TOKEN_QUESTIOB_COLON			H_TEXT(":")

#define TOKEN_IF						H_TEXT("若")
#define TOKEN_ELSE						H_TEXT("否则")

#define TOKEN_FOR						H_TEXT("循环")
#define TOKEN_FOR_STEP					H_TEXT("步进")

#define TOKEN_BREAK						H_TEXT("跳出")
#define TOKEN_CONTINUE					H_TEXT("跳过")
#define TOKEN_RETURN					H_TEXT("返")

#define TOKEN_WHILE						H_TEXT("当")

#define TOKEN_CAST						H_TEXT("转")

#define TOKEN_VIRTUAL					H_TEXT("重载")
#define TOKEN_PUREVIRTUAL				H_TEXT("必重载")

#define TOKEN_DEFINE					H_TEXT("定义")

#define TOKEN_STANDARD_LIBRARY			H_TEXT("标准库")
#define TOKEN_DLL_LIBRARY				H_TEXT("动态库")
#define TOKEN_IMPORT_MODULE				H_TEXT("引")

#define TOKEN_MULTI_VARIABLE			H_TEXT("...")

#define TOKEN_NEW						H_TEXT("生成")

#define TOKEN_NULL_PTR					H_TEXT("空指针")

#define TOKEN_TYPENAME					H_TEXT("类型")
#define TOKEN_TEMPLATE					H_TEXT("模板")

#define TOkEN_ARRAY_LENGTH				H_TEXT("数组长度")
#define TOKEN_SIZE_OF					H_TEXT("字节数")
