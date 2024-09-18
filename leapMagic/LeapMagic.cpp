// LeapMagic.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "LeapMagic.h"
# include "lm_configInfoReader.h"
# include "gameApp.h"
# include <zmouse.h>

#define MAX_LOADSTRING 100


// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

#define   LOGFILE_NAME   "log\\fdLog"
#define   LOGDIR_CREATE  "������ϷĿ¼�´���logĿ¼"





//
// ���ݵ�ǰ�����ں�ʱ�䣬�õ�log�ļ���.log�ļ��ŵ�logĿ¼�¡�
bool get_logfileName( char* _lf )
{
	//gFontManager.RegisterFont(EFT_Font2,"����",16);
	SYSTEMTIME   t_systime;
	s_string     t_name;
	char         t_str[256];

	GetLocalTime( &t_systime );

	t_name = "";
	t_name += LOGFILE_NAME;
	sprintf( t_str,"%d��_%d��_%d��_%dʱ_%d��.txt",t_systime.wYear,
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
	// TODO: �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	//! �����log�ļ�����
	char   t_szLogFile[256];

	// �������ǳ���ʱ��log�ļ�.
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

		// ��ʼ��ȫ���ַ���
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_LEAPMAGIC, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// ִ��Ӧ�ó����ʼ��:
		if (!InitInstance (hInstance, nCmdShow)) 
		{
			return FALSE;
		}

		hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_LEAPMAGIC);

		// ����Ϣѭ����
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
		// ����Debug��������
		sgdebug_end( _str );

		ChangeDisplaySettings( NULL,0 );
		release_renderRes();
		release_packinterface();

	}



	ChangeDisplaySettings( NULL,0 );
	release_renderRes();
	release_packinterface();

	// ����Debug��������
	sgdebug_end();

	return (int) msg.wParam;
}



//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��: 
//
//    ����ϣ��������ӵ� Windows 95 ��
//    ��RegisterClassEx������֮ǰ�˴����� Win32 ϵͳ����ʱ��
//    ����Ҫ�˺��������÷������ô˺���
//    ʮ����Ҫ������Ӧ�ó���Ϳ��Ի�ù�����
//   ����ʽ��ȷ�ġ�Сͼ�ꡣ
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
  �����˶���Ԫ���У�

  ĳ������ļ��ٶ�.
  ��������һֱ���ڵļ��ٶ�.
  ���������������ٶ�.
  
  �����������ֵ���ܹ��õ����յ�Ч����
 
  ʵ�����������ٶȣ��������ٶȵ���ֵ����������Ӱ�����յ��˶�.
  ��ر�����ײ������䵽�ر��ϣ�������µĴ���
  ���������ٶȷ�������ʱ�仯������Ʒ���˶�����ʱ���෴.

  ���ֵ��ʱ�򣬳�ʼ���ٶȱ�Ϊ�㣬������ֻ�����������������á�
  ���ٶ�Ϊ˲����ٶȣ��ٶȺܸߣ�ʱ��ܿ죬Ȼ����ٶȵ�ʱ�������
  �����������ٶȺͼ��ٶ��ˡ�

 */
/*
struct s_acceleration
{
	// ���ٶȵķ���ʹ�С
	osVec3d   m_vec3AcceDir;
	float     m_fAcceSpeed;
};
*/

//
//   ����: InitInstance(HANDLE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
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

	// ����ȫ�ֵ�3D�豸��ʼ�����ݡ�
	if( !g_ptrConfigInfoReader->read_d3dDevInitVar( gc_gameApp::m_sDevInit ) )
		return FALSE;

	sprintf( t_szTitle,"GameTest" );
	if( gc_gameApp::m_sDevInit.m_bFullScr )
	{
		t_dwWindowStyle = WS_POPUP;
		// �޸�׿��ֱ��ʡ�
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
//  ����: WndProc(HWND, unsigned, WORD, LONG)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
/*  ���߳���Ϣ����Ĵ������̣�
    1:����gameappҪ�����message,���뵽gameApp����Ϣ���С�

    2:���������ȡ��Ϣ����ʱ��ȡ.��ȡ������Ϣ�󣬴浽����Ļ�������.

    3:��������̣߳���Ⱦ��ʼ֮ǰ���ȴ�����Ϣ�����������е���Ϣ��������ȡ��������Ϣ��

    4:������Ϣ������windows��������Ϣ��ȡ��
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_ptrGameApp->msg_proc( hWnd,message,wParam,lParam );

}


