// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料

#if _MSC_VER > 1310 // more than VC71
	#define stricmp _stricmp
	#define itoa	_itoa
	#define mkdir	_mkdir
	#define tzset	_tzset
#endif //_MSC_VER > 1310 // more than VC71

#ifdef SCENE_EDITOR
	#include "../../gameTool/MapEditor/scene_editor/scene_editor/stdafx.h"
#else

	#ifdef SCENE_RENDER
		#include "../../gameTool/MapEditor/SceneRender/stdafx.h"
	#else

		// Windows 头文件:
		#include <windows.h>
		#include <windowsx.h>
		#include <Winsock2.h>
		#include <direct.h>
		#include <tchar.h>
		#include <stdio.h>
		#include <stdarg.h>
		#include <algorithm>
		#include <functional>
		#include <numeric>
		#include <string>
		#include <stdlib.h>
		#include <ctype.h>

		#include <D3D9.h>
		#include <dmusicc.h>
		#include <dmusici.h>
		#include <dsound.h>
		#include <dxerr9.h>
		#include <mmsystem.h>
		#include <dxerr9.h>
		#include <d3dx9mesh.h>
		#include <vfw.h>
		#include <dinput.h>



		# include "shlwapi.h" 
		# pragma comment ( lib,"shlwapi.lib" )
		# pragma comment( lib ,"winmm" )
		# pragma comment( lib ,"d3d9" )
		# pragma comment( lib ,"d3dx9" )
		# pragma comment( lib ,"dxerr9" )
		# pragma comment( lib ,"dsound" )
		# pragma comment(lib, "vfw32.lib")
		# pragma comment (lib,"winmm.lib")
		# pragma comment( lib,"Ws2_32" )

		# pragma comment( lib,"dinput8" )
		# pragma comment( lib,"dxguid" )


		# include "..\..\common\include\sgdebug.h"
		# include "..\interface\fileoperate.h"
		# include "..\interface\smPlayerInterface.h"

	#if _MSC_VER > 1310

		# ifdef _DEBUG
			#pragma comment ( lib,"../../lib/common.8.0.lib" )
			#pragma comment ( lib,"../../lib/fileOperate.8.0.lib" )
			#pragma comment ( lib,"../../lib/smManager.8.0.lib" )
		# else
			#pragma comment ( lib,"../../lib/commonR.lib" )
			#pragma comment ( lib,"../../lib/fileOperateR.lib" )
			#pragma comment ( lib,"../../lib/smManagerR.lib" ) 
		# endif 
   # else
		#ifdef _DEBUG
		#	pragma comment(lib,"../../lib/common.lib")
		#	pragma comment(lib,"../../lib/fileOperate.lib")
		#	pragma comment(lib,"../../lib/smManager.lib")
		#else
		#	pragma comment(lib,"../../lib/commonR.lib")
		#	pragma comment(lib,"../../lib/fileOperateR.lib")
		#	pragma comment(lib,"../../lib/smManagerR.lib")
		#endif //_DEBUG
   # endif 

	#endif //SCENE_RENDER

#endif //SCENCE_EDITOR
