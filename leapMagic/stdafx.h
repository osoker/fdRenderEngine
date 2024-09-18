// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
// Windows 头文件:
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: 在此处引用程序要求的附加头文件

# include "../fdrEngine/interface/osInterface.h"
# include "direct.h"



//! 使用我们的链接库
#pragma comment ( lib,"winmm" )
#pragma comment ( lib,"d3dx9" )

#ifdef _DEBUG

#pragma comment ( lib,"../lib/common" )
#pragma comment ( lib,"../lib/fdREngine" )
#pragma comment ( lib,"../lib/fileOperate" )
#pragma comment ( lib,"../lib/smManager" )

# else

#pragma comment ( lib,"../lib/commonR" )
#pragma comment ( lib,"../lib/fdREngineR" )
#pragma comment ( lib,"../lib/fileOperateR" )
#pragma comment ( lib,"../lib/smManagerR" )

# endif 

//! 是否是修改地表碰撞信息的版本,如果要修改成可以修改碰撞信息的版本的话，则
//! 把宏__COLL_DISGRID__的数据修改成5.
# define   __COLL_MOD__   0

//! 中心点周围显示多少层碰撞信息,必须是奇数
# define   __COLL_DISGRID__   5


//! 加入刷怪信息相关的操作和数据
# define  __DEPLOY_MONSTERMOD__ 0
//! 刷怪区域最大的顶点数目.
# define MAX_POINTS   100

# if 0
#ifdef _DEBUG

//!用于内存操作
#pragma comment( lib,"../../lib/strmbasd.lib")

//!媒体流控制
#pragma comment( lib,"../../lib/asynbase.lib")


#endif 

#ifdef NDEBUG

//!用于内存操作
#pragma comment( lib,"../../lib/strmbaseR.lib")

//!媒体流控制
#pragma comment( lib,"../../lib/asynbaseR.lib")

#endif
# endif 