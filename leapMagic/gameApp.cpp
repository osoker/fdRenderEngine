///////////////////////////////////////////////////////////////////////////////////////////
/** \file
*  Filename: gameApp.cpp
*
*  Desc:     魔法demo最上层的类,负责引擎接口的创建,游戏消息的处理.
*            场景的渲染等上层的工作.
* 
*  His:      River created @ 2006-4-13 
* 
*/
///////////////////////////////////////////////////////////////////////////////////////////
# include "gameApp.h"
# include "gc_testScene.h"
# include "resource.h"
# include "lm_configInfoReader.h"
# include <Windowsx.h>
# include "gc_camera.h"
# include <zmouse.h>
# include "gc_charSound.h"
//# include "gc_chaEffect.h"
# include "deployMonster_mod.h"
# include "gc_creature.h"
# include "dinput.h"


extern LPDIRECT3DSURFACE9    g_pCharTarget;

gc_gameApp* g_ptrGameApp = NULL;
static gc_gameApp g_app;


os_screenPicDisp   g_scrPicDisp;

//! 设备初始化
os_deviceinit gc_gameApp::m_sDevInit;

//! 3d设备的接口指针。
I_deviceManager* gc_gameApp::m_3dMgr = NULL;

//! gameApp的场景操作指针
gc_gameScene*    gc_gameApp::m_ptrActiveScene = NULL;

//! 只有在render Loop结束时，才能真正的释放场景内的3d设备。
HANDLE           gc_gameApp::m_sRenderLoopFinish = NULL;

//! gameApp场景的声音管理器指针
I_soundManager*  gc_gameApp::m_ptrSndInterface = NULL;


//! Dinput的设置状态
I_inputMgr* gc_gameApp::m_ptrInputDevice = NULL;


// “关于”框的消息处理程序。
extern LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


/** \brief
*   用于渲染引擎选择我们需要的渲染设备。
*/
static bool cfm_callback( LPDIRECT3D9 _d3d9,D3DCAPS9* _caps,
						 ose_VertexProcessingType _vpt,D3DFORMAT _ffor,D3DFORMAT _bfor )
{

	if( _caps->DeviceType == D3DDEVTYPE_REF  )
		return false;

	//g_bWaterReflect = FALSE;
    //g_bShaderDebug = true;


	if( (_vpt != SOFTWARE_VP) &&(_vpt != MIXED_VP) )
	{
		if( _caps->VertexShaderVersion < D3DVS_VERSION(2, 0) )
			return false;
	}

	

	// 软件渲染时，有足够的blendMatrixIndex.
	// 
	/*
	// River mod 2007-10-30:
	// 使用blend matrix index主要做什么？研究明白!!!
	// 此段代码主用是人物的骨骼动画时，如果使用软件的vs会比较慢，所以．
	// 硬件的blendindex代码，目前引擎内部不再使用blendIndex,所以不需要
	// 再使用此段的代码。
	if( (_vpt != SOFTWARE_VP)&&
		(_caps->MaxVertexBlendMatrixIndex < 32) )
		return false;
	*/


	// Need to support post-pixel processing (for fog and stencil)
	if( FAILED( _d3d9->CheckDeviceFormat( _caps->AdapterOrdinal, _caps->DeviceType,
		_ffor, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
		D3DRTYPE_SURFACE, _bfor ) ) )
	{
		return false;
	}

	/*
	// 
	// 7100GS的显卡，如果使用软件顶点渲染，场景内的水面会出问题
	// DEBUG USE:使用软件顶点渲染
	if( (_vpt != SOFTWARE_VP) )
		return false;
	*/

	//g_bShaderDebug = false;

	if( g_bShaderDebug )
	{
		if( _vpt != SOFTWARE_VP )
			return false;
	}


	// test code:
	//g_bHeroShadow = false;


	return true;
}


gc_gameApp::gc_gameApp()
{
	g_ptrGameApp = this;

	m_ptrTestScene = NULL;

	m_bConsoleLastFrame = FALSE;

	m_bActive = TRUE;


}

gc_gameApp::~gc_gameApp()
{
	m_iCommonCursorId = -1;


}


//! 重设窗口的大小
void gc_gameApp::reset_screen( void )
{
	guard;

	static bool t_b = true;
	float  t_fNewAspect;
	float  t_fOldAspect;

	if( t_b )
	{
		gc_gameApp::m_3dMgr->reset_device( 1024,768,true );

		t_fNewAspect = 1024.0f/768.0f;
		t_fOldAspect = float(m_sDevInit.m_dwScrWidth)/float(m_sDevInit.m_dwScrHeight);

		m_sDevInit.m_dwScrWidth = 1024;
		m_sDevInit.m_dwScrHeight = 768;

		if( m_sDevInit.m_bFullScr )
		{
			m_sDevInit.m_bFullScr = false;
			ChangeDisplaySettings( NULL,0 );
		}

		t_b = false;
	}
	else
	{
		t_b = true;
		

		t_fNewAspect = 1280.0f/800.0f;
		t_fOldAspect = float(m_sDevInit.m_dwScrWidth)/float(m_sDevInit.m_dwScrHeight);

		m_sDevInit.m_dwScrWidth = 1280;
		m_sDevInit.m_dwScrHeight = 800;

		m_sDevInit.m_bFullScr = true;

		change_deskScreenSize();

		gc_gameApp::m_3dMgr->reset_device( 1280,800,false );



	}

	t_fNewAspect -= t_fOldAspect;
	gc_gameScene::m_ptrCamera->m_ptrCamera->set_camAspect( t_fNewAspect );
	gc_gameScene::m_ptrCamera->m_ptrCamera->set_viewport( 0,0,
		m_sDevInit.m_dwScrWidth,m_sDevInit.m_dwScrHeight,0.0f,1.0f );


	// River @ 2009-2-26:让窗口居中
	int sfx=0,sfy=0,capt=0,menu=0;
	if( !m_sDevInit.m_bFullScr )
	{
		capt = GetSystemMetrics(SM_CYCAPTION);
		sfx = GetSystemMetrics( SM_CXFIXEDFRAME );
		sfy = GetSystemMetrics( SM_CYFIXEDFRAME );
	}
	if( ::GetMenu(m_hWnd) )
		menu = GetSystemMetrics( SM_CYMENU );

	RECT rect;
	rect.left = 0;
	rect.right = gc_gameApp::m_sDevInit.m_dwScrWidth + sfx + sfy;
	rect.top = 0;
	rect.bottom = gc_gameApp::m_sDevInit.m_dwScrHeight + capt  + menu + sfx + sfy;


	int t_xOffset,t_yOffset;
	t_xOffset = GetSystemMetrics( SM_CXFULLSCREEN );
	t_yOffset = GetSystemMetrics( SM_CYFULLSCREEN );
	t_xOffset -= rect.right;
	t_xOffset /= 2;
	if( t_xOffset < 0 ) t_xOffset = 0;
	t_yOffset -= rect.bottom;
	t_yOffset /= 2;
	if( t_yOffset < 0 ) t_yOffset = 0;

	SetWindowPos( m_hWnd,HWND_TOP,t_xOffset,t_yOffset, rect.right,
		rect.bottom,
		NULL );

	unguard;
}


//! 使用这个函数修改玩家的屏幕分辩率。
bool gc_gameApp::change_deskScreenSize()
{
	guard;

	// 修改卓面分辩率.
	DEVMODE dmScreenSettings;

	// 初始化 DEVMODE 结构
	ZeroMemory(&dmScreenSettings, sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);

	//@{
	// 这段代码 是取系统的屏幕分辨率
	if (0 == EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings))
	{
		osassert( false );
		return false;
	}
	//@} 

	dmScreenSettings.dmPelsWidth = 
		m_sDevInit.m_dwScrWidth;
	dmScreenSettings.dmPelsHeight= 
		m_sDevInit.m_dwScrHeight;
	dmScreenSettings.dmBitsPerPel= 
		m_sDevInit.m_dwColorBits; 

	// syq @ 05-3-18: 在此处flag不要设CDS_FULLSCREEN，全屏影响输入法提示窗口的显示
	int ret = ChangeDisplaySettings( &dmScreenSettings,CDS_FULLSCREEN );

	if( DISP_CHANGE_SUCCESSFUL != ret )
	{
		MessageBox( NULL,"不支持的分辨率,游戏将以800*600的分辨率运行!","边缘提示",MB_OK );
		return false;
	}

	return true;

	unguard;
}

//! 重设窗口的大小和样式
void gc_gameApp::resetWindowSizeNStyle( HWND _hwnd )
{
	guard;

	DWORD  t_dwWindowStyle;

	t_dwWindowStyle = GetWindowLong( _hwnd,GWL_STYLE );
	//t_dwWindowStyle &= ~ (WS_MAXIMIZEBOX|WS_SIZEBOX);
	SetWindowLong( _hwnd, GWL_STYLE, t_dwWindowStyle );

	int sfx=0,sfy=0,capt=0,menu=0;
	if( t_dwWindowStyle & WS_CAPTION )
	{
		capt = GetSystemMetrics(SM_CYCAPTION);
		sfx = GetSystemMetrics( SM_CXFIXEDFRAME );
		sfy = GetSystemMetrics( SM_CYFIXEDFRAME );
	}
	if( ::GetMenu(_hwnd) )
		menu = GetSystemMetrics( SM_CYMENU );

	RECT rect;
	rect.left = 0;
	rect.right = gc_gameApp::m_sDevInit.m_dwScrWidth + sfx + sfy;
	rect.top = 0;
	rect.bottom = gc_gameApp::m_sDevInit.m_dwScrHeight + capt  + menu + sfx + sfy;

	// River @ 2009-2-26:让窗口居中
	int t_xOffset,t_yOffset;
	t_xOffset = GetSystemMetrics( SM_CXFULLSCREEN );
	t_yOffset = GetSystemMetrics( SM_CYFULLSCREEN );
	t_xOffset -= rect.right;
	t_xOffset /= 2;
	if( t_xOffset < 0 ) t_xOffset = 0;
	t_yOffset -= rect.bottom;
	t_yOffset /= 2;
	if( t_yOffset < 0 ) t_yOffset = 0;

	SetWindowPos( _hwnd,HWND_TOP,t_xOffset,t_yOffset, rect.right,
		rect.bottom,
		NULL );

	unguard;
}



//! 初始化当前的gameApp，在这个函数内初始化了3d设备
bool gc_gameApp::init_gameApp( HWND _hwnd,HINSTANCE _hins )
{
	guard;

	osassert( _hwnd );
	osassert( _hins );
	
	m_hIns = _hins;
	m_hWnd = _hwnd;

	//g_bWaterReflect = !g_bWaterReflect;


	m_3dMgr = ::get_deviceManagerPtr();

	g_bExportStackInfo = TRUE;

	MAX_INVIEWSM_NUM = 128;

	// 绝对不能创建正正全屏的dX窗口。
	m_sDevInit.m_bFullScr = false;

	//! TEST CODE:
	g_fTerrDetailCoef = 0.5f;

	//! 
	//g_bAlphaShelter = false;


	if( !m_3dMgr->init_device( m_hWnd,&m_sDevInit,cfm_callback ) )
	{		
		MessageBox( m_hWnd,"创建3D设备失败..","ERROR",MB_OK );
		return false;
	}

	int t_rand = 0;
	for( int t_i=0;t_i<10;t_i ++ )
		t_rand = OSRAND;



	//
	// 分配测试场景的空间
	gcs_sceneOnceInit    t_init;
	t_init.m_dwPort = 20;
	t_init.m_szIp[0] = NULL;
	t_init.m_hIns = m_hIns;
	t_init.m_hWnd = m_hWnd;
	t_init.m_iSceHeight = m_sDevInit.m_dwScrHeight;
	t_init.m_iScrWidth = m_sDevInit.m_dwScrWidth;

	gc_gameScene::init_gameScene( t_init );
	m_ptrTestScene = new gc_testScene();
	m_ptrActiveScene = (gc_gameScene*)m_ptrTestScene;

	g_pCharSoundMgr->init_charSoundMgr();

	// 初始化输入设备	
	m_ptrInputDevice = ::get_inputManagerPtr();
	osassert( m_ptrInputDevice );
	if( !m_ptrInputDevice->init_inputmgr( _hins,_hwnd ) )
		osassertex( false,"初始化输入设备失败...\n" ); 
	m_ptrInputDevice->acquire();

	
	//! 初始化声音设备
	m_ptrSndInterface = ::get_soundManagerPtr();
	osassert( m_ptrSndInterface );
	m_ptrSndInterface->PlayFileMP3( "music\\1.mp3",true );
	
	/*
	//m_ptrSndInterface->StopMP3();
	//m_ptrSndInterface->PlayFileMP3( "mp3\\2.mp3",true );

	//m_ptrSndInterface->PlayFileMP3( "mp3\\2.mp3",true );
	*/
	

	// 初始化常用的CursorId
	m_iCommonCursorId = gc_gameScene::m_ptrScene->create_cursor( "ui\\cursor\\正常.hon" );

	int id = gFontMgrPtr->RegisterFont("宋体",12,FW_LIGHT);

	//gc_gameScene::m_ptrScene->gala_initGrowScene( m_sDevInit.m_dwScrWidth,m_sDevInit.m_dwScrHeight );

	return true;

	unguard;
}

//! 重设光标信息
void gc_gameApp::reset_cursor( const char* _cursorname )
{
	guard;

	osassert( _cursorname );
	osassert( _cursorname[0] );

	// 光标文件不存在
	if( !file_exist( (char*)_cursorname ) )
		return ;

	// 不需要删除原来的光标,直接创建新的光标.
	m_iCommonCursorId = gc_gameScene::m_ptrScene->create_cursor( _cursorname );

	gc_gameScene::m_ptrScene->set_cursor( m_iCommonCursorId );

	unguard;
}


//@{
//! 要输出的调试信息
os_stringDisp gc_gameApp::m_arrDebugInfo[MAX_DEBUGINFO];
int gc_gameApp::m_iInfoNum = 0;
//@}
//@{
//! 场景内画一些示例的DebugBox
os_bbox gc_gameApp::m_arrDebugBox[MAX_DEBUGINFO];
int     gc_gameApp::m_iDebugBoxNum = 0;
//@}


//! 推进要渲染的DebugInfo.
void gc_gameApp::push_debugInfo( const char* _info,int _x,int _y,DWORD _color )
{
	guard;

	if( m_iInfoNum >= MAX_DEBUGINFO )
		return;

	m_arrDebugInfo[m_iInfoNum] = _info;
	m_arrDebugInfo[m_iInfoNum].x = _x;
	m_arrDebugInfo[m_iInfoNum].y = _y;
	m_arrDebugInfo[m_iInfoNum].color = _color;
	m_arrDebugInfo[m_iInfoNum].charHeight = 24;

	m_iInfoNum ++;

	unguard;
}

//! 推进要渲染的DebugBBox
void gc_gameApp::push_debugBox( os_bbox& _bbox )
{
	guard;

	if( m_iDebugBoxNum>= MAX_DEBUGINFO )
		return;

	m_arrDebugBox[m_iDebugBoxNum] = _bbox;

	m_iDebugBoxNum ++;

	return;
	
	unguard;
}


//! 在屏幕上渲染输出一些调试信息
void gc_gameApp::render_debugInfo( void )
{
	guard;

	osassert( gc_gameScene::m_ptrScene );

	if( lm_gvar::g_bRenderDebugInfo )
	{
		gc_gameScene::m_ptrScene->disp_string( 
			m_arrDebugInfo,m_iInfoNum ,0);
	}
	

	m_iInfoNum = 0;

	unguard;
}

//! 处理鼠标控制相机的旋转
void gc_gameApp::process_camRotPitch( const os_dInputdata* _idata )
{
	guard;

	lm_command   t_cmd;

	if( !m_ptrActiveScene->is_renderEnable() )
		return;

	static	int	t_iMouseX	= _idata->mousedata.lx;
	static	int	t_iMouseY	= _idata->mousedata.ly;

	// 如果右键按下:
	if( _idata->mousedata.rgbButtons[1] & 0x80  )
	{
		int      t_iOffsetY,t_iOffsetX;
		float    t_fRadian;

		t_iOffsetX = _idata->mousedata.lx;// - t_iMouseX;
		t_iOffsetY = _idata->mousedata.ly;// - t_iMouseY;

		t_cmd.m_dwCmdContentSize = sizeof( float );

		//竖直旋转屏幕牧
		if( t_iOffsetY != 0 )
		{
			t_fRadian = t_iOffsetY*gc_camera::m_fRotSpeed;
			t_cmd.m_eCommandId = LM_CAMERAPITCH;
			t_cmd.m_ptrCommandContent = &t_fRadian;

			lmc_commandMgr::Instance()->do_commandImm( t_cmd );
		}

		//水平旋转屏幕
		if( t_iOffsetX != 0 )
		{
			t_fRadian = t_iOffsetX*gc_camera::m_fRotSpeed;
			t_cmd.m_eCommandId = LM_CAMERAROT;
			t_cmd.m_ptrCommandContent = &t_fRadian;

			lmc_commandMgr::Instance()->do_commandImm( t_cmd );
		}
	}

	// 记录最后的鼠标位置.
	t_iMouseX	= _idata->mousedata.lx;
	t_iMouseY	= _idata->mousedata.ly;

	unguard;
}


//! 处理手工的相机控制
void gc_gameApp::process_customCamControl( const os_dInputdata* _idata )
{
	guard;

	lm_command   t_cmd;

	// 处于人物跟随状态，则不处理
	if( lm_gvar::g_bCamFollowHero )
		return;
# if 1
	float   t_fEletime = sg_timer::Instance()->get_lastelatime();
	float   t_f = lm_gvar::g_fCamMoveSpeed * t_fEletime;
	float   t_fNeg = -t_f;

	if( OS_KEYDOWN( _idata->keydata,DIK_Q) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVEUP;
		t_cmd.m_ptrCommandContent = &t_f;

		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}

	if( OS_KEYDOWN( _idata->keydata,DIK_E) ) // End
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVEUP;
		t_cmd.m_ptrCommandContent = &t_fNeg;

		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}

	if( OS_KEYDOWN( _idata->keydata,DIK_W) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVEFORWORD;
		t_cmd.m_ptrCommandContent = &t_f;

		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}
	if( OS_KEYDOWN( _idata->keydata,DIK_S) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVEFORWORD;
		t_cmd.m_ptrCommandContent = &t_fNeg;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}
	if( OS_KEYDOWN( _idata->keydata,DIK_A) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVERIGHT;
		t_cmd.m_ptrCommandContent = &t_fNeg;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}
	if( OS_KEYDOWN( _idata->keydata,DIK_D ) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVERIGHT;
		t_cmd.m_ptrCommandContent = &t_f;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}

	// 拉近和放远
	if( OS_KEYDOWN( _idata->keydata,DIK_B) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVECLOSEF;
		t_cmd.m_ptrCommandContent = &t_f;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}
	if( OS_KEYDOWN( _idata->keydata,DIK_N) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_MOVECLOSEF;
		t_cmd.m_ptrCommandContent = &t_fNeg;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}

	// YawLeft和YawRight.
	float   t_fRot = t_fEletime*gc_camera::m_fRotSpeed;
	t_fRot *= (lm_gvar::g_fCamMoveSpeed*20);
	float   t_fRotNeg = -t_fRot;

	if( OS_KEYDOWN( _idata->keydata,DIK_T) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_CAMERAROT;
		t_cmd.m_ptrCommandContent = &t_fRot;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}
	if( OS_KEYDOWN( _idata->keydata,DIK_Y) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_CAMERAROT;
		t_cmd.m_ptrCommandContent = &t_fRotNeg;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}

	// PitchUp和PitchDown.
	if( OS_KEYDOWN( _idata->keydata,DIK_G) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_CAMERAPITCH;
		t_cmd.m_ptrCommandContent = &t_fRot;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}
	if( OS_KEYDOWN( _idata->keydata,DIK_H) )
	{
		t_cmd.m_dwCmdContentSize = sizeof( float );
		t_cmd.m_eCommandId = LM_CAMERAPITCH;
		t_cmd.m_ptrCommandContent = &t_fRotNeg;
		lmc_commandMgr::Instance()->do_commandImm( t_cmd );
	}

# endif 	


	return;

	unguard;
}


//! 在场景内渲染我们加入的bbox盒子，用于一个调试信息的示例
void gc_gameApp::render_debugBox( void )
{
	guard;

	osassert( gc_gameScene::m_ptrScene );
	for( int t_i=0;t_i<m_iDebugBoxNum;t_i ++ )
		gc_gameScene::m_ptrScene->draw_bbox( &m_arrDebugBox[t_i],0xffff0000 );

	return;

	unguard;
}





/** \brief
*  对当前的app进行渲染的函数.
*/
bool gc_gameApp::render_gameApp( void )
{
	guard;

	//osColor t_color = new osColor();
	int t_i = sizeof( D3DXCOLOR );

	g_bRenderTerr = true;

	sg_timer::Instance()->framemove();

	// 如果处于时间停止状态
	if( lm_gvar::g_bSuspendTime )
		sg_timer::Instance()->suspend_lastFrameTime();

	/*
	// TEST CODE:
	osDebugOut( ".............上一帧的帧速为:<%f>...\n",
		sg_timer::Instance()->get_fps() );
	*/
	
	/* River @ 2007-3-12:怪物过多后，声音播放的速度也是一个比较大的问题
	g_pCharSoundMgr->framemove_charSoundMgr();
	*/	


	// 只有处于激活状态，才接受dinput的输入
	const os_dInputdata*   t_ptrData;
	if( m_bActive )
	{
		t_ptrData = m_ptrInputDevice->get_inputdata();
		process_customCamControl( t_ptrData );
		process_camRotPitch( t_ptrData );
	}
	else
		::Sleep( 80 );


	// 处理自动命令队列
	lmc_commandMgr::Instance()->
		frame_moveAutoCmd( sg_timer::Instance()->get_lastelatime() );


	// 如果上一帧弹出conole,则设置上一帧过去的时间为零 
	if( m_bConsoleLastFrame )
	{
		m_bConsoleLastFrame = FALSE;
		sg_timer::Instance()->suspend_lastFrameTime();
	}


	// 处理命令队列内的命令
	process_cmdQueue();


	g_bFadeEnable = true;


	//
	// 清空设备并渲染场景到屏幕。
	m_3dMgr->d3d_clear( 0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 
		0x0/*g_dwFarTerrColor*/, 1.0f, 0L );

	m_3dMgr->d3d_begindraw();

# if 1
	if( m_ptrActiveScene->is_renderEnable() )
	{
		m_ptrActiveScene->render_scene();
	}


	// 推入帧速信息
#if __COLL_MOD__
	gc_gameApp::push_debugInfo( 
		va( "地形碰撞修改说明:\n点中键或 [SHIFT]+左键可修改单个点碰撞信息.\n点[Z][X]可修改整体碰撞信息\n \
		点[S]键保存碰撞信息到文件中\n FPS is: <%f> ",sg_timer::Instance()->get_fps() ),
		500,10,0xffffffff );
#else
	gc_gameApp::push_debugInfo( 
		va( "FPS is: <%f> ",sg_timer::Instance()->get_fps() ),
		500,10,0xffffffff );
#endif

	// 屏幕上显示一些调试信息
	render_debugInfo();

	/*
	// TEST CODE:
	//@{ 
	// WORK START:
	// 在此处加入错误字符串的显示：t_str = "<&ffffff:裡停留的&>";显示的宽度是36,高度200.
	os_colorInterpretStr    t_szDebug;
	t_szDebug.charHeight = 12;
	t_szDebug.m_bForceDis = true;
	t_szDebug = "<&ffffff:里停留的&>";
	t_szDebug.m_iMaxPixelX = 36;
	t_szDebug.m_iMaxPixelY = 400;
	t_szDebug.m_iPosX = 0;
	t_szDebug.m_iPosY = 300;

	gc_gameScene::m_ptrScene->disp_string( &t_szDebug );
	//@}
	*/
# endif 

	m_3dMgr->d3d_enddraw();
	

	//! 全局的载屏处理
	if( lm_gvar::g_bCatchScreen )
	{
		m_3dMgr->catch_screen();
		lm_gvar::g_bCatchScreen = false;
	}

	m_3dMgr->d3d_present();

	return true;

	unguard;
}

//! 立即执行一条命令
void gc_gameApp::do_commandImm( lm_command* _cmd )
{
	guard;

	osassert( _cmd );

	m_ptrTestScene->process_command( *_cmd );

	unguard;
}



//! 处理全局命令队列内的命令
void gc_gameApp::process_cmdQueue( void )
{
	guard;

	lmc_commandMgr*   t_ptrCmdMgr = lmc_commandMgr::Instance();
	lm_command*       t_ptrCmd;

	int t_iCmdNum = t_ptrCmdMgr->get_cmdNum();

	// 
	// 在处理命令的过程中，命令数目可能会动态增加，所以每次循环一定要实时得到命令数目
	for( int t_i=0;t_i<t_ptrCmdMgr->get_cmdNum();t_i ++ )
	{
		t_ptrCmd = t_ptrCmdMgr->get_cmdIdx( t_i );
		m_ptrTestScene->process_command( *t_ptrCmd );
	}

	t_ptrCmdMgr->clear_command();

	unguard;
}

//! 处理键盘按下的消息
void gc_gameApp::process_keyDownMsg( UINT _uMsg, WPARAM _wParam, LPARAM _lParam )
{
	guard;

	lm_command   t_cmd;
	static BOOL  t_sbRain = FALSE;	
	static osVec3D t_vec3StaticPos;
	osVec3D      t_vec3Pos;

	// 
	osDebugOut( "The key down is<%d>...\n",_wParam );

	// 支持中文的"｀"键.
	if( (_wParam == 192)||(_wParam == 229) )// '~'键
	{

		m_bConsoleLastFrame = TRUE;
		DialogBox(m_hIns, (LPCTSTR)IDD_ABOUTBOX,m_hWnd, (DLGPROC)About);
	}

	static int t_osaId = -1;


	// x键控制是否网格状态
	if( _wParam == 'X' )
	{
		//reset_screen();
		
		//g_bTileWireFrame = !g_bTileWireFrame;

		/*
		static bool t_bHdr = false;
		static bool t_bInit = false;
		if( !t_bInit )
		{
			gc_gameScene::m_ptrScene->gala_initGrowScene( 1024,768 );
			t_bInit = true;
		}
		t_bHdr = !t_bHdr;
		if( gc_gameScene::m_ptrScene )
			gc_gameScene::m_ptrScene->gala_actGrowScene( t_bHdr );
			*/


/*
		// TEST CODE:
		BYTE*	t_ptrStart = START_USEGBUF;
		int t_i;
		t_i = ::read_fileToBuf( (char*)"xj.xml", t_ptrStart, TMP_BUFSIZE );
		if( t_i <= 0 )
		{
			END_USEGBUF;
			return ;
		}
		t_ptrStart[t_i] = 0;
		TiXmlDocument xmlDoc;
		xmlDoc.Parse( (const char*)t_ptrStart );
		END_USEGBUF;
		osassert( !xmlDoc.Error() );

		TiXmlElement* t_ptrEle = xmlDoc.RootElement();
*/

		/*
		RECT  t_rect;
		::GetClientRect( m_hWnd,&t_rect );

		gc_camera::m_ptrCamera->set_viewport( 0,0,t_rect.right,t_rect.bottom,0.0f,1.0f );

		g_bObjWireFrame = !g_bObjWireFrame;
		g_bCharWireFrame = !g_bCharWireFrame;
		*/


		static int t_iEffId = -1;

# if 0
		// River @ 2006-8-12:不删除特效，用于测试最在场景内加入更多的特效:
		t_vec3Pos.x = lm_gvar::g_iHeroPosX*1.5f;
		t_vec3Pos.z = lm_gvar::g_iHeroPosZ*1.5f;
		gc_gameScene::m_ptrScene->get_detailPos( t_vec3Pos,true );
		t_vec3Pos.y += 1.5f;

		static t_bFirst = false;


		D3DXVECTOR3  t_v[3];
		D3DXVECTOR3  t_s,t_e;
		float        tu,tv,t_dis;

		t_v[0] = D3DXVECTOR3( 0.0f,0.0f,0.0f );
		t_v[1] = D3DXVECTOR3( 1.0f,0.0f,0.0f );
		t_v[2] = D3DXVECTOR3( -1.0f,1.0f,0.0f );

		osVec3Cross( &t_s,&(t_v[1]-t_v[0]),&(t_v[2]-t_v[0]) );

		float t_f = osVec3Dot( &t_v[1],&t_v[2] );

		t_s = D3DXVECTOR3( 0.1f,0.1f,-0.8f );
		t_e = D3DXVECTOR3( 0.0f,0.0f,1.0f );

		BOOL t_b = D3DXIntersectTri( &t_v[0],&t_v[2],
			&t_v[1],&t_s,&t_e,&tu,&tv,&t_dis );

		if( t_bFirst == false )
		{
			
			//t_iEffId = gc_gameScene::m_ptrScene->create_effect( 
			//	"speff\\ZS_nuZhanMengJi.spe",t_vec3Pos,4.8f,true );
			t_iEffId = gc_gameScene::m_ptrScene->create_effect( 
				"ZS_HLZN.spe",t_vec3Pos,1.0f,true );

			/*

			*/
			//t_bFirst = true;

			float t_f = gc_gameScene::m_ptrScene->get_effectKeyTime( t_iEffId );

			osVec3D  t_vec3;
			osVec3D  t_vec3Size;
			bool t_b = gc_gameScene::m_ptrScene->get_effectBox( t_iEffId,t_vec3,t_vec3Size );

			lm_gvar::g_iEffectId = t_iEffId;

			int t_k = 0;
			/**/
			//t_iEffId = gc_gameScene::m_ptrScene->create_billBoard( 
			//	"efftexture\\0001.BB",t_vec3Pos );

		}
		else
		{
			/*
			gc_gameScene::m_ptrScene->delete_effect( t_iEffId );
			*/
			gc_gameScene::m_ptrScene->delete_billBoard( t_iEffId );

			t_iEffId = -1;
			t_bFirst = false;
		}		

# endif 

		/*
		{
			
			float t_fTime = 0;
			if( !gc_gameScene::m_ptrScene->validate_effectId( t_iEffId ) )
				t_iEffId = gc_gameScene::m_ptrScene->create_effect(
				"speff\\j_wuleihongding.spe",t_vec3Pos,0.0f,TRUE,1.0f,&t_fTime );
			else
			{
				static bool t_b = true;

				if( t_b )
				{
					gc_gameScene::m_ptrScene->set_effectAlpha( t_iEffId,0.5f );
					t_b = false;
				}
				else
				{
					gc_gameScene::m_ptrScene->set_effectAlpha( t_iEffId,1.0f );
					t_b = true;
				}

			}

		}
		*/



		/*
		os_particleCreate  t_par;
		strcpy( t_par.m_szParticleName,"particle\\buzuoPar_01.fdp" );
		t_par.m_bCreateIns = TRUE;
		t_par.m_fSizeScale = 1.0f;



		t_par.m_vec3Offset.x = lm_gvar::g_iHeroPosX*1.5f;
		t_par.m_vec3Offset.z = lm_gvar::g_iHeroPosZ*1.5f;
		gc_gameScene::m_ptrScene->get_detailPos( t_par.m_vec3Offset,true );
		t_iEffId = gc_gameScene::m_ptrScene->create_particle( t_par );
		*/
		
		
		// 创建osa测试动画的代码
		
# if 1		
		os_aniMeshCreate    t_osa;

		t_osa.m_bAutoDelete = false;
		t_osa.m_bCreateIns = true;
		t_osa.m_bEffectByAmbiL = false;
		t_osa.m_ePlayType = OSE_PLAYFOREVER;//;OSE_PLAYWITHTIME//
		t_osa.m_fPlayTime = 0.35f;
		t_osa.m_iPlayFrames = 1;
		t_osa.m_fAngle = 0.0f;
		strcpy( t_osa.m_szAMName,"keyani\\bugtest.OSA" );
		t_osa.m_vec3Pos.x = lm_gvar::g_iHeroPosX*1.5f;
		t_osa.m_vec3Pos.z = lm_gvar::g_iHeroPosZ*1.5f;
		t_osa.m_fSizeScale = 2;

		t_osa.m_vec3Pos.y = 0.0f;
# else		
		gc_gameScene::m_ptrScene->get_detailPos( t_osa.m_vec3Pos,true );
# endif 
		t_osa.m_vec3Pos.y += 2.0f;
		//t_osa.m_fSizeScale = 0.1f;

		t_osa.m_bCreateIns = false;
		//gc_gameScene::m_ptrScene->create_osaAmimation( &t_osa );
		t_osa.m_bCreateIns = true;

       
		if( t_osaId == -1 )
		{
			t_osaId = gc_gameScene::m_ptrScene->create_osaAmimation( &t_osa );
			//gc_gameScene::m_ptrScene->scale_aniMesh( t_osaId,6.0f,0.35f );		
		}
		else
		{
			gc_gameScene::m_ptrScene->delete_animationMesh( t_osaId );
			t_osaId = -1;
		}
	     /**/

		//t_vec3StaticPos = t_osa.m_vec3Pos;
		/**/

		/*
		static bool  t_b = true;

		if( t_b )
		{
			gc_gameScene::m_ptrCamera->m_ptrCamera->limit_pitch( (30/180.0f)*OS_PI );
			t_b = false;
		}
		else
		{
			t_b = true;
			gc_gameScene::m_ptrCamera->m_ptrCamera->unlimit_pitch();
		}



		/*
		osVec3D   t_vec3( 127.142f,0.0f,113.804f );
		gc_gameScene::m_ptrScene->get_detailPos( t_vec3,true );
		
		gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();
		static bool t_b = true;
		t_ptrHero->get_skinPtr()->set_fadeState( t_b,0.6f );
		t_b = !t_b;*/

		/*
		if( g_bTileWireFrame )
			gc_gameScene::m_ptrCamera->m_ptrCamera->set_farClipPlane( 280.0f );
		else
			gc_gameScene::m_ptrCamera->m_ptrCamera->set_farClipPlane( 180.0f );
		*/


	}
	else if( _wParam == 'C' )
	{

		/*
		static bool t_b = true;
		static LONGLONG  t_pos = 0;

		if( t_b == true )
		{
			m_ptrSndInterface->ToggleMuteMP3( true );
			t_pos = m_ptrSndInterface->GetMp3PlayPosition();
			m_ptrSndInterface->PlayFileMP3( "music\\test.mp3" );
			t_b = false;
		}
		else
		{
			m_ptrSndInterface->ToggleMuteMP3( false );
			m_ptrSndInterface->PlayFileMP3( "music\\1.mp3" );
			m_ptrSndInterface->SetMp3PlayPosition( t_pos );
			t_b = true;
		}
		*/
	


		/*
		gc_gameScene::m_ptrScene->gala_initGrowScene( 1024,768 );
		static bool t_btruefalse = true;
		gc_gameScene::m_ptrScene->gala_actGrowScene( t_btruefalse );
		if( t_btruefalse )
			t_btruefalse = false;
		else
			t_btruefalse = true;
		*/
		/*
		lm_gvar::g_bDisplayColInfo = (!lm_gvar::g_bDisplayColInfo);
		if( !lm_gvar::g_bDisplayColInfo )
			m_ptrActiveScene->reset_colDecal();

		g_bHeroShadow = !g_bHeroShadow;
		*/
		// 创建一个renderTarget.
		//! 得到当前skinMesh的身体部位多边形数目。
# if __GLOSSY_EDITOR__
		gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();
		int t_k = 0;
		for( int t_i = 0;t_i<8;t_i ++ )
			t_k = t_ptrHero->get_skinPtr()->get_bodyTriNum( t_i );

		t_k = 0;

# endif 


	}
# if __DEPLOY_MONSTERMOD__
	else if( _wParam == 'D' )
	{
		if( m_ptrActiveScene )
			m_ptrActiveScene->delete_selMonster();
	}
	// 键盘左边的Ctrl键.
	else if( _wParam == 'Q' )
	{
		/*
		lm_gvar::g_bDisplayAllMonInfo = !lm_gvar::g_bDisplayAllMonInfo;
		*/
		t_vec3StaticPos.x += 0.03f;
		gc_gameScene::m_ptrScene->set_aniMeshPos( t_osaId,t_vec3StaticPos );

	}
	// 怪物的四种矩离显示框.
	else if( _wParam == 'W' )
	{
		lm_gvar::g_bDisTrigger = !lm_gvar::g_bDisTrigger;
	}
	else if( _wParam == 'E' )
	{
		lm_gvar::g_bDisVision = !lm_gvar::g_bDisVision;
	}
	else if( _wParam == 'R' )
	{
		lm_gvar::g_bDisCall = !lm_gvar::g_bDisCall;
	}
	else if( _wParam == 'T' )
	{
		lm_gvar::g_bDisRun = !lm_gvar::g_bDisRun;
	}
	else if( _wParam == 'S' )
		g_bDrawCreatureBBox = !g_bDrawCreatureBBox;
# endif 
	else if( _lParam == 'C' )
		g_bObjWireFrame = !g_bObjWireFrame;
	else if( _lParam == 'V' )
        g_bRenderCollision = !g_bRenderCollision;		


# if 1
	// 测试场景下雨相关的数据
	if( _wParam == 'R' )
	{
		static t_sbRain = false;
		t_sbRain = !t_sbRain;
		if( t_sbRain )
		{
			os_rainStruct  t_rs;
		
			t_rs.m_fRainMoveSpeed /= 2.0f;
			t_rs.m_iRainDotNum = 500;

			gc_gameScene::m_ptrScene->start_sceneRain( &t_rs );
		}
		else
			gc_gameScene::m_ptrScene->stop_sceneRain();

	}
# endif 


	// 截屏功能
	if( _wParam == '0' )
	{
		/*
		//lm_gvar::g_bCatchScreen = true;
		gc_testScene* t_ptrScene = (gc_testScene*)m_ptrActiveScene;
		t_ptrScene->change_fightScene();
		*/

		gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();
		t_ptrHero->get_skinPtr()->scale_skinMesh( osVec3D(5.0f,5.0f,5.0f) );


		/*
		t_ptrHero->get_skinPtr()->playSwordGlossy( true,true );
		os_equipment   t_s;
		t_s.m_iCPartNum = 1;
		t_s.m_arrId[0] = 7;
		t_s.m_arrMesh[0] = 2;
		t_s.m_arrSkin[0] = 1;
		*/

		/*
		t_s.m_arrId[1] = 3;
		t_s.m_arrMesh[1] = 5;
		t_s.m_arrSkin[1] = 1;
		*/

		//t_ptrHero->get_skinPtr()->change_equipment( t_s );

	}

	//! windy add Shift + 左键
	if( _wParam == 'z'||_wParam == 'Z'  )
	{
		/*
		if( !this->m_ptrActiveScene ) return;
		if( !this->m_ptrActiveScene->is_renderEnable() ) return;
		BOOL bPass = TRUE;
		

		os_tileIdx    t_sMousePos;

		t_sMousePos.m_iX = GET_X_LPARAM( _lParam ); 
		t_sMousePos.m_iY = GET_Y_LPARAM( _lParam );

		t_cmd.m_eCommandId = LM_SCENE_CONTROLDOWN;
		t_cmd.m_dwCmdContentSize = sizeof( BOOL );
		t_cmd.m_ptrCommandContent = &bPass;

		lmc_commandMgr::Instance()->push_command( t_cmd );
		*/
		//gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();

		//t_ptrHero->get_skinPtr()->change_weaponEffect( 1,I_skinMeshObj::WEAPON8 );

	}
	if( _wParam == 'x'||_wParam == 'X'  )
	{
		/*
		if( !this->m_ptrActiveScene ) return;
		if( !this->m_ptrActiveScene->is_renderEnable() ) return;
		BOOL bPass = FALSE;

		os_tileIdx    t_sMousePos;

		t_sMousePos.m_iX = GET_X_LPARAM( _lParam ); 
		t_sMousePos.m_iY = GET_Y_LPARAM( _lParam );

		t_cmd.m_eCommandId = LM_SCENE_CONTROLDOWN;
		t_cmd.m_dwCmdContentSize = sizeof( BOOL );
		t_cmd.m_ptrCommandContent = &bPass;

		lmc_commandMgr::Instance()->push_command( t_cmd );
		
		//gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();

		g_scrPicDisp.m_bUseZBuf = false;
		g_scrPicDisp.m_scrVer[0].m_vecPos.x = 0.0f;
		g_scrPicDisp.m_scrVer[0].m_vecPos.y = 0.0f;

		g_scrPicDisp.m_scrVer[1].m_vecPos.x = 30.0f;
		g_scrPicDisp.m_scrVer[1].m_vecPos.y = 0.0f;

		g_scrPicDisp.m_scrVer[2].m_vecPos.x = 30.0f;
		g_scrPicDisp.m_scrVer[2].m_vecPos.y = 0.0f;

		g_scrPicDisp.m_scrVer[3].m_vecPos.x = 0.0f;
		g_scrPicDisp.m_scrVer[3].m_vecPos.y = 30.0f;

		g_scrPicDisp.tex_id = gc_gameScene::m_ptrScene->create_scrTexture( "ui\\Spirit.tga" );
		int t_m = 0;*/
		//D3DXSaveSurfaceToFile( "d:/test.tga",D3DXIFF_TGA,g_pCharTarget,NULL,NULL );

	}
	if( _wParam == 's'||_wParam == 'S'  )
	{
	/*
# if __COLL_MOD__
		m_ptrActiveScene->m_ptrScene->save_colheiModInfo();
# endif 
	*/
		//g_bDrawCreatureBBox = !g_bDrawCreatureBBox;


		//lm_gvar::g_bDisplayCharInRect = !lm_gvar::g_bDisplayCharInRect;
		//g_bAlphaShelter = false;

		gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();

    /*
    5:  环境纹理.     ok
	6:  流动纹理.     ok
    7:  自发光纹理.   ok
	8:  流动与环境相结合纹理    
	9:  流动与自发光相结合纹理.
	10: 环境与自发光相结合纹理.
	*/
		os_chaSecEffStruct  t_sEff;
		t_ptrHero->get_skinPtr()->get_chaSecEffect( &t_sEff,2 );
		t_sEff.m_wEffectType = 7;
		t_sEff.m_fRotSpeed = 2.0f;
		strcpy( t_sEff.m_szSecTexName,"character\\secTexture\\charShine.hon");
		t_sEff.m_szThdTexName[0] = NULL;
		t_sEff.m_szFourthTexName[0] = NULL;
		//strcpy( t_sEff.m_szThdTexName,"character\\secTexture\\charShine.hon");
		//strcpy( t_sEff.m_szFourthTexName,"character\\secTexture\\charShine.hon");

		t_ptrHero->get_skinPtr()->update_chaSecEffect( &t_sEff,2 );
		t_ptrHero->get_skinPtr()->save_chaSecEffect( 2 );
		int t_m = 0;

/*
		static int t_iSett = 0;

		const I_weaponEffect* t_ptrEff;

		HALF_FADETIME = 0.0f;

# if __GLOSSY_EDITOR__	
		if( t_iSett == 2 )
		{
			t_ptrHero->get_skinPtr()->change_weaponEffect( 3 );
			t_ptrEff = t_ptrHero->get_skinPtr()->get_weaponEffect( 3 );
			t_iSett ++;
		}
		if( t_iSett == 1 )
		{
			t_ptrHero->get_skinPtr()->change_weaponEffect( 2 );
			t_ptrEff = t_ptrHero->get_skinPtr()->get_weaponEffect( 2 );
			t_iSett ++;
		}
		if( t_iSett == 0 )
		{
			t_ptrHero->get_skinPtr()->change_weaponEffect( 1 );
			t_ptrEff = t_ptrHero->get_skinPtr()->get_weaponEffect( 1 );
			t_iSett ++;
		}
# endif 
*/

	}

	if( _wParam == 'b' || _wParam == 'B' )
	{
		gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();

		static int t_iSet = 0;
		I_weaponEffect  t_szEffect;
		const I_weaponEffect* t_ptrEff;


		t_ptrHero->get_skinPtr()->playSwordGlossy( true,true );
		os_equipment   t_s;
		t_s.m_iCPartNum = 1;
		t_s.m_arrId[0] = 6;
		t_s.m_arrMesh[0] = 10;
		t_s.m_arrSkin[0] = 1;


		t_ptrHero->get_skinPtr()->change_equipment( t_s );
# if __GLOSSY_EDITOR__	
		if( t_iSet == 3 )
		{
			t_ptrHero->get_skinPtr()->save_weaponFile();

			t_ptrEff = t_ptrHero->get_skinPtr()->get_weaponEffect( 3 );

			t_iSet ++;
		}

		if( t_iSet == 2 )
		{
			strcpy( t_szEffect.m_szEffObjName[0],"keyAni\\nan2000_7_03_eff.OSA" );
			strcpy( t_szEffect.m_szEffObjName[1],"keyAni\\nan2000_7_04_eff.OSA" );
			t_ptrHero->get_skinPtr()->set_weaponEffect( 3,t_szEffect );
			t_ptrEff = t_ptrHero->get_skinPtr()->get_weaponEffect( 2 );
			t_iSet ++;
		}

		if( t_iSet == 1 )
		{
			strcpy( t_szEffect.m_szEffObjName[0],"" );
			t_ptrHero->get_skinPtr()->set_weaponEffect( 2,t_szEffect );
			t_ptrEff = t_ptrHero->get_skinPtr()->get_weaponEffect( 1 );
			t_iSet ++;
		}

		if( t_iSet == 0 )
		{
			strcpy( t_szEffect.m_szEffObjName[0],"keyAni\\nan2000_7_02_eff.OSA" );
			strcpy( t_szEffect.m_szEffObjName[1],"particle\\smoke.fdp" );

			
			// 测试加入人物的武器特效。
			t_ptrHero->get_skinPtr()->set_weaponEffect( 1,t_szEffect,I_skinMeshObj::WEAPON7 );
			t_ptrHero->get_skinPtr()->scale_weaponEffect( 0,25.0f );
			t_ptrHero->get_skinPtr()->scale_weaponEffect( 1,0.15f );

			/*
			t_ptrHero->get_skinPtr()->set_sgEffect( 0.1,t_szEffect );
			*/

			t_ptrEff = t_ptrHero->get_skinPtr()->get_weaponEffect( 0 );
			t_iSet ++;
		}

		if( t_iSet == 0 )
		{
		}
# endif 

	}


# if __GLOSSY_EDITOR__	
	gc_hero*  t_ptrHero = (gc_hero*)gc_gameScene::m_ptrCreatureMgr->get_heroEntity();
	if( t_ptrHero )
	{
		osVec3D   t_vec3Offset = osVec3D( 0.0f,0.0f,0.0f );
		if( _wParam == VK_LEFT )
			t_vec3Offset.x = 0.02f;
		if( _wParam == VK_RIGHT )
		{
			t_vec3Offset.x = -0.02f;
		}
		if( _wParam == VK_UP )
		{
			t_vec3Offset.z = 0.02f;
		}
		if( _wParam == VK_DOWN )
		{
			t_vec3Offset.z = -0.02f;
		}
		if( _wParam == VK_HOME )
		{
			t_vec3Offset.y = 0.02f;
		}
		if( _wParam == VK_END )
		{
			t_vec3Offset.y = -0.02f;
		}

		t_ptrHero->add_ridePosOffset( t_vec3Offset );
		if( _wParam == VK_SPACE )
		{
			t_ptrHero->save_rideOffset();
			MessageBox( NULL,"存储马背位置成功...\n","SUCCESS",MB_OK );
		}
	}
# endif 


	unguard;
}


//! 处理鼠标相关的消息
void gc_gameApp::process_mouseMsg( UINT _uMsg, WPARAM _wParam, LPARAM _lParam )
{
	guard;

	lm_command   t_cmd;

	// Mouse wheel msg
	if( WM_MOUSEWHEEL == _uMsg )
	{
		int t_iDelta = ( ( short ) HIWORD( _wParam ) ) / WHEEL_DELTA;;
		t_cmd.m_dwCmdContentSize = sizeof( int );
		t_cmd.m_eCommandId = LM_MOVECLOSE;
# if __DEPLOY_MONSTERMOD__
		if( m_ptrActiveScene->get_curActiveArea() >= 0 )
			t_iDelta *= 30;
# endif 
		t_cmd.m_ptrCommandContent = &t_iDelta;
		lmc_commandMgr::Instance()->push_command( t_cmd );
	}

	//! control键按下.
	if( (_wParam & MK_CONTROL) )
		lm_gvar::g_bCtrlDown = TRUE;
	else
		lm_gvar::g_bCtrlDown = FALSE;

	//@{
	// 处理鼠标右键事件
	static	int	t_iMouseX	= LOWORD(_lParam);
	static	int	t_iMouseY	= HIWORD(_lParam);

	//! 如果仅仅是鼠标的移动事件,在场景内处理鼠标的移动
	if( m_ptrActiveScene )
	{
		if( (_uMsg == WM_MOUSEMOVE) && (m_ptrActiveScene->is_renderEnable())&&
			(_wParam != MK_RBUTTON) && (_wParam != MK_LBUTTON) )
		{
			if( !this->m_ptrActiveScene ) return;
			if( !this->m_ptrActiveScene->is_renderEnable() ) return;

			os_tileIdx    t_sMousePos;

			t_sMousePos.m_iX = GET_X_LPARAM( _lParam ); 
			t_sMousePos.m_iY = GET_Y_LPARAM( _lParam );

			t_cmd.m_eCommandId = LM_SCENE_MOUSEMOVE;
			t_cmd.m_dwCmdContentSize = sizeof( os_tileIdx );
			t_cmd.m_ptrCommandContent = &t_sMousePos;

			lmc_commandMgr::Instance()->push_command( t_cmd );		

		}
	}

	// 如果鼠标移动是 鼠标的右键处于按下的状态
# if 1 // 使用DInput来处理鼠标的旋转?
	if( _wParam == MK_RBUTTON && m_ptrActiveScene->is_renderEnable() )
	{
		int      t_iOffsetY,t_iOffsetX;
		float    t_fRadian;

		t_iOffsetX = LOWORD(_lParam) - t_iMouseX;
		t_iOffsetY = HIWORD(_lParam) - t_iMouseY;

		t_cmd.m_dwCmdContentSize = sizeof( float );

		//竖直旋转屏幕牧
		if( t_iOffsetY != 0 )
		{
			t_fRadian = t_iOffsetY*gc_camera::m_fRotSpeed;
			t_cmd.m_eCommandId = LM_CAMERAPITCH;
			t_cmd.m_ptrCommandContent = &t_fRadian;

			lmc_commandMgr::Instance()->do_commandImm( t_cmd );
		}

		//水平旋转屏幕
		if( t_iOffsetX != 0 )
		{
			t_fRadian = t_iOffsetX*gc_camera::m_fRotSpeed;
			t_cmd.m_eCommandId = LM_CAMERAROT;
			t_cmd.m_ptrCommandContent = &t_fRadian;

			lmc_commandMgr::Instance()->do_commandImm( t_cmd );
		}
	}
# endif 

	// 非右键按下状态也需要记录
	t_iMouseX	= LOWORD(_lParam);
	t_iMouseY	= HIWORD(_lParam);


	// 处理鼠标的移动事件


	// 左键点下，在场景内查找点击点，显示光标
	if( WM_LBUTTONDOWN == _uMsg && ((GetAsyncKeyState(VK_SHIFT)&0x8000)==0) )
	{
		if( !this->m_ptrActiveScene ) return;
		if( !this->m_ptrActiveScene->is_renderEnable() ) return;

		os_tileIdx    t_sMousePos;

		t_sMousePos.m_iX = GET_X_LPARAM( _lParam ); 
		t_sMousePos.m_iY = GET_Y_LPARAM( _lParam );

		t_cmd.m_eCommandId = LM_SCENE_LBUTTONDOWN;
		t_cmd.m_dwCmdContentSize = sizeof( os_tileIdx );
		t_cmd.m_ptrCommandContent = &t_sMousePos;

		lmc_commandMgr::Instance()->push_command( t_cmd );
	}

	// 右键点击下来，目前是在场景修改碰撞信息`
# if __DEPLOY_MONSTERMOD__
	if( WM_RBUTTONDOWN == _uMsg )
# endif
# if __COLL_MOD__
	if( WM_MBUTTONDOWN == _uMsg )
# endif  
	{
		if( !this->m_ptrActiveScene ) return;
		if( !this->m_ptrActiveScene->is_renderEnable() ) return;

		os_tileIdx    t_sMousePos;

		t_sMousePos.m_iX = GET_X_LPARAM( _lParam ); 
		t_sMousePos.m_iY = GET_Y_LPARAM( _lParam );

		t_cmd.m_eCommandId = LM_SCENE_RBUTTONDOWN;
		t_cmd.m_dwCmdContentSize = sizeof( os_tileIdx );
		t_cmd.m_ptrCommandContent = &t_sMousePos;

		lmc_commandMgr::Instance()->push_command( t_cmd );
	}
	//! windy add Shift + 左键
	if( WM_LBUTTONDOWN == _uMsg && ((GetAsyncKeyState(VK_SHIFT)&0x8000)!=0) )
	{
		if( !this->m_ptrActiveScene ) return;
		if( !this->m_ptrActiveScene->is_renderEnable() ) return;

		os_tileIdx    t_sMousePos;

		t_sMousePos.m_iX = GET_X_LPARAM( _lParam ); 
		t_sMousePos.m_iY = GET_Y_LPARAM( _lParam );

		t_cmd.m_eCommandId = LM_SCENE_RBUTTONDOWN;
		t_cmd.m_dwCmdContentSize = sizeof( os_tileIdx );
		t_cmd.m_ptrCommandContent = &t_sMousePos;

		lmc_commandMgr::Instance()->push_command( t_cmd );
	}


	unguard;
}




/** \brief
*  对所有的窗口消息进行处理.
*/
LRESULT gc_gameApp::msg_proc( HWND _hWnd,UINT _uMsg, WPARAM _wParam, LPARAM _lParam )
{
	guard;

	PAINTSTRUCT   ps;
	HDC          hdc;
	RECT  t_rect;

	// 
	//osDebugOut( "The Message is<%d>.<%d>..\n",_uMsg,WM_KEYUP );

	if( _uMsg == WM_KEYUP )
		osDebugOut( "The key msg is:<%d>...\n",_wParam );

	switch( _uMsg ) 
	{
	case WM_KEYDOWN:
		{
			process_keyDownMsg( _uMsg,_wParam,_lParam );
			break;
		}
	case WM_SIZE:
		{

			::GetClientRect( m_hWnd,&t_rect );
			if( gc_camera::m_ptrCamera )
				gc_camera::m_ptrCamera->set_viewport( 0,0,t_rect.right,t_rect.bottom,0.0f,1.0f );

			return DefWindowProc( _hWnd,_uMsg,_wParam,_lParam );
		}

	case WM_SETCURSOR:
		{
			if( m_3dMgr && (m_iCommonCursorId >= 0) )
			{
				m_3dMgr->set_hwCursor();
				if( gc_gameScene::m_ptrScene )
					gc_gameScene::m_ptrScene->set_cursor( m_iCommonCursorId );
			}
			break;
		}
	case WM_SETFOCUS:
		{
			m_bActive = TRUE;
		}
		break;
	case WM_KILLFOCUS:
		{
			m_bActive = FALSE;
		}
		break;
	case WM_ACTIVATE:
		{
			if( (WA_INACTIVE != _wParam) && m_ptrInputDevice )
			{
				m_ptrInputDevice->acquire();			
			}
			break;
		}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
	case WM_MBUTTONDOWN:
		{
			process_mouseMsg( _uMsg,_wParam,_lParam );
			break;
		}
	case WM_COMMAND:
		{
			// 分析菜单选择：
			switch (LOWORD(_wParam))
			{
			case IDM_ABOUT:
				DialogBox(m_hIns, (LPCTSTR)IDD_ABOUTBOX,_hWnd, (DLGPROC)About);
				break;
			}
			break;
		}
	case WM_PAINT:
		{
			hdc = BeginPaint( _hWnd, &ps);
			// TODO: 在此添加任意绘图代码...
			EndPaint(_hWnd, &ps);
			break;
		}
	case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc( _hWnd,_uMsg,_wParam,_lParam );

	}

	return 0;

	unguard;
}

