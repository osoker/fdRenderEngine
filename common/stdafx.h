// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料

// Windows 头文件:
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

// TODO: 在此处引用程序要求的附加头文件
