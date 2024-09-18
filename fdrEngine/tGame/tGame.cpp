// tGame.cpp : 定义应用程序的入口点。
//
#include "stdafx.h"
# include <Windowsx.h>


#include "tGame.h"

# include "tEngine.h"
#include "cameractrl.h"
# include "Winsock2.h"


# include <WinInet.h>

# pragma comment( lib,"Wininet.lib" )


# pragma comment( lib ,"winmm" )
# pragma comment( lib ,"d3d9" )
# pragma comment( lib ,"d3dx9" )
# pragma comment( lib ,"dxerr9" )
# pragma comment( lib ,"dsound" )	


#define MAX_LOADSTRING 100

# define    GAME_VERSION   "0.65"

# define    DIS_VERSION    __DATE__//"美术内部使用0.04版"
# define    DIS_TIME       __TIME__

// 全局变量：
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 窗口句柄,
HWND  hWnd;

char   g_titleStr[128];


# define   ART_VERSION    1
int g_terrainMaterialIdx = 0;
I_skinMeshObj* pRideObj;
/** \brief
 *  得到key.
 */
DWORD   Get_key( char* _buf,int _size )
{
	DWORD    t_dwSer;
	DWORD    t_dwFlag;
	DWORD    t_dwSer1;

	if( !GetVolumeInformation( "c:/",NULL,0,&t_dwSer,NULL,&t_dwFlag,NULL,0 ) )
		return 0;


	_ultoa( t_dwSer,_buf,10 );

	if( !GetVolumeInformation( "d:/",NULL,0,&t_dwSer1,NULL,&t_dwFlag,NULL,0 ) )
		return 0;

	if( (t_dwSer1+t_dwSer)>0xffffffff )
		return t_dwSer;

	t_dwSer += t_dwSer1;

	_ultoa( t_dwSer,_buf,10 );

	return t_dwSer;

}


# define PASS_DEBUG  0

# define MUTI_NUM   17
# define SUB_NUM    163


/** \brief
 *  处理成为最终的口令.
 */
DWORD  Process_key( DWORD  _key )
{
	char   t_buf[256];
	DWORD  t_dw;

	_ultoa( _key,t_buf,10 );

	if( strlen( t_buf )<6 )
		return -1;

	t_buf[6] = NULL;

	for( int t_i=0;t_i<6;t_i++ )
	{
		BYTE  t_bt;
		
		t_bt = t_buf[t_i];
		if( t_bt<53 )
		{
			t_bt += 5;
			t_buf[t_i] = t_bt;
		}
		else
		{
			t_bt -= 5;
			t_buf[t_i] = t_bt;
		}

	}

	t_dw = atoi( t_buf );


# if PASS_DEBUG 
	MessageBox( NULL,t_buf,"LocalKey",MB_OK );
# endif

	t_dw *= MUTI_NUM;
	t_dw += SUB_NUM;

	return t_dw;

}




/** \brief 
 *  验证口令
 */
bool                 Verify_passWord( void )
{
	FILE*   t_stream;
	char    t_buf[256];
	DWORD   t_dwFileKey,t_dwLocalKey;

	t_stream  = fopen( "sys.ini","rt" );
	if( !t_stream ) 
		return false;

	fscanf( t_stream,"%s",t_buf );
	fclose( t_stream );

	t_dwFileKey = atoi( t_buf );

# if PASS_DEBUG 
	MessageBox( NULL,t_buf,"fileKey",MB_OK );
# endif

	t_dwLocalKey = Get_key( t_buf,256 );
	t_dwLocalKey = Process_key( t_dwLocalKey );

	if( t_dwLocalKey == t_dwFileKey )
		return true;
	
	return false;
}



//! 每次最多的处理的事件数目。
# define MAX_EVENTPROCESS  100
/** \brief
 *  处理服务器发发送的相关数据。
 */
void   process_socket( void )
{
	guard;// process_socket() );

	int t_error = 0;
	int t_type = 0;

/*
// 从服务器接收消息。
	if( !g_ptrSocket->receive() )
	{
		g_ptrSocket->close_socket();
		return;
	}
*/

	for( int t_i=0;t_i<MAX_EVENTPROCESS;t_i++ )
	{
		WORD  t_iMsgSize;
		char * t_msg = g_ptrSocket->read_message( t_iMsgSize,&t_error, &t_type); 
		if( t_msg == NULL ) 
			break;
		if(t_error!=0) 
			osassert( false );

		// 
		// 对接收到消息进行处理。
		//...OnPacketEvent( t_iMsgSize,t_msg );

		_TDEFAULTMESSAGE*  t_test;

		t_test = (_TDEFAULTMESSAGE*)t_msg;

	}

	if( t_i == MAX_EVENTPROCESS )
		osassert( false );
	return;

	unguard;
}


char   t_szLogFile[256];


//! 把出错的log文件上传到指定的ftp服务器。
bool          ftp_logFileToServer( void )
{


	bool t_return = true;
	HINTERNET hInternet,hHost;
	hInternet=InternetOpen("Ipedo",
		LOCAL_INTERNET_ACCESS,
		NULL,0,0);
	if(hInternet==NULL)
	{
		MessageBox(NULL,"传送文件失败！","错误！",MB_OK);
		t_return = false;
	}
	if(t_return)
	{
		hHost=InternetConnect(hInternet,"192.168.0.5",
			21,
			"","",
			INTERNET_SERVICE_FTP,
			INTERNET_FLAG_PASSIVE,0);
		if(hHost==NULL)
		{
			MessageBox(NULL,"连接服务器失败！","错误！",MB_OK);
			t_return = false;
		}
	}

	if(t_return)
	{
		if(FtpSetCurrentDirectory(hHost,"log")==false)
		{
			MessageBox(NULL,"指定目录不存在！","错误！",MB_OK);
			t_return = false;
		}
	}
	if(t_return)
	{
		if((FtpPutFile(hHost,
			t_szLogFile,
			"log.txt",
			FTP_TRANSFER_TYPE_ASCII,0)==FALSE)&&t_return)
		{
			MessageBox(NULL,"传送文件失败！","错误！",MB_OK);
			t_return = false;
		}else{
			MessageBox(NULL,"传送文件成功！","成功！",MB_OK);
		}
	}
	InternetCloseHandle(hHost);
	InternetCloseHandle(hInternet);
	return t_return;
}



// 此代码模块中包含的函数的前向声明：
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	int   t_dwfvf = FVF_OBJECTCOLVERTEX;

	MAXTG_INTGMGR = 16;

	g_bHeroShadow = false;

	strcpy( t_szLogFile,"c:\\log.txt" );
	if( !sgdebug_start( t_szLogFile ) )
		return -1;

	g_bUseHDRLight = FALSE;

	try
	{ 
		//g_bUsePackSystem = true;
		g_bInterruptInProgram = true;

		//　初始化全局的包数据。
		init_packinterface( "load_pack.ini" );

		g_bLogToFile = true;

		g_bHeroShadow = true;


		//g_bExportInfoUseGBuf = true;

		osDebugOut( "Hello,world..\n" );


		// 初始化全局字符串
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_TGAME, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// 执行应用程序初始化：
		if (!InitInstance (hInstance, nCmdShow)) 
		{
			return FALSE;
		}


		::ShowWindow( hWnd,SW_SHOWNORMAL );

		// 初始化渲染需要的资源.
		t_initRender( hInstance,hWnd );


		hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TGAME);

		// 主消息循环：
		ZeroMemory( &msg, sizeof(msg) );
		while( msg.message!=WM_QUIT )
		{
			// 
			// 对整个渲染过程进行profile.
			PROFILEMOVESTART;	

			if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				t_render();
			}
			PROFILEMOVEEND;

		}
	}
	catch( char* _str )
	{
		//termination.
		sgdebug_end( _str,true );

		::ShowCursor( TRUE );

		ChangeDisplaySettings( NULL,0 );

		ftp_logFileToServer();

		// 释放全局的引擎资源
		release_renderRes();

		release_packinterface();

		return 0;
	}

	// 改回默认的卓面。
	ChangeDisplaySettings( NULL,0 );
	
	//normal termination
	sgdebug_end();

	// 释放全局的引擎资源
	release_renderRes();
	release_packinterface();
	

	return (int) msg.wParam;
}



//
//  函数：MyRegisterClass()
//
//  目的：注册窗口类。
//
//  注释：
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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TGAME);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//(LPCTSTR)IDC_TGAME;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


// 是否修改分辨率,得到假全屏. 
# define FAKE_FULLSCREEN   0
//
//   函数：InitInstance(HANDLE, int)
//
//   目的：保存实例句柄并创建主窗口
//
//   注释：
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	guard;


   hInst = hInstance; // 将实例句柄存储在全局变量中

   sprintf( g_titleStr,"内部使用版,版本号:<%s>,编译日期:<%s  %s>",GAME_VERSION,DIS_VERSION,DIS_TIME );

 
# if FAKE_FULLSCREEN
   DEVMODE dmScreenSettings;
   memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
   dmScreenSettings.dmSize=sizeof(dmScreenSettings);
   dmScreenSettings.dmPelsWidth = 1024;
   dmScreenSettings.dmPelsHeight= 768;
   dmScreenSettings.dmBitsPerPel= 32;
   dmScreenSettings.dmDisplayFrequency = 75;
   dmScreenSettings.dmFields= DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;

   if( DISP_CHANGE_SUCCESSFUL != ChangeDisplaySettings( &dmScreenSettings,CDS_FULLSCREEN ) )
	   osassert( false );
# endif 

# if FAKE_FULLSCREEN
   hWnd = CreateWindow( szWindowClass, g_titleStr, WS_POPUP,
      0, 0, 1024, 768, NULL, NULL, hInstance, NULL);
# else
   hWnd = CreateWindow( szWindowClass, g_titleStr,WS_OVERLAPPEDWINDOW,
      0, 0, 1024, 768, NULL, NULL, hInstance, NULL);
# endif 

   if (!hWnd)		
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   osDebugOut( "Create windows success" );

   return TRUE;
   
   unguard;
}


void   move_cameraRight( float _fscale )
{
	osVec3D   t_vec3Right;
	osVec3D   t_pos;
	g_cCamera.m_pCamera->get_rightvec( t_vec3Right );
	t_vec3Right*=_fscale;
	g_cCamera.m_pCamera->get_curpos( &t_pos );
	t_pos += t_vec3Right;
	g_cCamera.m_pCamera->set_curpos( &t_pos );
	g_cCamera.m_pCamera->get_camFocus( t_pos );
	t_pos += t_vec3Right;
	g_cCamera.m_pCamera->set_camFocus( t_pos );

};

void   move_cameraFor( float _f )
{
	osVec3D   t_vec3Forward;
	osVec3D   t_vec3Focus;
	osVec3D   t_vec3Pos;
	osVec3D   t_pos;

	g_cCamera.m_pCamera->get_curpos( &t_vec3Pos );
	g_cCamera.m_pCamera->get_camFocus( t_vec3Focus );
	t_vec3Forward = t_vec3Focus - t_vec3Pos;

	t_vec3Forward.y = 0.0f;
	osVec3Normalize( &t_vec3Forward,&t_vec3Forward );
	t_vec3Forward *= _f;
	g_cCamera.m_pCamera->get_curpos( &t_pos );
	t_pos += t_vec3Forward;
	g_cCamera.m_pCamera->set_curpos( &t_pos );
	g_cCamera.m_pCamera->get_camFocus( t_pos );
	t_pos += t_vec3Forward;
	g_cCamera.m_pCamera->set_camFocus( t_pos );


};

//! 调整相机使用的数值。
# define CAM_ADJ_SIZE 0.05f

//
//  函数：WndProc(HWND, unsigned, WORD, LONG)
//
//  目的：处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
SimpleSpline g_spline;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	DWORD  t_xPos,t_yPos;
	POINT   t_pt;
	static float  t_scale = 1.0f;
	osVec3D  t_vec3Pos;
	float    t_f;

	switch (message) 
	{

		// 处理键盘消息。
	case WM_KEYDOWN:
		wParam = wParam;

		// 按Esc键退出程序.
		if( wParam == 27 )
		{
			::DestroyWindow( hWnd );
			break;
		}
		if ( wParam == 'L')
		{
			// light_test();
			//g_bChrBox = !g_bChrBox;
			//WinExec("HELPER.EXE",SW_SHOW);
			//osColor maxBright(1.0,1.0,1.0,1.0);
			//g_cHero.m_pObj->set_maxBright(maxBright);

			load_tNpc();
			//play_actWithName( NULL );

			//test_snapBbox();
		//	spacemove_test();

			break;
		}
		if (wParam == 'O')
		{
			osColor maxBright(0.0,0.0,0.0,0.0);
			g_cHero.m_pObj->set_maxBright(maxBright);
			//g_sceneMgr->delete_dynamicLight(0,1.0f);
			break;
		}
		if (wParam =='0')
		{
			//LPBYTE   t_fstart;

			//ChaHeader m_header;
			//os_skinMeshInit m_osSkinMeshInit;

			//t_fstart = START_USEGBUF;

			//if( 0 >= read_fileToBuf( "character\\zjilong.cha",t_fstart,TMP_BUFSIZE ) )
			//{
			//	osassert( false );
			//	return false;
			//}

			//memcpy(&m_header,t_fstart,sizeof(ChaHeader));

			//if (m_header.mID==ChaFileID)
			//{
			//	//mHeadPicFileNamStr = (char*)(t_fstart + m_header.mHeadPicFileNameOfs);
			//}
			//END_USEGBUF;

			//strcpy(m_osSkinMeshInit.m_strSMDir,"character\\zjilong");

			//m_osSkinMeshInit.m_iTotalActTypeNum = m_header.mTotalActTypeNum;
			//m_osSkinMeshInit.m_smObjLook.m_iPartNum = m_header.mPartNum;
			//osassert( m_osSkinMeshInit.m_smObjLook.m_iPartNum > 0 );
			//osassert( m_osSkinMeshInit.m_smObjLook.m_iPartNum < MAX_SKINPART );

			//m_osSkinMeshInit.m_vec3Max = m_header.mBoxMax;
			//m_osSkinMeshInit.m_vec3Min = m_header.mBoxMin;
			//m_osSkinMeshInit.m_vec3AdjPos = m_header.mPos;
			//m_osSkinMeshInit.m_fAdjRotAgl = m_header.mRot;
			//m_osSkinMeshInit.m_bUseDirLAsAmbient = m_header.mbUseDirLAsAmbient;
			//m_osSkinMeshInit.m_bIsRide = m_header.mbIsRide;
			//m_osSkinMeshInit.m_vec3RideOffset = m_header.mRidePos;
			//

			//osVec3D t_vec3Pos;
			//float t_fAngle;
			////if( g_cHero.m_pObj )
			//g_cHero.m_pObj->get_posAndAgl( t_vec3Pos,t_fAngle );
			////else
			////	t_vec3Pos = _vec3Pos;
			//m_osSkinMeshInit.m_vec3Pos = t_vec3Pos;

			//WORD mesh[MAX_SKINPART] = { 1,1,1,1,1,0,0,0 };
			//WORD skin[MAX_SKINPART] = { 1,1,1,1,1,0,0,0 };
			//memcpy( m_osSkinMeshInit.m_smObjLook.m_mesh , mesh, sizeof(WORD)*MAX_SKINPART );
			//memcpy( m_osSkinMeshInit.m_smObjLook.m_skin , skin, sizeof(WORD)*MAX_SKINPART );

			//m_osSkinMeshInit.m_iActIdx = 0;
			//pRideObj = g_sceneMgr->create_character(&m_osSkinMeshInit);
			
			//g_cHero.m_pObj->SetRide(pObj);
			//pObj->
			change_weather();
			//g_sceneMgr->delete_dynamicLight(0,1.0f);
			break;
		}
		if (wParam =='1')
		{
			os_aniMeshCreate  t_mc;
			int              t_k; 
			t_mc.m_bAutoDelete = false;
			t_mc.m_dwAmbientL = 0xfffffff;
			t_mc.m_iPlayFrames = 1;
			t_mc.m_ePlayType = OSE_PLAYFOREVER;//
			t_mc.m_fPlayTime = 5.0f;
			t_mc.m_fAngle = 0.0f;

			// 

			strcpy( t_mc.m_szAMName, "keyAni\\H_water01.OSA" );
			t_mc.m_vec3Pos = g_cHero.m_verPos;

			


			 g_sceneMgr->create_osaAmimation( &t_mc );
			
			//g_cHero.m_pObj->SetRide(pRideObj);
			//g_sceneMgr->delete_dynamicLight(1,1.0f);
			break;
		}if (wParam =='2')
		{
			//g_sceneMgr->delete_dynamicLight(2,1.0f);
			//g_spline.clear();
			break;
		}
		if (wParam =='3')
		{
			//g_sceneMgr->delete_dynamicLight(3,1.0f);
			//int LineSegment = 2;
			osVec3D vec3Pos = g_cHero.m_verPos;
			//osVec3D *lineList = new osVec3D[LineSegment];

			//for (int i=0;i < LineSegment;i++)
			//{
			//	lineList[i] = vec3Pos;
			//	lineList[i].x+=1.0f;
			//}
			g_spline.addPoint(vec3Pos);
			break;
		}
		if (wParam =='P')
		{
			extern osVec3D      g_vecLine[MAX_SPLITNUM];
		//	osVec3D splitpoint[MAX_SPLITNUM];
			for (int i = 0 ; i< MAX_SPLITNUM ; ++i)
			{
				g_vecLine[i] = g_spline.interpolate((float)i/MAX_SPLITNUM);
			}
			//g_sceneMgr->draw_line(splitpoint,MAX_SPLITNUM,0xff00ff00);
			/*
			int t_iX = (int)(g_cHero.m_verPos.x*10.0f);
			int t_iY = (int)(g_cHero.m_verPos.z*10.0f);
			osVec3D	pos;
			//osVec3D pos2;
			osVec3D oldpos = g_cHero.m_verPos;
			//g_sceneMgr->get_tilePos((t_iX/15)*2,(t_iY/15)*2,pos2);
			g_sceneMgr->get_detailTilePos(t_iX,t_iY,pos);
			//g_cHero.Move()
			g_cHero.m_pObj->frame_move( pos, 0.0f);
			*/
			//move_to_fmna();
			


			break;
		}
		//if (wParam =='T')
		//{

		//	os_weather    t_wea;
		//	g_sceneMgr->get_sceneWeather( t_wea );

		//	t_wea.m_bRain = false;
		//	t_wea.m_fFogEnd = 500.0f;
		//	t_wea.m_fFogStart = 300.0f;
		//	t_wea.m_sFogColor.a = 0.5f;
		//	t_wea.m_sFogColor.r = 0.5f;
		//	t_wea.m_sFogColor.g = 0.5f;
		//	t_wea.m_sFogColor.b = 0.5f;

		//	t_wea.m_sAmbientColor.a = 1.0f;
		//	t_wea.m_sAmbientColor.r = 0.6f;
		//	t_wea.m_sAmbientColor.g = 0.6f;
		//	t_wea.m_sAmbientColor.b = 0.6f;

		//	t_wea.m_sDirLightColor.a = 1.0f;
		//	t_wea.m_sDirLightColor.r = 0.4f;
		//	t_wea.m_sDirLightColor.g = 0.4f;
		//	t_wea.m_sDirLightColor.b = 0.4f;

		//	t_wea.m_vec3DirLight = osVec3D( 1.0f,1.0f,1.0f );

		//	t_wea.m_fSkyMoveSpeed = 0.01f;
		//	strcpy( t_wea.m_szSkyTex,"dfault_sky" );

		//	//if (t_iVersion>=107)
		//	//{
		//	//	//! 107版本保存的雾开始和结束为与远裁平面的相对值
		//	//	//!g_fTerrFar 为从配置文件中读到的"farclip"字段数据
		//	//	t_wea.m_fFogStart *= g_fTerrFar;
		//	//	t_wea.m_fFogEnd *= g_fTerrFar;
		//	//}
		//	g_sceneMgr->change_sceneWeather( t_wea,5.0f );

		//	break;
		//}
		if (wParam == 'R')
		{

			os_weather    t_wea;

			g_sceneMgr->get_sceneWeather( t_wea );

			t_wea.m_bRain = true;
			t_wea.m_fFogEnd = 100.0f;
			t_wea.m_fFogStart = 20.0f;
			t_wea.m_sFogColor.a = 1.0f;
			t_wea.m_sFogColor.r = 0.4f;
			t_wea.m_sFogColor.g = 0.4f;
			t_wea.m_sFogColor.b = 0.7f;

			t_wea.m_sAmbientColor.a = 1.0f;
			t_wea.m_sAmbientColor.r = 0.4f;
			t_wea.m_sAmbientColor.g = 0.4f;
			t_wea.m_sAmbientColor.b = 0.7f;

			t_wea.m_sDirLightColor.a = 1.0f;
			t_wea.m_sDirLightColor.r = 0.3f;
			t_wea.m_sDirLightColor.g = 0.3f;
			t_wea.m_sDirLightColor.b = 0.7f;

			t_wea.m_vec3DirLight = osVec3D( 1.0f,1.0f,1.0f );

			t_wea.m_fSkyMoveSpeed = 0.01f;
			strcpy( t_wea.m_szSkyTex,"skynight" );
			//if (t_iVersion>=107)
			//{
			//	//! 107版本保存的雾开始和结束为与远裁平面的相对值
			//	//!g_fTerrFar 为从配置文件中读到的"farclip"字段数据
			//	t_wea.m_fFogStart *= g_fTerrFar;
			//	t_wea.m_fFogEnd *= g_fTerrFar;
			//}

			g_sceneMgr->change_sceneWeather( t_wea,5.0f );
			g_sceneMgr;
		}
	//case 'W':
		if (wParam == 'W')
		{
			osVec2D vec3Pos;
			vec3Pos.x = g_cHero.m_verPos.x;
			vec3Pos.y = g_cHero.m_verPos.z;

		//	os_newAction  t_newAction;


			

		//	t_newAction.m_iNewAct = 3;
		//	t_newAction.m_iPlayNum = 2;

		//	t_newAction.m_changeType = OSE_DIS_NEWACT_LASTFRAME;

		//	g_cHero.m_pObj->change_skinMeshAction(t_newAction);

		//	g_bTileWireFrame = !g_bTileWireFrame ;

		    change_action();
		}
		if (wParam == 'F')
		{
			os_weather    t_wea;
			FILE*   t_file = NULL;
			t_file = fopen( "weather\\vilg_dxg.wea","rb" );


			char    t_str[4];
			int     t_iVersion;

			osassert( t_file );
			fread( t_str,sizeof( char ),4,t_file );
			if( strcmp( t_str,"wea" ) != 0 )
				osassertex( false,"不是天气文件" );
			fread( &t_iVersion,sizeof( int ),1,t_file );
			osassertex( t_iVersion >= 102,"天气文件版本号不对，使用最新的天气编辑器" );

			fread( &t_wea,sizeof( os_weather ),1,t_file );
			fclose( t_file );
			if (t_iVersion>=107)
			{
				//! 107版本保存的雾开始和结束为与远裁平面的相对值
				//!g_fTerrFar 为从配置文件中读到的"farclip"字段数据
				t_wea.m_fFogStart *= g_fTerrFar;
				t_wea.m_fFogEnd *= g_fTerrFar;
			}
			g_sceneMgr->change_sceneWeather( t_wea,1.0f );
		//	g_bUpdateTile = !g_bUpdateTile;
		}
		//@{
		// 处理场景中NPC人物的信息.
		if( wParam == 0x70 )
		{
			g_npcAdjust.active_nextNpc();
			break;
		}
		if( wParam == 0x71 )
		{
			g_npcAdjust.reset_npcAction();
			break;
		}

		if( wParam == 0x73 )
		{
			g_npcAdjust.play_selectAction();
			break;
		}

		if( wParam == 0x77 )
		{
			g_npcAdjust.save_npcChrToFile( "chr.ini" );
			break;
		}

		if( wParam == 0x78 )
		{
			g_bCameraMove = !g_bCameraMove;
			break;
		}

		if( wParam == 0x64 )
		{
			g_npcAdjust.rotate_curNpc( 0.02f );
			break;
		}

		if( wParam == 0x66 )
		{
			g_npcAdjust.rotate_curNpc( -0.02f );
			break;
		}

		if( wParam == 0x68 )
		{
			g_npcAdjust.move_npcY( 0.02f );
			break;
		}

		if( wParam == 0x62 )
		{
			g_npcAdjust.move_npcY( -0.02f );
			break;
		}

		if( wParam == 0x67 )
		{
			g_npcAdjust.move_npcX( 0.02f );
			break;
		}

		if( wParam == 0x61 )
		{
			g_npcAdjust.move_npcX( -0.02f );
			break;
		}

		if( wParam == 0x69 )
		{
			g_npcAdjust.move_npcZ( 0.02f );
			break;
		}

		if( wParam == 0x63 )
		{
			g_npcAdjust.move_npcZ( -0.02f );
			break;
		}

		// <-
		if( wParam == 37 )
		{
			adj_volume( -5, SOUNDTYPE_3D );
			break;
		}

		// ->
		if( wParam == 39 )
		{
			adj_volume( +5, SOUNDTYPE_3D );
			break;
		}
		// 
		// 上箭头.
		if( wParam == 38 )
		{
			/*
			if( g_bCameraMove )
				g_cHero.move_heroZ( 0.01f );
			else
				picth_cam( CAM_ADJ_SIZE );
			*/
			move_cameraFor( 1.0f );
			break;
		}
		// 下箭头
		if( wParam == 40 )
		{
			/*
			if( g_bCameraMove )
				g_cHero.move_heroZ( -0.01f );
			else
				picth_cam( -CAM_ADJ_SIZE );
			*/
			move_cameraFor( -1.0f );
			break;
		}

		if( wParam == 'X' )
		{
			adj_volume( -5, SOUNDTYPE_UI );
			break;
		}

		if( wParam == 'T' )
		{
			adj_volume( +5, SOUNDTYPE_UI );
			break;
		}




# if ART_VERSION
		if( wParam == 'E' )
		{
			//test_bolt1();
			
			play_effect1();
			/*float t_f = -0.005;
			g_cCamera.m_pCamera->set_camAspect( t_f );
			*/

			//osVec3D   t_vec3( 1.0f,1.5f,1.0f );
			//g_cHero.m_pObj->scale_skinMesh( t_vec3 );

			break;
		}
		if( wParam == 'R' )
		{
		//	float t_f = 0.005f;
			
			//osVec3D   t_vec3( 1.5f,1.0f,1.0f );
			//g_cHero.m_pObj->scale_skinMesh( t_vec3 );

			//g_cCamera.m_pCamera->set_camAspect( t_f );
			break;
		}

		// 
		// 处理hellohelloo
		if( wParam == 'C' )
		{
			 play_effect2();

			 //play_effect3();

			//g_bCHMod = !g_bCHMod;

		    //test_wEff();

			// next_actPose();

			// t_action();

			//load_equFile();

			break;
		}
	
		if( wParam == 'B' )
		{

/*
			rot_character( 0.1 );

			test_color();
*/
			//g_bCharWireFrame = !g_bCharWireFrame;

			test_actionSuspend();

			//rot_osa();
			break;
		}
		if( wParam == 'N' )
		{
			//rot_character( -0.1 );
			//test_shadow();

			bef_actPose();
			
			break;
		}
# endif 


		if( wParam == 'U' )
		{
			g_bDisInfo = !g_bDisInfo;

			//g_sceneMgr->modify_heightInHeroPos( 0.02f );

			
			osColor t_color( 0.8f,0.8f,0.8f,1.0f );
			g_cHero.m_pObj->set_maxBright( t_color );


			break;
		}

		if( wParam == 'I' )
		{
			//g_sceneMgr->modify_heightInHeroPos( -0.02f );

			osColor t_color( 0.0f,0.0f,0.0f,1.0f );
			g_cHero.m_pObj->set_maxBright( t_color );

			break;
		}

		if( wParam == 'Y' )
		{
			
			rain_test();
			/*g_cHero.m_pObj->start_moveAction();
			*/
			//test_detailTile();

			//g_bRenderBtn = !g_bRenderBtn;

			//change_weapon();

			break;
		}

		if( wParam == 'L' )
		{
			/*
			if( g_bCameraMove )
			{
				g_cHero.move_heroY( 0.01f );
			}
			else
			{
				t_moveClose( CAM_ADJ_SIZE*10.0f );
			}
			*/

		

			break;
		}
		if( wParam == 'K' )
		{
			delete_light();
			/*
			if( g_bCameraMove )
				g_cHero.move_heroY( -0.01f );
			else
				t_moveClose( -CAM_ADJ_SIZE*10.0f );
			*/
			break;
		}

		//
		// 存储当前的相机位置文件,在三维的背景界面中，每次退出游戏时，
		// 下一次应该使用这次存储的默认相机位置文件。
		if( wParam == 'S' )
		{
			// 切的在ini文件中的下一套动作名字。
			g_dwSkyFogDistance+=5.0f;
			/*g_iRBtnActIdx++;
			g_iRBtnActIdx %= 10;

			g_3dMgr->start_fullscrFade( 0x00ff0000,0x88ff0000,3.0f );*/

			break;
		}
		// 调入场景目录下默认的相机。
		if(wParam == 'A')
		{
			//// 如果文件存在，则调入特定的相机文件，如果文件不存在，存储当前的相机文件。
			//if( file_exist( "veryNiu.cam" ) )
			//{
			//	g_cCamera.m_pCamera->load_camera( "veryNiu.cam" );
			//	//g_bCatchScr = true;
			//	g_3dMgr->start_fullscrFade( 0xff000000,0,1.0f );
			//}
			//else
			//{
			//	g_cCamera.m_pCamera->save_camera( "veryNiu.cam" );
			//	MessageBox( NULL,"存储相机到veryNiu.cam成功","INFO",MB_OK );
			//}
			//load_tNpc();
			//move_to_fmna();
//			test_chunk();
				g_dwSkyFogDistance-=5.0f;
			break;
		}
		if (wParam == 'Z')
		{
			g_dwTerrFogDistance +=5.0f;
			break;
		}
		if (wParam == 'X')
		{
			g_dwTerrFogDistance -=5.0f;
			break;
		}

		if(wParam == 'M')
		{

			if(g_cHero.m_nState == OBJ_STATE_WALK||g_cHero.m_nState == OBJ_STATE_LEAF)
			{
				g_cHero.ChangeAction(OBJ_STATE_RUN);
			}

			//move_to_fmna();

		}

		if( wParam == '1' )
		{
			I_soundManager* pSoungMgr = get_soundManagerPtr();
			os_lisenerParam info;
			info.m_vOrientFront= osVec3D(0,0,1);
			info.m_vOrientTop = osVec3D(0,1,0);
			info.m_vPosition = osVec3D(150,150,0);

			pSoungMgr->SetListenerPara( info );
			os_initSoundSrc m_startPlayInfo;
			m_startPlayInfo.m_flMinDistance=1;
			m_startPlayInfo.m_flMaxDistance=100;
			m_startPlayInfo.m_vPosition=osVec3D(150,150,0);
			m_startPlayInfo.m_lVolume=0;
			pSoungMgr->PlayFromFile("E:\\server\\gameClient_my\\run\\sound\\nsw.wav",
				&m_startPlayInfo,1);

		}


		if( wParam == 'P' )
		{
		//	g_bCatchScr = true;
			//g_sceneMgr->save_colheiModInfo();
	//		test_herog();	
			move_to_fmna();
		}

		if( wParam == 'W' )
		{
			/*osVec3D pos;
			osVec3D look;
			osVec3D dest;
			g_gameApp.m_ptrCamera->get_curpos(&pos);
			g_gameApp.m_ptrCamera->get_godLookVec(look);
			dest = pos + look;
			g_gameApp.m_ptrCamera->set_curpos(&dest,true);

			dest = dest+ 2*look;
			g_gameApp.m_ptrCamera->set_camFocus(dest);
			g_gameApp.m_ptrCamera->get_curpos(&s_pos);
			g_gameApp.m_ptrCamera->get_godLookVec(s_look);*/
			//g_bObjWireFrame = !g_bObjWireFrame;
			//g_bTileWireFrame = !g_bTileWireFrame ;

			
	       
			/**/
			//delete_sword();
			//equip_change();

			//change_actionM

			//move_to_fmna();
		}
		break;


    case WM_SETCURSOR:
		if( g_3dMgr )
		{
			g_3dMgr->set_hwCursor();
			return TRUE ;
		}
		break;
	case 0x020A:
		on_mouseWheel( wParam,LOWORD(lParam),HIWORD(lParam) );
		break;

	case WM_LBUTTONDBLCLK:
		//g_cHero.ChangeAction(OBJ_STATE_RUN);
		MessageBox(NULL,"dd","",MB_OK);
		break;
	case WM_LBUTTONDOWN:

		
		t_pt.x = GET_X_LPARAM(lParam); 
		t_pt.y = GET_Y_LPARAM(lParam); 
		t_getTGidx( t_pt.x,t_pt.y );
		/**/

		//test_sceCursor();

	   
		break;
	case WM_RBUTTONDOWN:

# if 1
//		load_tNpc();
//		g_bRbtnDown = true;
# endif 

		::ShowCursor( FALSE );
		
	   GetCursorPos( &t_pt );
	   g_iCursorXPos = t_pt.x;
	   g_iCursorYPos = t_pt.y;

		break;
	case WM_RBUTTONUP:
		::ShowCursor( TRUE );
		::SetCursorPos( g_iCursorXPos,g_iCursorYPos );

		break;
	case WM_MOUSEMOVE:
		if( g_3dMgr )
			g_3dMgr->set_cursorPos();
		mouse_rotCam( wParam,LOWORD(lParam),HIWORD(lParam) );
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// 分析菜单选择：
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

		//! 从服务器端接收信息
	case  WSA_READ:
		{
			if( g_ptrSocket )
			{
				// 
				if (WSAGETSELECTEVENT(lParam) != FD_READ)
				{
					g_ptrSocket->close_socket();
					
					// 处理关闭socket以后的事件。
					break;
				}

				process_socket();
				break;
			}
		}


	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		process_beforeExit();
		if( g_camera )
			g_camera->save_camera( "default.cam" );
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}


