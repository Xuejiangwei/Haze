#pragma once

#define HAZE_BASE_LIBRARY_STREAM_NAME H_TEXT("��׼��")
#define HAZE_BASE_LIBRARY_STREAM_CODE H_TEXT("��̬�� ��׼��\n\
{\n\
	����\n\
	{\n\
		�� ��ӡ(�ַ� ��, ...)\n\
		�� ����(�ַ� ��, ...)\n\
		��̬�� ���ɶ�̬��()\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_MEMORY_NAME H_TEXT("��׼�ڴ�")
#define HAZE_BASE_LIBRARY_MEMORY_CODE H_TEXT("��̬�� ��׼�ڴ�\n\
{\n\
	����\n\
	{\n\
		�� �ڴ渴��(������64 Ŀ���ַ, ������64 Դ��ַ, ������64 �ֽ���)\n\
		�� �������(������64 �׵�ַ, ������64 ���캯����ַ, ������64 �����С, ������64 �������)\n\
	}\n\
}")

#define HAZE_BASE_LIBRARY_FILE_NAME H_TEXT("��׼�ļ�")
#define HAZE_BASE_LIBRARY_FILE_CODE H_TEXT("��̬�� ��׼�ļ�\n\
{\n\
	����\n\
	{\n\
		������64 ���ļ�(������64 �ļ�·��, ���� ������ʽ)\n\
		�� �ر��ļ�(������64 �ļ�ָ��)\n\
		���� ��ȡ�ַ�(������64 �ļ�ָ��)\n\
		������64 ��ȡ�ַ���(������64 �ļ�ָ��, ���� ������, ������64 �����ַ���)\n\
		������64 ��ȡһ��(������64 �ļ�ָ��, ������64 �����ַ���)\n\
		������64 ��ȡ(������64 �ļ�ָ��, ������64 �ֽ���, ������64 ����, ������64 �����ַ���)\n\
		���� д���ַ�(������64 �ļ�ָ��, ������ �����ַ�)\n\
		���� д���ַ���(������64 �ļ�ָ��, ������64 �����ַ���)\n\
		�������� д��(������64 �ļ�ָ��, ������64 �ֽ���, ������64 ����, ������64 �����ַ���)\n\
	}\n\
}")

#define HAZE_BASE_LIBS { HAZE_BASE_LIBRARY_STREAM_NAME, HAZE_BASE_LIBRARY_STREAM_CODE, \
						 HAZE_BASE_LIBRARY_MEMORY_NAME, HAZE_BASE_LIBRARY_MEMORY_CODE, \
						 HAZE_BASE_LIBRARY_FILE_NAME, HAZE_BASE_LIBRARY_FILE_CODE \
 }