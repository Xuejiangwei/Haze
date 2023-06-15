#pragma once

#define HAZE_SINGLE_COMMENT				HAZE_TEXT("//")
#define HAZE_MULTI_COMMENT_START		HAZE_TEXT("/*")
#define HAZE_MULTI_COMMENT_END			HAZE_TEXT("*/")

#define TOKEN_VOID						HAZE_TEXT("��")
#define TOKEN_BOOL						HAZE_TEXT("����")
#define TOKEN_CHAR						HAZE_TEXT("�ַ�")
#define TOKEN_BYTE						HAZE_TEXT("�ֽ�")
#define TOKEN_SHORT						HAZE_TEXT("˫�ֽ�")
#define TOKEN_INT						HAZE_TEXT("����")
#define TOKEN_FLOAT						HAZE_TEXT("С��")
#define TOKEN_LONG						HAZE_TEXT("������")
#define TOKEN_DOUBLE					HAZE_TEXT("��С��")

#define TOKEN_UNSIGNED_BYTE				HAZE_TEXT("���ֽ�")
#define TOKEN_UNSIGNED_SHORT			HAZE_TEXT("��˫�ֽ�")
#define TOKEN_UNSIGNED_INT				HAZE_TEXT("������")
#define TOKEN_UNSIGNED_LONG				HAZE_TEXT("��������")

#define TOKEN_ARRAY						HAZE_TEXT("����")
#define TOKEN_ARRAY_START				HAZE_TEXT("[")
#define TOKEN_ARRAY_END					HAZE_TEXT("]")

#define TOKEN_STRING_MATCH				HAZE_TEXT("\"")

#define TOKEN_CLASS						HAZE_TEXT("��")
#define TOKEN_CLASS_DATA				HAZE_TEXT("����")
#define TOKEN_CLASS_DATA_PUBLIC			HAZE_TEXT("��")
#define TOKEN_CLASS_DATA_PRIVATE		HAZE_TEXT("˽")
#define TOKEN_CLASS_DATA_PROTECTED		HAZE_TEXT("��")

#define TOKEN_FUNCTION					HAZE_TEXT("����")

#define TOKEN_MAIN_FUNCTION				HAZE_MAIN_FUNCTION_TEXT

#define TOKEN_TRUE						HAZE_TEXT("��")
#define TOKEN_FALSE						HAZE_TEXT("��")

#define TOKEN_ADD						HAZE_TEXT("+")
#define TOKEN_SUB						HAZE_TEXT("-")
#define TOKEN_MUL						HAZE_TEXT("*")
#define TOKEN_DIV						HAZE_TEXT("/")
#define TOKEN_MOD						HAZE_TEXT("%")

#define TOKEN_AND						HAZE_TEXT("��")		//&&
#define TOKEN_OR						HAZE_TEXT("��")		//||
#define TOKEN_NOT						HAZE_TEXT("��")		//!

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

#define TOKEN_IF						HAZE_TEXT("��")
#define TOKEN_ELSE						HAZE_TEXT("����")

#define TOKEN_FOR						HAZE_TEXT("ѭ��")
#define TOKEN_FOR_STEP					HAZE_TEXT("����")

#define TOKEN_BREAK						HAZE_TEXT("����")
#define TOKEN_CONTINUE					HAZE_TEXT("����")
#define TOKEN_RETURN					HAZE_TEXT("��")

#define TOKEN_WHILE						HAZE_TEXT("��")

#define TOKEN_CAST						HAZE_TEXT("ת")

#define TOKEN_DEFINE					HAZE_TEXT("����")

#define TOKEN_STANDARD_LIBRARY			HAZE_TEXT("��׼��")
#define TOKEN_DLL_LIBRARY				HAZE_TEXT("��̬��")
#define TOKEN_IMPORT_MODULE				HAZE_TEXT("��")

#define TOKEN_MULTI_VARIABLE			HAZE_TEXT("...")

#define TOKEN_NEW						HAZE_TEXT("����")

#define TOKEN_NULL_PTR					HAZE_TEXT("��ָ��")
