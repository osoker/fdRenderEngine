// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
//��ı�׼�������� DLL �е������ļ��������������϶���� COMMON_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
//�κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ 
// COMMON_API ������Ϊ�ǴӴ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
# pragma once

#ifdef COMMON_EXPORTS
	#ifdef _USE_COMMON_STATIC_LIB
		// some project use the common static lib
		//
		#define COMMON_API 
	#else
		#define COMMON_API __declspec(dllexport)
	#endif _USE_STATIC_LIB
#else
	#ifdef _USE_COMMON_STATIC_LIB
		// some project use the common static lib
		//
		#define COMMON_API 
	#else
		#define COMMON_API __declspec(dllimport)
	#endif //_USE_STATIC_LIB
#endif
