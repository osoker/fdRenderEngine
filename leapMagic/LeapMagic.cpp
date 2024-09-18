// LeapMagic.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "LeapMagic.h"
# include "lm_configInfoReader.h"
# include "gameApp.h"
# include <zmouse.h>

#define MAX_LOADSTRING 100


// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

#define   LOGFILE_NAME   "log\\fdLog"
#define   LOGDIR_CREATE  "请在游戏目录下创建log目录"





//
// 根据当前的日期和时间，得到log文件名.log文件放到log目录下。
bool get_logfileName( char* _lf )
{
	//gFontManager.RegisterFont(EFT_Font2,"黑体",16);
	SYSTEMTIME   t_systime;
	s_string     t_name;
	char         t_str[256];

	GetLocalTime( &t_systime );

	t_name = "";
	t_name += LOGFILE_NAME;
	sprintf( t_str,"%d年_%d月_%d日_%d时_%d分.txt",t_systime.wYear,
		t_systime.wMonth,t_systime.wDay,t_systime.wHour,t_systime.wMinute );

	t_name += t_str;
	strcpy( _lf,t_name.c_str() );

	return true;
};

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	//! 输出的log文件名。
	char   t_szLogFile[256];

	// 设置我们出错时的log文件.
	get_logfileName( t_szLogFile );
	if( !sgdebug_start( t_szLogFile ) )
	{
		MessageBox( NULL,LOGDIR_CREATE,"ERROR",MB_OK );		
		return -1;
	}

	try
	{
		::init_packinterface( "load_pack.ini" );

		if( !g_ptrConfigInfoReader->read_engineInitVar() )
			osassert( false );

		//::init_packinterface( g_ptrConfigInfoReader->get_packIniName() );

		// 初始化全局字符串
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_LEAPMAGIC, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// 执行应用程序初始化:
		if (!InitInstance (hInstance, nCmdShow)) 
		{
			return FALSE;
		}

		hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_LEAPMAGIC);

		// 主消息循环：
		ZeroMemory( &msg, sizeof(msg) );
		while( msg.message!=WM_QUIT )
		{
			if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				PROFILEMOVESTART;

//				if( g_bActive )
					g_ptrGameApp->render_gameApp();

				PROFILEMOVEEND;
			}
		}
	}
	catch( char* _str )
	{
		// 结束Debug缓冲区。
		sgdebug_end( _str );

		ChangeDisplaySettings( NULL,0 );
		release_renderRes();
		release_packinterface();

	}



	ChangeDisplaySettings( NULL,0 );
	release_renderRes();
	release_packinterface();

	// 结束Debug缓冲区。
	sgdebug_end();

	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释: 
//
//    仅当希望在已添加到 Windows 95 的
//    “RegisterClassEx”函数之前此代码与 Win32 系统兼容时，
//    才需要此函数及其用法。调用此函数
//    十分重要，这样应用程序就可以获得关联的
//   “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_LEAPMAGIC);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOWTEXT+1);
	wcex.lpszMenuName	= NULL;//(LPCTSTR)IDC_LEAPMAGIC;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}





/*
  决定运动的元素有：

  某个方向的加速度.
  阻力，即一直存在的减速度.
  重力，即重力加速度.
  
  调整这个三个值，能够得到最终的效果。
 
  实质是三个加速度，三个加速度的数值调整，将会影响最终的运动.
  与地表做碰撞，如果落到地表上，则进行新的处理。
  其中阻力速度方向是随时变化，跟物品的运动方向时刻相反.

  最峰值的时候，初始加速度变为零，接下来只有阻力和重力起作用。
  加速度为瞬间加速度，速度很高，时间很快，然后加速度的时间结束，
  接下来就是速度和减速度了。

 */
/*
struct s_acceleration
{
	// 加速度的方向和大小
	osVec3d   m_vec3AcceDir;
	float     m_fAcceSpeed;
};
*/

//
//   函数: InitInstance(HANDLE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

	HWND hWnd;
	char    t_szTitle[128];
	DWORD   t_dwWindowStyle;

	hInst = hInstance; 

	// TEST CODE:
	osPlane t_plane;
	osVec3D v01,v10,v11;

	v01 = osVec3D( 0,10,0 );
	v10 = osVec3D( 10,10,0 );
	v11 = osVec3D( 0,10,-10 );
	osPlaneFromPoints(&t_plane,&v01,&v10,&v11);

	osVec3D normal( 1.0,0.0,0.0 );
	::osPlaneFromPointNormal( &t_plane,&v10,&normal );

	// 读入全局的3D设备初始化数据。
	if( !g_ptrConfigInfoReader->read_d3dDevInitVar( gc_gameApp::m_sDevInit ) )
		return FALSE;

	sprintf( t_szTitle,"GameTest" );
	if( gc_gameApp::m_sDevInit.m_bFullScr )
	{
		t_dwWindowStyle = WS_POPUP;
		// 修改卓面分辩率。
		if( !gc_gameApp::change_deskScreenSize() )
			osassertex( false,"Change Desk screen size failed...\n" );
	}
	else
		t_dwWindowStyle = WS_OVERLAPPEDWINDOW;


	hWnd = CreateWindow(szWindowClass, t_szTitle, t_dwWindowStyle,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	gc_gameApp::resetWindowSizeNStyle( hWnd );

	if( !g_ptrGameApp->init_gameApp( hWnd,hInstance ) )
		osassert( false );

	
	return TRUE;
}

//
//  函数: WndProc(HWND, unsigned, WORD, LONG)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
/*  多线程消息处理的大体流程：
    1:对于gameapp要处理的message,推入到gameApp的消息队列。

    2:对于网络读取消息，即时读取.读取网络消息后，存到另外的缓冲区内.

    3:在另外的线程，渲染开始之前，先处理消息缓冲区内所有的消息，包括读取的网络消息。

    4:其它消息，调用windows的网络消息读取，
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_ptrGameApp->msg_proc( hWnd,message,wParam,lParam );

}


