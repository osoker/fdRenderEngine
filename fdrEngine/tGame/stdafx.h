// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
// Windows 头文件：
#include <windows.h>
# include <Windowsx.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>



// TODO: 在此处引用程序要求的附加头
# include "../interface/osInterface.h"
# include "Character.h"

#include "../common/com_include.h"

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料

#ifndef WINVER				// 允许使用 Windows 95 和 Windows NT 4 或更高版本的特定功能。
#define WINVER 0x0400		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif

#ifndef _WIN32_WINNT		// 允许使用 Windows NT 4 或更高版本的特定功能。
#define _WIN32_WINNT 0x0400		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif						

#ifndef _WIN32_WINDOWS		// 允许使用 Windows 98 或更高版本的特定功能。
#define _WIN32_WINDOWS 0x0410 //为 Windows Me 及更新版本改变为适当的值。
#endif

//# include "../astar/newaStar.h"

// lib 
# pragma comment ( lib,"common.lib" )
# pragma comment ( lib,"fdREngine.lib" )
# pragma comment ( lib,"astar.lib" )
# pragma comment ( lib,"smManager.lib" )
# pragma comment ( lib,"fileOperate.lib" )

