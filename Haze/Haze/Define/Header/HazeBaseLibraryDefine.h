#pragma once

#define HAZE_BASE_LIBRARY_STREAM_NAME H_TEXT("��׼��")
#define HAZE_BASE_LIBRARY_STREAM_CODE H_TEXT("��׼�� ��׼��\n\
{\n\
	����\n\
	{\n\
		�� ��ӡ(�ַ�* ��, ...)\n\
		�� ����(�ַ�* ��, ...)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_MEMORY_NAME H_TEXT("��׼�ڴ�")
#define HAZE_BASE_LIBRARY_MEMORY_CODE H_TEXT("��׼�� ��׼�ڴ�\n\
{\n\
	����\n\
	{\n\
		�� �ڴ渴��(��* Ŀ���ַ, ��* Դ��ַ, ������ �ֽڴ�С)\n\
		�� �������(��* �׵�ַ, ��* ���캯����ַ, ������ �����С, ������ �������)\n\
		������ ����ַ�����(��* �ַ��׵�ַ)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_FILE_NAME H_TEXT("��׼�ļ�")
#define HAZE_BASE_LIBRARY_FILE_CODE H_TEXT("��׼�� ��׼�ļ�\n\
{\n\
	����\n\
	{\n\
		����* ���ļ�(�ַ�* �ļ�·��, �ַ�* ������ʽ)\n\
		�� �ر��ļ�(����* �ļ�ָ��)\n\
		���� ��ȡ�ַ�(����* �ļ�ָ��, ���� �����ַ�)\n\
		�ַ�* ��ȡ�ַ���(����* �ļ�ָ��, ���� ������, �ַ�* �����ַ���)\n\
		�������� ��ȡ(����* �ļ�ָ��, �������� �ֽڴ�С, �������� ����, �ַ�* �����ַ���)\n\
		���� д���ַ�(����* �ļ�ָ��, ���� �����ַ�)\n\
		���� д���ַ���(����* �ļ�ָ��, �ַ�* �����ַ���)\n\
		�������� д��(����* �ļ�ָ��, �������� �ֽڴ�С, �������� ����, �ַ�* �����ַ���)\n\
	}\n\
}")

#define HAZE_BASE_LIBS { HAZE_BASE_LIBRARY_STREAM_NAME, HAZE_BASE_LIBRARY_STREAM_CODE, \
						 HAZE_BASE_LIBRARY_MEMORY_NAME, HAZE_BASE_LIBRARY_MEMORY_CODE, \
						 HAZE_BASE_LIBRARY_FILE_NAME, HAZE_BASE_LIBRARY_FILE_CODE \
 }