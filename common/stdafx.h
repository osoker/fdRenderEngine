// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����

// Windows ͷ�ļ�:
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <direct.h>
#include <time.h>

#include <vector>
#include <list>

#if _MSC_VER > 1310 // more than VC71
	#define stricmp _stricmp
	#define itoa	_itoa
	#define mkdir	_mkdir
#endif //_MSC_VER > 1310 // more than VC71

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ�ļ�
