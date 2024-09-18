//=======================================================================================================
/**  \file
 *   Filename: tengine.cpp
 *
 *   His:      River created @ 2003-12-23
 *
 *   Des:      用于测试渲染引擎，调入新的地图格式，把人物和物品加入到引擎中去。
 *             引擎测试文件.
 */
//=======================================================================================================
# include "tengine.h"

# include "Character.h"
//#include "../Terrain/GeoMipTerrain/osMapTerrain.h"
//# include "Commdlg.h"

#include "cameractrl.h"

# include "equipmentmanager.h" 

#include "dxutil.h"
//#include "Pathmanage.h"

# include "direct.h"
# include "commDlg.h"

# pragma comment( lib,"Comdlg32" )


/*
关于场景的球形碰撞检测:
1: 计算出当前bsphere影响的tileGround块
2: 对每一个TG的quadTree结点,计算此结点是否跟bsphere相交,如果相交,则继续下一步处理:
3: 根据碰撞顶点高度，构建多边形，计算此bsphere是否跟这些多边形相交

4: 
*/

# define HEIGHT_FROMGROUND   1.5f


npc_adjust   g_npcAdjust;

extern 	int g_terrainMaterialIdx ;

static int     t_siDLId = -1;

static I_skinMeshObj*   t_sSmPtr = NULL;

osVec3D      g_vecLine[MAX_SPLITNUM];
//! 是否处下调整地图高度信息的模式。
bool                g_bCHMod = false;


int          g_iCursorXPos = 0;
int          g_iCursorYPos = 0;


//! 每0.2秒一步，即0.75米/0.2秒.
float       MOVE_SPEED = 1.0f;

# define    TEST_FONT   1

# define    SCENE_EXIST 1

//! 主角人物插入地图的位置。　
int         g_iInsertX;
int         g_iInsertZ;

//! 右键点击鼠标时播放的动作的名称。
char        g_strRBtnName[10][256];
int         g_iRBtnActIdx = 0;

//! 相机移动状态。
bool        g_bCameraMove = false;



# if MUSIC_TEST
I_soundManager*   g_ptrSMgr = NULL;
# endif 


bool       g_bRenderBtn = false;

bool       g_bAmbiChange = false;
osVec3D    g_vec3AmbiChange;
osVec3D    g_vec3DirChange;

// 是否是测试选择人物的版本。
# define VERSION_SELCHR 0

// 是否测试界面。
# define UI_TEST    1

//! 全局的设备管理类。
I_deviceManager*   g_3dMgr = NULL;
//! 全局的输入管理类.
I_inputMgr*       g_inputMgr = NULL;
//! 全局的资源管理类.
I_resource*       g_resourceMgr = NULL;

I_fdScene*        g_sceneMgr = NULL;

I_camera*         g_camera = NULL;

I_socketMgr*      g_ptrSocket = NULL;

sg_timer*         g_timer;


//!　测试bbox.
os_bbox           t_testBbox;
static  bool      t_bBoxSet = true;
DWORD             t_boxColor = 0xffff0000;
static  float     t_fBoxRot = 0.0f;

I_skinMeshObj*    g_skinMeshObj;

I_skinMeshObj*    g_ptrNpcSkin = NULL;

os_deviceinit     g_devinit;
int              g_iTexid = -1;


//! 游戏运行过程中的动态光id.
int              g_iDL2Id = -1;
int              g_iDL3Id = -1;


//! 跟随主角运动的动态光的id.
int              g_iDLid = -1;



//! 跟随主角的阴影的xzBillBoardid.
int              g_iShadowId = -1;

//! 测试粒子系统的id. 
static int       g_iParId = -1;
static int       g_iPar2Id = -1;

static osVec3D   g_vec3ParPos;
static osVec3D   g_vec3Par2Pos;

static  float    g_etime = 0.0f;
static  float    g_etime2 = 0.0f;


static int       g_iBBid = -1;

//! Console的输入状态.
bool             g_bConsoleIO = false;

HWND             g_hWnd;


//! 当前帧是否截图.
bool                g_bCatchScr = false;

//! 使用人物box
bool                g_bChrBox = false;



VEC_stringDisp      g_stringDisp;



//! 当前是否处理移动状态。
bool            g_bMoveStatus = false;


//! 人物要移到的目标和开始顶点。
osVec3D         g_vec3Start;
osVec3D         g_vec3End;

//@{
//！王凯add @ 1/14 2004
//!英雄
I_Hero	g_cHero;
//!全部的npc
CNpcManager  g_cNpcManager;

//!相机
CCameraCtrl  g_cCamera;

//!寻路A*管理类
//Pathmanage  g_PathManage;

char            g_initMap[128];

CEquipmentManager g_EquipManager;
//test
I_Npc  g_Hero[10];
s_string g_filename[10];
//@}
// 定义是否调入场景。
# define       LOAD_SCENE 0





sc_BBox     t_b1,t_b2;




//! 是否显示帧速度等信息。
bool         g_bDisInfo = true;



/** \brief
 *   用于渲染引擎选择我们需要的渲染设备。
 */
static bool cfm_callback( LPDIRECT3D9 _d3d9,D3DCAPS9* _caps,
								ose_VertexProcessingType _vpt,D3DFORMAT _ffor,D3DFORMAT _bfor )
{


	if( _caps->DeviceType == D3DDEVTYPE_REF  )
		return false;


	if( (_vpt != SOFTWARE_VP) &&(_vpt != MIXED_VP) )
	{
		if( _caps->VertexShaderVersion < D3DVS_VERSION(1, 1) )

			return false;
	}

	// 软件渲染时，有足够的blendMatrixIndex.
	//if( (_vpt != SOFTWARE_VP)&&
	//	(_caps->MaxVertexBlendMatrixIndex < 32) )
	//	return false;

# if 0
	if( _vpt != SOFTWARE_VP )
		return false;
# endif 



	// Need to support post-pixel processing (for fog and stencil)
    if( FAILED( _d3d9->CheckDeviceFormat( _caps->AdapterOrdinal, _caps->DeviceType,
        _ffor, D3DUSAGE_RENDERTARGET | D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
        D3DRTYPE_SURFACE, _bfor ) ) )
    {
        return false;
    }


	if( g_bShaderDebug )
	{
		if( _vpt != SOFTWARE_VP )
			return false;
	}

	return true;

}


char   g_szTestStr[256];



char*  g_szTTT = "hello,fukkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkfdasfdasfdasfdasfdasfdasfdasfdas";


//! 测试输入法的速度
void                t_addStr( void)
{

	static bool t_bF = true;
	static int  t_iIdx = 0;
	if( t_bF )
	{
		memset( g_szTestStr,0,sizeof( char )*256 );
		t_bF = false;
	}

	g_szTestStr[t_iIdx] = g_szTTT[t_iIdx];

	t_iIdx ++;

	os_stringDisp  t_disp;

	t_disp.color = osColor( 1.0f,1.0f,0.0f,1.0f );
	t_disp.x = 140;
	t_disp.y = 300;
	t_disp.charHeight = 12;
	t_disp = g_szTestStr;


	static os_screenPicDisp    t_pic;

	static bool t_bf = true;

	t_pic.blend_mode = 3;
	t_pic.m_scrVer[0].m_dwDiffuse = 0xffffffff;
	t_pic.m_scrVer[1].m_dwDiffuse = 0xffffffff;
	t_pic.m_scrVer[2].m_dwDiffuse = 0xffffffff;
	t_pic.m_scrVer[3].m_dwDiffuse = 0xffffffff;
	t_pic.m_scrVer[0].m_vecPos.x = 0;t_pic.m_scrVer[0].m_vecPos.y = 0;
	t_pic.m_scrVer[1].m_vecPos.x = 100;t_pic.m_scrVer[1].m_vecPos.y = 0;
	t_pic.m_scrVer[2].m_vecPos.x = 100;t_pic.m_scrVer[2].m_vecPos.y = 100;
	t_pic.m_scrVer[3].m_vecPos.x = 0;t_pic.m_scrVer[3].m_vecPos.y = 100;

	if( t_bf )
	{
		t_pic.tex_id = g_sceneMgr->create_scrTexture( "ui\\ui1.tga" );
		t_bf = false;
	}


	g_sceneMgr->disp_stringImm( &t_disp,0,&t_pic );


};



/** \brief
 *  使用当前帧过去的时间计算从源点到目标点可以移动的距离。
 *  
 *  在此函数封装更合理的方法。
 */
void               t_moveInterpolate( float _etime,osVec3D* _moveVec,osVec3D* _resVec )
{
	guard;// t_moveInterpolate() );

	osassert( _etime );

	osVec3Normalize( _moveVec,_moveVec );
	*_resVec = (*_moveVec)*_etime;
	return;

	unguard;
}



//! 每一帧去执行的函数，如果当前处于可移动状态的话，处理人物的移动。
void              t_moveHero( void )
{
	guard;// t_moveHero() );

	osVec3D  t_moveVec;
	osVec3D  t_resVec;
	osVec3D  t_xVec; 
	float    t_angle;
	float    t_eleTime = sg_timer::Instance()->get_lastelatime();


	g_cNpcManager.UpdatePos(t_eleTime);

	g_cHero.Move(t_eleTime);

	// 得到跟随主角的动态光的id,并移动这个动态光。
	os_dlightStruct   t_dl;
	float            t_agl;

	if( g_iDLid >= 0 )
	{
		t_dl.m_lightColor = osColor( g_fDlR,g_fDlG,g_fDlB,1.0f );
		t_dl.m_fRadius = g_fDlRadius;
		g_cHero.m_pObj->get_posAndAgl( t_dl.m_vec3Pos,t_agl );
		t_dl.m_vec3Pos.y += 0.2f;
		g_sceneMgr->set_dynamicLight( g_iDLid,t_dl );
	}
	t_dl.m_vec3Pos.y -= 0.2f;

	if( g_iShadowId >= 0 )
		g_sceneMgr->set_shadowPos( g_iShadowId,g_cHero.m_verPos );

	g_cCamera.Update();


	unguard;

}



//@{
//! 是否全屏.
bool                g_bFullScr;
int                 g_iScrWidth;
int                 g_iScrHeight;
//@}

//! 远剪切面的矩离。
float               g_fTerrFar = 168.0f;


//!   Read the device init struct from the ini file.
bool                read_ini( os_deviceinit& _devinit )
{
	CIniFile    inifile;
	float       t_r,t_g,t_b;
	int         t_i;

	if( !inifile.OpenFileRead( "init.ini" ) )
	{
		MessageBox( NULL,"Open ini file failed","Msg",MB_OK );
		return false;
	}


	inifile.ReadSection( "screen" );
	inifile.ReadLine( "width",(int*)&_devinit.m_dwScrWidth );
	inifile.ReadLine( "height",(int*)&_devinit.m_dwScrHeight );
	inifile.ReadLine( "bpp",(int*)&_devinit.m_dwColorBits );
	inifile.ReadLine( "fullscr",(int*)&_devinit.m_bFullScr );
	g_bFullScr = _devinit.m_bFullScr;
	_devinit.m_bFullScr = false;
	g_iScrWidth = _devinit.m_dwScrWidth;
	g_iScrHeight = _devinit.m_dwScrHeight;


	inifile.ReadLine( "depth",(int*)&_devinit.m_dwDepthBits );
	inifile.ReadLine( "stencil",(int*)&_devinit.m_dwStencilBits );

	//
	// 加入logo信息。
	strcpy( _devinit.m_szDisTexName,"ui/logo.tga" );
	_devinit.m_fStartX = 0.4f;
	_devinit.m_fStartY = 0.4f;



	//
	// 读入引擎的初始设定.
	inifile.ReadSection( "engine" );

	// 读入引擎要初始化的地图。
	if( !inifile.ReadLine( "map",(char*)g_initMap,128 ) )
		return false;

	int   t_itest;
	if( !inifile.ReadLine( "rhinder",(int*)&t_itest ) )
		return false;
	if( !inifile.ReadLine( "tilewire",(int*)&g_bTileWireFrame ) )
		return false;
	if( !inifile.ReadLine( "charwire",(int*)&g_bCharWireFrame ) )
		return false;
	if( !inifile.ReadLine( "objwire",(int*)&g_bObjWireFrame ) )
		return false;
	if( !inifile.ReadLine( "shaderdebug",(int*)&g_bShaderDebug ) )
		return false;
	if( !inifile.ReadLine( "animation",(int*)&g_bUseAnimationObj ) )
		return false;

	// 读入雾相关的变量。
	int fog;
	if( !inifile.ReadLine( "fogenable",&fog ) )
		return false;
	g_bUseFog = fog;
	
	if( !inifile.ReadLine( "fogcolorR",(float*)&t_r ) )
		return false;
	if( !inifile.ReadLine( "fogcolorG",(float*)&t_g ) )
		return false;
	if( !inifile.ReadLine( "fogcolorB",(float*)&t_b ) )
		return false;
	D3DXCOLOR  t_color( t_r,t_g,t_b,1.0f );
	g_dwFogColor = t_color;

	if( !inifile.ReadLine( "fogstart",(float*)&g_fFogStart ) )
		return false;
	if( !inifile.ReadLine( "fogend",(float*)&g_fFogEnd ) )
		return false;

	if( !inifile.ReadLine( "farclip",(float*)&g_fTerrFar ) )
		return false;

	// 读入地图的宽度和高度.
	if( !inifile.ReadLine( "maxmapwidth",(int*)&t_i ) )
		return false;

	if( !inifile.ReadLine( "maxmapheight",(int*)&t_i) )
		return false;
	
	// 读入纹理的设置.
	if( !inifile.ReadLine( "texdiv",(int*)&g_dwTexDiv ) )
		return false;

	if( !inifile.ReadLine( "useDXTC",(int*)&g_bUseDXTC ) )
		return false;
	if( !inifile.ReadLine( "usesound",(int*)&g_bUseSound ) )
		return false;

	//@{
	// River @ 2004-5-10:
	// 读入动态光相关的数据。
	if( !inifile.ReadSection( "dlight" ) )
		return false;

	if( !inifile.ReadLine( "ambientR",(float*)&g_fAmbiR ) )
		return false;
	if( !inifile.ReadLine( "ambientG",(float*)&g_fAmbiG ) )
		return false;
	if( !inifile.ReadLine( "ambientB",(float*)&g_fAmbiB ) )
		return false;

	// 是否打开动态光。
	if( !inifile.ReadLine( "dLightEnable",(int*)&g_bEnableDLight ) )
		return false;
	
	if( !inifile.ReadLine( "dRadius",(float*)&g_fDlRadius ) )
		return false;

	if( !inifile.ReadLine( "dlightR",(float*)&g_fDlR ) )
		return false;
	if( !inifile.ReadLine( "dlightG",(float*)&g_fDlG ) )
		return false;
	if( !inifile.ReadLine( "dlightB",(float*)&g_fDlB ) )
		return false;
	//@} o

	//
	// 设置全局光的位置。
	if( !inifile.ReadSection( "directLight" ) )
		return false;

	g_bUseGDirLight = TRUE;

	if( !inifile.ReadLine( "lightPosX",(float*)&g_vec3LPos.x ) )
		return false;
	if( !inifile.ReadLine( "lightPosY",(float*)&g_vec3LPos.y ) )
		return false;
	if( !inifile.ReadLine( "lightPosZ",(float*)&g_vec3LPos.z ) )
		return false;

	// 设置全局方向光的颜色。
	if( !inifile.ReadLine( "lightCR",(float*)&g_fDirectLR ) )
		return false;
	if( !inifile.ReadLine( "lightCG",(float*)&g_fDirectLG ) )
		return false;
	if( !inifile.ReadLine( "lightCB",(float*)&g_fDirectLB ) )
		return false;

	// 读入右键点击鼠标时播放的动作名字。
	inifile.ReadSection( "rbtn" );
	for( int t_i = 0;t_i<10;t_i ++ )
	{
		char    t_str[16];
		sprintf( t_str,"ract%d",t_i );
		inifile.ReadLine( t_str,(char*)g_strRBtnName[t_i],256 );
	}

	g_bAlphaShelter = TRUE;

	//g_bForceSecEff = TRUE;

	g_bFadeEnable = true;



 	inifile.CloseFile();

	return true;

}

void delete_sword( void )
{
	os_equipment  t_equ;
	static bool   t_b = true;


	if( t_b )
	{
		t_equ.m_iCPartNum = 1;
		t_equ.m_arrId[0] = 6;
		t_equ.m_arrMesh[0] = 0;
		t_equ.m_arrSkin[0] = 1;
		t_b = false;
	}
	else
	{
		t_equ.m_iCPartNum = 1;
		t_equ.m_arrId[0] = 6;
		t_equ.m_arrMesh[0] = 1;
		t_equ.m_arrSkin[0] = 1;
		t_b = true;
	}

	g_cHero.m_pObj->change_equipment( t_equ );
}



/** \brief
 *  得到当前选择的地图块的函数.
 *  输入值是屏幕的二维坐标.
 */
void                t_getTGidx( int _x,int _y )
{
	guard;

	os_pickTileIdx   t_pickIdx;
	os_tileIdx       t_tileIdx;
	POINT           t_pt;

	osassert( _x >= 0 );
	osassert( _y >= 0 );

	if( !t_bBoxSet )
	{
		t_fBoxRot += 0.2f;
		t_testBbox.set_bbYRot( t_fBoxRot );
	}


	// TEST CODE:
	// 是否和人物的Bounding Box相交。
	osVec3D    t_vecStart,t_vecDir;
	osVec2D    t_vec2D;
	osMatrix   t_mat;
	float      t_f;

	osMatrixIdentity( &t_mat );
	g_cCamera.m_pCamera->get_curpos( &t_vecStart );
	t_vec2D.x = _x;t_vec2D.y = _y;
	t_vecDir = g_cCamera.m_pCamera->get_upprojvec( &t_vec2D,t_mat );

	os_bbox*  t_ptrBbox = g_cHero.m_pObj->get_smBBox();
	if( t_ptrBbox->ray_intersect( t_vecStart,t_vecDir,t_f ) )
		osDebugOut( "Pick hero,hello!!!!" );

	t_testBbox.ray_intersect( t_vecStart,t_vecDir,t_f );
		


	t_tileIdx.m_iX = _x;
	t_tileIdx.m_iY = _y;
	osVec3D   t_vec3Pick;
	if( !g_sceneMgr->get_pickTileIdx( t_pickIdx,t_tileIdx,&t_vec3Pick ) )
		return;


	os_tileIdx   t_curIdx;
	t_curIdx.m_iX = t_pickIdx.m_tgIdx.m_iX*128 + t_pickIdx.m_colTileIdx.m_iX;
	t_curIdx.m_iY = t_pickIdx.m_tgIdx.m_iY*128 + t_pickIdx.m_colTileIdx.m_iY;


	float   t_x,t_z;
	t_x = t_curIdx.m_iX * HALF_TWIDTH;
	t_z = t_curIdx.m_iY * HALF_TWIDTH;

	os_sceneCursorInit   g_sCursorInit;
	
	strcpy( g_sCursorInit.m_szCursorName,"s_animCursor" );
	g_sCursorInit.m_wXFrameNum = 3;
	g_sCursorInit.m_wYFrameNum = 3;
	g_sCursorInit.m_fFrameTime = 0.08f;
	g_sCursorInit.m_vec3CursorPos = t_vec3Pick;
# if 0
	strcpy( g_sCursorInit.m_szOsaCursorName,"keyani\\s_flag01.OSA" );
	g_sCursorInit.m_iPlayOsaNum = 1;
# endif 

	if( g_sceneMgr->collision_detection( t_curIdx ) )
		g_sCursorInit.m_dwCursorColor = 0xffffff00;
	else
		g_sCursorInit.m_dwCursorColor = 0xffff0000;
	g_sceneMgr->draw_cursorInScene( g_sCursorInit );


	PSTART( findPath );

	g_cHero.FindPath(t_pickIdx.m_tgIdx.m_iX,t_pickIdx.m_tgIdx.m_iY,
		t_pickIdx.m_colTileIdx.m_iX, t_pickIdx.m_colTileIdx.m_iY);

	PEND( findPath );



	unguard;
}


/** \brief
 *  拉近相机与地面的距离.
 */
void                t_moveClose( float _length )
{

	osVec3D    t_vec;
	osVec3D    t_camPos;

	g_cCamera.m_pCamera->get_godLookVec( t_vec );
	t_vec *= _length;

	g_cCamera.m_pCamera->get_curpos( &t_camPos );
	t_camPos += t_vec;

	osVec3D   t_vec3Focus;
	g_cCamera.m_pCamera->get_camFocus( t_vec3Focus );

	g_cCamera.m_pCamera->set_curpos( &t_camPos );

# if 0
	osDebugOut( "The pos is:<%f,%f,%f>...The Focus is:<%f,%f,%f>..\n",
		t_camPos.x,t_camPos.y,t_camPos.z ,t_vec3Focus.x,t_vec3Focus.y,t_vec3Focus.z );
# endif 

	t_vec3Focus -= t_camPos;
	osVec3Normalize( &t_vec,&t_vec );
	osVec3Normalize( &t_vec3Focus,&t_vec3Focus );

# if 0
	osDebugOut( "The vec is:<%f,%f,%f>...,After vec is:<%f,%f,%f>...\n",
		t_vec.x,t_vec.y,t_vec.z,t_vec3Focus.x,t_vec3Focus.y,t_vec3Focus.z );
# endif 
}

// 
void                on_mouseWheel( WPARAM _wParam,int _nX,int _nY )
{
	int t_iDelta = (int)_wParam;

	if( (t_iDelta > 0) )
		t_moveClose( 1.0f );

	if( (t_iDelta < 0) )
		t_moveClose( -1.0f );

	return ;
}


struct tmp_testStruct
{
	_TDEFAULTMESSAGE   t_send;
	char              m_szPassWord[128];
};



//! 读入主角人物插入的位置。　
void render_heroInsertPos()
{
	guard;

	CIniFile   t_file;

	t_file.OpenFileRead( "heroIni/hero.ini"  );

	t_file.ReadSection( "HERO" );
	t_file.ReadLine( "POSX",&g_iInsertX );
	t_file.ReadLine( "POSY",&g_iInsertZ );

	t_file.CloseFile();

	unguard;

}


static void display_processInfo( const char* _mapName )
{
	osDebugOut( _mapName );
}

void t_initRender( HINSTANCE _hInst,HWND _hwnd )
{
	guard;

	int   t_iSize = sizeof( os_sceVerTile );
	t_iSize = t_iSize * 4096 * 4 / 1024;

# if MUSIC_TEST

	//! 得到全局的声音管理器指针.
	g_ptrSMgr = get_soundManagerPtr(); 


	os_lisenerParam  t_listen;

	t_listen.m_vOrientFront = osVec3D( 0,0,-1 );
	t_listen.m_vOrientTop = osVec3D( 0,1,0 );

	t_listen.m_vPosition = osVec3D( 0.0f,0.0f,0.0f );

	if( !g_ptrSMgr->InitSoundManager( _hwnd,44100,16,t_listen ) )
		MessageBox( NULL,"初始化声音管理器失败!!!","ERROR",MB_OK );

# endif 

	// Init direct3d device.
	g_3dMgr = ::get_deviceManagerPtr();
	read_ini( g_devinit );

	g_3dMgr->init_device( _hwnd,&g_devinit,cfm_callback );


	os_bbox   t_box;
	float     t_f;
	bool      t_b;


	START_USEGBUF;
	END_USEGBUF;

	


	// 
	// 设置调入场景时使用的背景图片。
	g_3dMgr->set_backGround( "back",100,100,"边缘发行倒计时,目前处于黎明前的黑暗....",0xffff0000 );
	g_3dMgr->set_progress( 0.0f,400,500,"正调入地图..",0xffffff00 );

	// 读入主角人物的插入位置。
	render_heroInsertPos();


	g_hWnd = _hwnd;
	g_stringDisp.resize( 20 );
	g_sceneMgr = ::get_sceneMgr();



	//
	// 创建cursor,隐藏windows光标，并设置新的cursor
	int t_iCursor = g_sceneMgr->create_cursor( "ui\\cursor\\common.tga" );
	g_sceneMgr->set_cursor( t_iCursor );

	os_sceneLoadStruct  t_sl;
	

	//
	// 调入默认的位置文件，使用demo退出后回到上一次的位置。
	FILE*   t_file;
	t_file = fopen( "default.pos","rb" );
	if( t_file )
	{

		fread( &t_sl.m_iX,sizeof( int ),1,t_file );
		fread( &t_sl.m_iY,sizeof( int ),1,t_file );

		g_iInsertX = t_sl.m_iX;
		g_iInsertZ = t_sl.m_iY;

		fclose( t_file );
	}
	else
	{
		t_sl.m_iX = g_iInsertX;
		t_sl.m_iY = g_iInsertZ;
	}

# if TEST_FONT
	// 
	// 先设置合局光的颜色。
	g_sceneMgr->change_amibentCoeff( g_fAmbiR,g_fAmbiG,g_fAmbiB );


# if 1
	strcpy( t_sl.m_szMapName,g_initMap );

	if( !g_sceneMgr->load_scene( &t_sl ) )
		return ;

# endif 


	if(!LoadHeroFromIni("HeroIni\\hero.ini", &g_cHero))
	{
		osassert( false );
	}
	g_timer = sg_timer::Instance();



	//
	//　得到人物所在位置的高度。
	os_pickTileIdx      t_res;
	float              t_fAgl;
	osVec3D            t_heroPos;

	g_cHero.m_pObj->get_posAndAgl( t_heroPos,t_fAgl );
	if( !g_sceneMgr->get_sceneTGIdx( t_res,t_heroPos ) )
		t_res.m_fTileHeight = 0.0f;
	t_heroPos.y = t_res.m_fTileHeight;
	g_cHero.m_pObj->frame_move( t_heroPos,t_fAgl );

	// 
	// ATTENTION: Camera change:
	osVec3D   t_vecs( -20,20,-20 );
	g_cCamera.SetFollowObject(&g_cHero,&t_vecs);
	if(!g_cCamera.Create(g_hWnd))
		MessageBox(NULL,"g_cCamera.Create","",MB_OK);
	g_camera = g_cCamera.m_pCamera;

	


# if 0
	os_dlightStruct   t_dl;
	float            t_agl;
	t_dl.m_lightColor = osColor( 1.0f,1.0f,1.0f,1.0f );
	t_dl.m_fRadius = 15.0f;
	g_cHero.m_pObj->get_posAndAgl( t_dl.m_vec3Pos,t_agl );
	t_dl.m_vec3Pos.y += 0.4f; 
	g_iDLid = g_sceneMgr->create_dynamicLight( t_dl );
# endif 

	//调入退出时的相机位置。
	g_camera->load_camera( "default.cam" );


# endif 

# if VERSION_SELCHR 
	g_npcAdjust.load_npcFromIni( "chr.ini" );


	// 根据当前npc的位置设置相机。
	g_npcAdjust.set_camera( false );
# endif 


	RECT   t_rect;

	::GetClientRect( g_hWnd,&t_rect );
	int t_m = 0;

	osVec3D     t_vec3BPos;
	float       t_fRot;
	
	t_b1.m_fBoxRot = t_b2.m_fBoxRot = -OS_PI/4.0f;
	g_cHero.m_pObj->get_posAndAgl( t_vec3BPos,t_fRot );
	t_b1.m_vec3BoxPos = t_vec3BPos;
	t_b2.m_vec3BoxPos = t_vec3BPos;
	t_b1.m_vec3BoxPos.y += 3.0f;
	t_b2.m_vec3BoxPos.y += 2.0f;
	t_b1.m_vec3BoxPos.z += 1.0f;
	t_b2.m_vec3BoxPos.z -= 3.0f;
	t_b1.m_vec3BoxPos.x += 4.0f;
	t_b2.m_vec3BoxPos.x -= 3.0f;

	t_b1.vecmax = osVec3D( 3.84242f, 6.98674f, 4.96911f );
	t_b1.vecmin = osVec3D( -4.00273f, 0.0f, -3.92584f );

	t_b2.vecmax = osVec3D( 3.18219f, 0.106279f, 0.747631f );
	t_b2.vecmin = osVec3D( -2.7379f, 0.0342689f, -3.03477f );

	gFontMgrPtr->RegisterFont("宋体",12,FW_LIGHT);


	return;

	unguard;
}


void                test_renderUi()
{
	int   t_iWidth,t_iHeight,t_iTop;
	RECT  t_rect;
	static bool t_bFirst = true; 
	static int  t_iTexId[5];
	os_screenPicDisp   t_picDisp;


	osVec3D       t_pos,t_resPos;
	float         t_rotAngle;
	os_bbox*      t_ptrBbox;

	g_cHero.m_pObj->get_posAndAgl( t_pos,t_rotAngle );
	t_ptrBbox = g_cHero.m_pObj->get_smBBox();
	t_pos.y += 1.8f;

	t_resPos = t_pos;
	g_camera->get_scrCoodFromVec3( t_pos,t_resPos );

	osVec3D      t_vec3CamPos,t_vec3LookVec;
	g_camera->get_curpos( &t_vec3CamPos );
	g_camera->get_godLookVec( t_vec3LookVec );
	t_vec3CamPos += (t_vec3LookVec*1.1f );
	float  t_fFar = g_camera->get_farClipPlaneDis();
	g_camera->get_scrCoodFromVec3( t_vec3CamPos,t_vec3CamPos );


# if SCENE_EXIST
	static os_screenPicDisp   m_sBackPicDis;

	if( t_bFirst )
	{
		t_iTexId[0] = g_sceneMgr->create_scrTexture( "ui\\ui1.tga" );
		t_iTexId[1] = g_sceneMgr->create_scrTexture( "ui\\ui2.tga" );
		t_iTexId[2] = g_sceneMgr->create_scrTexture( "ui\\ui3.tga" );
		t_iTexId[3] = g_sceneMgr->create_scrTexture( "ui\\ui4.tga" );
		t_iTexId[4] = g_sceneMgr->create_scrTexture( "ui\\texture\\scb.tga" );

		t_bFirst = false;	

	}

# if 1
	::GetClientRect( g_hWnd,&t_rect );
	t_iHeight = 90;
	t_iTop = t_rect.bottom - t_iHeight;

	t_iWidth = t_rect.right-t_rect.left;
	t_iWidth /= 25;

	t_picDisp.blend_mode = 3;
	t_picDisp.tex_id = t_iTexId[0];


	// 第一张图片。
	t_picDisp.m_scrVer[0].m_dwDiffuse = 0xffffffff;
	t_picDisp.m_scrVer[1].m_dwDiffuse = 0xffffffff;
	t_picDisp.m_scrVer[2].m_dwDiffuse = 0xffffffff;
	t_picDisp.m_scrVer[3].m_dwDiffuse = 0xffffffff;

	t_picDisp.m_scrVer[0].m_vecUv = osVec2D( 0,0 );
	t_picDisp.m_scrVer[1].m_vecUv = osVec2D( 1,0 );
	t_picDisp.m_scrVer[2].m_vecUv = osVec2D( 1,90.0f/128.0f );
	t_picDisp.m_scrVer[3].m_vecUv = osVec2D( 0,90.0f/128.0f );


	t_picDisp.m_bUseZBuf = TRUE;

	t_picDisp.m_scrVer[0].m_vecPos.x = t_resPos.x - 20;
	t_picDisp.m_scrVer[0].m_vecPos.y = t_resPos.y - 20;

	t_picDisp.m_scrVer[1].m_vecPos.x = t_resPos.x + 20;
	t_picDisp.m_scrVer[1].m_vecPos.y = t_resPos.y - 20;

	t_picDisp.m_scrVer[2].m_vecPos.x = t_resPos.x + 20;
	t_picDisp.m_scrVer[2].m_vecPos.y = t_resPos.y + 20;

	t_picDisp.m_scrVer[3].m_vecPos.x = t_resPos.x - 20;
	t_picDisp.m_scrVer[3].m_vecPos.y = t_resPos.y + 20;

	t_picDisp.m_scrVer[0].m_vecPos.z = t_resPos.z;
	t_picDisp.m_scrVer[1].m_vecPos.z = t_resPos.z;
	t_picDisp.m_scrVer[2].m_vecPos.z = t_resPos.z;
	t_picDisp.m_scrVer[3].m_vecPos.z = t_resPos.z;

	g_sceneMgr->push_scrDisEle( &t_picDisp );

	g_sceneMgr->render_scene( true );

	t_picDisp.m_bUseZBuf = FALSE;
# if 1
	// 第二张图片。
	t_picDisp.tex_id = t_iTexId[1];
	t_picDisp.m_scrVer[0].m_vecPos.x = t_iWidth*8;
	t_picDisp.m_scrVer[0].m_vecPos.y = t_iTop;

	t_picDisp.m_scrVer[1].m_vecPos.x = t_iWidth*16;
	t_picDisp.m_scrVer[1].m_vecPos.y = t_iTop;

	t_picDisp.m_scrVer[2].m_vecPos.x = t_iWidth*16;
	t_picDisp.m_scrVer[2].m_vecPos.y = t_rect.bottom;

	t_picDisp.m_scrVer[3].m_vecPos.x = t_iWidth*8;
	t_picDisp.m_scrVer[3].m_vecPos.y = t_rect.bottom;
	g_sceneMgr->push_scrDisEle( &t_picDisp );

	// 第三张图片。
	t_picDisp.tex_id = t_iTexId[2];
	t_picDisp.m_scrVer[0].m_vecPos.x = t_iWidth*16;
	t_picDisp.m_scrVer[0].m_vecPos.y = t_iTop;

	t_picDisp.m_scrVer[1].m_vecPos.x = t_iWidth*24;
	t_picDisp.m_scrVer[1].m_vecPos.y = t_iTop;

	t_picDisp.m_scrVer[2].m_vecPos.x = t_iWidth*24;
	t_picDisp.m_scrVer[2].m_vecPos.y = t_rect.bottom;

	t_picDisp.m_scrVer[3].m_vecPos.x = t_iWidth*16;
	t_picDisp.m_scrVer[3].m_vecPos.y = t_rect.bottom;
	g_sceneMgr->push_scrDisEle( &t_picDisp );


	// 第4张图片。
	t_picDisp.tex_id = t_iTexId[3];
	t_picDisp.m_scrVer[0].m_vecPos.x = t_iWidth*24;
	t_picDisp.m_scrVer[0].m_vecPos.y = t_iTop;

	t_picDisp.m_scrVer[1].m_vecPos.x = t_rect.right;
	t_picDisp.m_scrVer[1].m_vecPos.y = t_iTop;

	t_picDisp.m_scrVer[2].m_vecPos.x = t_rect.right;
	t_picDisp.m_scrVer[2].m_vecPos.y = t_rect.bottom;

	t_picDisp.m_scrVer[3].m_vecPos.x = t_iWidth*24;
	t_picDisp.m_scrVer[3].m_vecPos.y = t_rect.bottom;
	g_sceneMgr->push_scrDisEle( &t_picDisp );

	g_sceneMgr->render_scene( true );
# endif 

# endif 

# endif 
}


void                t_renderStr( void )
{
	guard;
	int g_terrainMaterialIdx ;
	//osVec3D vec3Pos = g_cHero.m_verPos;
	osVec2D vec2Pos;
	vec2Pos.x = g_cHero.m_verPos.x;
	vec2Pos.y = g_cHero.m_verPos.z;

	os_terrainTexSnd     t_sTexSnd;
	int i = g_sceneMgr->get_materialIdx(vec2Pos,t_sTexSnd );
	int j =0;
	j++;
	g_terrainMaterialIdx = i;

	//osDebugOut( "Cur snd :<%s>..\n",t_sTexSnd.m_szSound );

	char   t_ptrstr[256],t_fpsstr[256];

# if SCENE_EXIST

	// 显示测试工程的版本号。
	g_stringDisp[0] = g_titleStr;
	g_stringDisp[0].color = 0xffffffff;
	g_stringDisp[0].x = 10;    
	g_stringDisp[0].y = 10;
	g_stringDisp[0].charHeight = 12;
	g_stringDisp[0].m_dwBackColor = 0;
	g_sceneMgr->disp_string( &g_stringDisp[0],1,false,true );

	sprintf( t_fpsstr,"%s:%d...可见格子数:<%d>..",
		"物品帧速率 ",int(sg_timer::Instance()->get_fps()),
		g_sceneMgr->get_inViewTileNum() );
	g_stringDisp[0] = t_fpsstr;
	g_stringDisp[0].color = 0xffffffff;//D3DXCOLOR( 1,1.0f,1.0f,1.0f );
	g_stringDisp[0].x = 560;    
	g_stringDisp[0].y = 2;
	g_stringDisp[0].charHeight = 12;
	g_stringDisp[0].m_dwBackColor = 0;


	// TEST CODE:
	sprintf( t_fpsstr,"帧时:<%f>,计时器品计时器品计时器品<%f>, 格子数<%d>, 2d纹理:<%d> camPos:<%f,%f,%f>,focus:<%f,%f,%f>",
		0.0123456789,0.123456789,10,10,0.123456789,0.123456789,0.123456789,
		0.123456789,0.123456789,0.123456789 );

/*
	sprintf( t_fpsstr,"%s%s%d%s%d","物理抗性提高当前地图: ","X--",
		g_cHero.m_nFieldX,"Y--",g_cHero.m_nFieldY );
		*/
	g_stringDisp[1] = t_fpsstr;
	g_stringDisp[1].color = D3DXCOLOR( 1,1.0f,1.0f,1.0f );
	g_stringDisp[1].x = 560;    
	g_stringDisp[1].y = 30;
	g_stringDisp[1].charHeight = 12;
	g_stringDisp[1].m_dwBackColor = 0;

	

	char   chunksInfo[2] = { 0,0 };


	sprintf( t_fpsstr,"%s",chunksInfo );
	g_stringDisp[8] = t_fpsstr;
	g_stringDisp[8].color = D3DXCOLOR( 1,1.0f,1.0f,1.0f );
	g_stringDisp[8].x = 60;    
	g_stringDisp[8].y = 30;
	g_stringDisp[8].charHeight = 12;
	g_stringDisp[8].m_dwBackColor = 0;


	
	int    t_x,t_y;
	t_x = g_cHero.m_nFieldX*128 + g_cHero.m_sBaseMesh.nX;
	t_y = g_cHero.m_nFieldY*128 + g_cHero.m_sBaseMesh.nY;
	sprintf( t_fpsstr,"%s%s%d%s%d:MaterialIdx:%d","当前位置: ","X--",t_x,"Y--",t_y ,g_terrainMaterialIdx);
	g_stringDisp[2] = t_fpsstr;
	g_stringDisp[2].color = D3DXCOLOR( 1,1.0f,1.0f,1.0f );
	g_stringDisp[2].x = 560;    
	g_stringDisp[2].y = 60;
	g_stringDisp[2].m_dwBackColor = 0;
	g_stringDisp[2].charHeight = 16;

	sprintf( t_fpsstr,"终..场景TriNum:<%d>,人物TriNum:<%d>,\n",
		g_sceneMgr->get_renderTriNum(),g_sceneMgr->get_skinMeshTriNum() );
	g_stringDisp[3] = t_fpsstr;
	g_stringDisp[3].color = D3DXCOLOR( 1,1.0f,1.0f,1.0f );
	g_stringDisp[3].x = 560;    
	g_stringDisp[3].y = 90;

	g_stringDisp[3].charHeight = 16;

	switch( g_eSkinMethod )
	{
	case OSE_D3DINDEXED:
		g_stringDisp[5] = "非VS人物渲染!!!\n";
		break;
	case OSE_D3DINDEXEDVS:
		g_stringDisp[5] = "VS人物渲染!!!\n";
		break;
	}
	g_stringDisp[5].color = osColor( 1.0f,1.0f,0.0f,1.0f );
	g_stringDisp[5].x = 560;
	g_stringDisp[5].y = 120;
	g_stringDisp[5].charHeight = 12;

	g_stringDisp[6].color = osColor( 1.0f,1.0f,0.0f,1.0f );
	g_stringDisp[6].x = 140;
	g_stringDisp[6].y = 300;
	g_stringDisp[6].charHeight = 12;
	g_stringDisp[6] = g_szTestStr;


	g_stringDisp[7].color = osColor( 1.0f,1.0f,0.0f,1.0f );
	g_stringDisp[7].x = 140;
	g_stringDisp[7].y = 320;
	if( g_bMapLoadding )
		sprintf( t_fpsstr,"地图调入中..." );
	else
		sprintf( t_fpsstr,"正常运行中..." );


# if __GLOSSY_EDITOR__
	sprintf( t_fpsstr,"人物动作帧索引：<%02d>..总帧数:<%d>",
		g_cHero.m_pObj->get_curActFrameIdx(),g_cHero.m_pObj->get_actFrameNum( "单手走路" ) );
# endif
	g_stringDisp[7] = t_fpsstr;
	g_stringDisp[7].charHeight = 12;


# if 0

	os_colorInterpretStr   t_istr;
	t_istr.m_fZ = 1.0f;
	t_istr.m_iPosX = 170;
	t_istr.m_iPosY = 235;
	
	FILE*  t_file = fopen( "tstring.txt","rb" );
	char   t_sz[4096*4];
	int    t_iSize = ::get_fileSize( "tstring.txt" );
	fread( t_sz,sizeof( char ),t_iSize,t_file );
	t_sz[t_iSize] = NULL;
	fclose( t_file );
	
	t_istr = t_sz;
	t_istr.m_iMaxPixelX = 404;
	t_istr.m_iMaxPixelY = 1231;
	t_istr.m_iRowDis = 2;
	t_istr.m_bForceDis = true;
	g_sceneMgr->disp_string( &t_istr,true,false  );
# endif 

	// 
	// TEST CODE:测试得到屏幕坐标的数据.
	osVec3D       t_pos,t_resPos;
	float         t_rotAngle;
	os_bbox*      t_ptrBbox;

	g_cHero.m_pObj->get_posAndAgl( t_pos,t_rotAngle );
	t_ptrBbox = g_cHero.m_pObj->get_smBBox();
	t_pos.y += t_ptrBbox->get_vecmax()->y;

	t_resPos = t_pos;
	g_camera->get_scrCoodFromVec3( t_resPos,t_resPos );
	g_stringDisp[4] = "Z缓冲测试";
	g_stringDisp[4].color = osColor( 1.0f,1.0f,1.0f,1.0f );
	g_stringDisp[4].x = t_resPos.x-16;
	g_stringDisp[4].y = t_resPos.y+20;
	g_stringDisp[4].z = t_resPos.z ;
	g_stringDisp[4].charHeight = 12;
	g_stringDisp[4].m_dwBackColor = 0xaa333333;
	g_sceneMgr->disp_string( &g_stringDisp[0],9,true,true );

# endif 

	unguard;
}

//
//  测试性的渲染函数.
//  His: 2003 4/17 加入对屏幕bill board的测试.
//
void                t_render( void )
{
	guard;

	HRESULT        t_hr;
	static int     length = 32;
	char           t_fpsstr[256];
	os_sceneFM      t_fmData;
	static float          t_f = 0;

	sg_timer::Instance()->framemove();



	DWORD   t_dwColor = D3DXCOLOR( 1.0f,1.0f,1.0f,0.3f );
	
	os_lisenerParam t_listen;
	float t_fAgl;
	osVec3D t_vecLookDir;
	osVec3D t_pos;
	g_camera->get_curpos( &t_pos );
	g_camera->get_godLookVec( t_vecLookDir );
	osVec3Normalize( &t_vecLookDir, &t_vecLookDir );
	//计算相机的方向
	t_listen.m_vPosition = t_pos;
	t_listen.m_vOrientTop = osVec3D( 0, 1, 0 );
	t_listen.m_vOrientFront = osVec3D( t_vecLookDir.x, 0, t_vecLookDir.z );
	g_ptrSMgr->SetListenerPara( t_listen );


	static float t_fStart = 0.0f;
	if( g_bAmbiChange )
	{
		t_fStart += sg_timer::Instance()->get_lastelatime();
		if( t_fStart >= 0.2f )
		{
			t_fStart = 0.0f;
			g_bAmbiChange = false;
			g_fDirectLR = g_vec3DirChange.x;
			g_fDirectLG = g_vec3DirChange.y;
			g_fDirectLB = g_vec3DirChange.z;

			g_fAmbiR = g_vec3AmbiChange.x;
			g_fAmbiG = g_vec3AmbiChange.y;
			g_fAmbiB = g_vec3AmbiChange.z;
		}
	}

# if SCENE_EXIST
	
	t_moveHero();

	t_fmData.m_ptrCam = g_cCamera.m_pCamera;
	t_fmData.m_iCx = g_cHero.m_nFieldX*128+g_cHero.m_sBaseMesh.nX;
	t_fmData.m_iCy = g_cHero.m_nFieldY*128+g_cHero.m_sBaseMesh.nY;
	t_fmData.m_vecChrPos = g_cHero.m_verPos;

# else

	t_fmData.m_ptrCam = g_cCamera.m_pCamera;
	t_fmData.m_iCx = 0;
	t_fmData.m_iCy = 0;
	t_fmData.m_vecChrPos = osVec3D( 0,0,0 );


# endif 

	static bool t_bf = true;

	{
		t_bf = false;
		PSTART( KFRAME_MOVE );
		g_sceneMgr->frame_move( &t_fmData );
		PEND( KFRAME_MOVE );
	}

# if 0
	if( g_iParId >= 0 )
	{
		g_vec3ParPos.x += 1.0f;
		g_sceneMgr->set_particlePos( g_iParId,g_vec3ParPos );
	}
# endif 


	//　测试让人物处于半透明状态
	//g_cHero.m_pObj->set_fadeState( true,0.5f );


	

	PSTART( clearScr() );
/*	
	osColor  t_color = g_dwFogColor;
	t_color.r = 0.0f;
	t_color.g = 0.0f;
	t_color.b = 0.0f;
*/
	g_3dMgr->d3d_clear( 0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 
		                                g_dwFarTerrColor, 1.0f, 0L );

	PEND( clearScr() );

	g_3dMgr->d3d_begindraw();

# if TEST_FONT

	PSTART( renderScene() );
	
	g_sceneMgr->render_scene();

# if 0
	// TEST CODE:另外对主角人物进行渲染
	static int t_iXPos = 480;
	if( t_iXPos < 700 )
		t_iXPos ++;
	else
		t_iXPos = 480;
	static int t_iYPos = 330;
	if( t_iYPos < 600 )
		t_iYPos ++;
	else
		t_iYPos = 330;
	RECT  t_rect;
	t_iXPos = 0;
	t_iYPos = 0;
	t_rect.left = t_iXPos;
	t_rect.right = t_iXPos + 320;
	t_rect.top = t_iYPos;
	t_rect.bottom = t_iYPos + 240;
	g_cHero.m_pObj->render_charInScrRect( t_rect,OS_PI/2.0f,1.0f );
# endif 

	PEND( renderScene() );

	//
	// 必须在调用场景的渲染函数之前把场景中需要显示的二维图片推进到场景中去。
	PSTART( ui_test() );
# if 0
	if( g_bDisInfo )
	{
		for( int t_i=0;t_i<1;t_i++ )
		{
			test_renderUi();
		}
	}
# endif 
	PEND( ui_test() );


# if 0
	/*os_bbox  t_tmpBox;
	t_tmpBox.set_bbMaxVec( t_b1.vecmax );
	t_tmpBox.set_bbMinVec( t_b1.vecmin );
	t_tmpBox.set_bbPos( t_b1.m_vec3BoxPos );
	t_tmpBox.set_bbYRot( t_b1.m_fBoxRot );*/

	os_bbox t_tmpBox1;
	osVec3D minpos(0,0,0);
	t_tmpBox1.set_bbMinVec(minpos);
	osVec3D maxpos = minpos;
	maxpos.x += 10.0f;
	maxpos.y += 10.0f;
	maxpos.z += 10.0f;
	t_tmpBox1.set_bbMaxVec(maxpos);
	t_tmpBox1.set_bbPos(g_cHero.m_verPos);
	t_tmpBox1.set_bbYRot(0);
	/*t_tmpBox1.set_bbMaxVec( t_b2.vecmax );
	t_tmpBox1.set_bbMinVec( t_b2.vecmin );
	t_tmpBox1.set_bbPos( t_b2.m_vec3BoxPos );
	t_tmpBox1.set_bbYRot( t_b2.m_fBoxRot );*/
//	g_sceneMgr->draw_bbox( &t_tmpBox,0xffffff00 );
	g_sceneMgr->draw_bbox( &t_tmpBox1,0xffffff00 );
# endif 


# ifdef _DEBUG 

	if( g_bChrBox )
		g_sceneMgr->draw_bbox( g_cHero.m_pObj->get_smBBox(),0xffffff00 );
	if( g_ptrNpcSkin )
		g_sceneMgr->draw_bbox( g_ptrNpcSkin->get_smBBox(),0xffffff00 );

	if( !t_bBoxSet )
		g_sceneMgr->draw_bbox( &t_testBbox,t_boxColor );

/*
	os_bbox  t_box;
	t_box.vecmax = osVec3D( 20.0f,2.0f,20.0f );
	t_box.vecmin = osVec3D( 24.0f,2.0f,24.0f );
	g_sceneMgr->draw_bbox( &t_box,0xffffffff );
*/

# endif 


# endif 


	// 
	// 显示统计信息。
	if( g_bDisInfo )
		t_renderStr();
	
	// 测试画人物手部的bounding Box.
	draw_handBox();

# ifdef _DEBUG
	if( t_sSmPtr )
		g_sceneMgr->draw_bbox( t_sSmPtr->get_smBBox(),0xffff0000 );
# endif 


# if 1
	osVec3D      t_vecLine[4];
	osVec3D      t_vec3 = g_cHero.m_verPos;

	t_vecLine[0] = t_vecLine[1] = t_vecLine[2] = t_vecLine[3] = t_vec3;
	t_vecLine[1].x -= 2.0f;
	t_vecLine[2].z += 2.0f;
	g_sceneMgr->draw_line( g_vecLine,MAX_SPLITNUM,0xffff0000,2.0f );
# endif



	g_3dMgr->d3d_enddraw();


	//g_sceneMgr->frame_move( &t_fmData );

	if( g_bCatchScr )
	{
		g_3dMgr->catch_screen();
		g_bCatchScr = false;
	}

	//PSTART( present() );


	t_hr = g_3dMgr->d3d_present();

	if( FAILED( t_hr ) )
	{
		//
		//  在以后加入更多代码的时候,此处会有更加复杂的代码出现,
		if( t_hr == D3DERR_DEVICELOST )  
		{

		}
		else
			osassert( false );
	}
	
	//PEND( present() );

	g_ptrSMgr->FrameMove();

	unguard;
}


//! 是否点下右键。
bool          g_bRbtnDown = false;


//! 测试调入的人物或是怪物。
I_skinMeshObj*                load_tNpc( osVec3D& _vec3Pos )
{
	//g_SceneSoundBox;
	// 
	// 得到要创建人物的位置,在主角人物位置边上。
	osVec3D    t_vec3Pos;
	float      t_fAngle;

	static bool   t_b = TRUE;


	if( g_cHero.m_pObj )
		g_cHero.m_pObj->get_posAndAgl( t_vec3Pos,t_fAngle );
	else
		t_vec3Pos = _vec3Pos;


	// 创建测试人物
	os_skinMeshInit   t_smInit;

# if 1
	t_smInit.m_fAngle = t_fAngle;
	t_smInit.m_iActIdx = 0;
	t_smInit.m_iTotalActTypeNum = 1;
	t_smInit.m_smObjLook.m_iPartNum = 2;
	t_smInit.m_smObjLook.m_mesh[0] = 1;
	t_smInit.m_smObjLook.m_skin[0] = 1;
	t_smInit.m_smObjLook.m_mesh[1] = 1;
	t_smInit.m_smObjLook.m_skin[1] = 1;
	strcpy( t_smInit.m_strSMDir, "character\\gjqizhi" );
# else
	t_smInit.m_fAngle = 0.0f;
	t_smInit.m_iActIdx = 0;
	t_smInit.m_iTotalActTypeNum = 1;
	t_smInit.m_smObjLook.m_iPartNum = 1;

	t_smInit.m_smObjLook.m_mesh[0] = 1;
	t_smInit.m_smObjLook.m_skin[0] = 1;

	t_smInit.m_smObjLook.m_mesh[1] = 1;
	t_smInit.m_smObjLook.m_skin[1] = 1;

	t_smInit.m_smObjLook.m_mesh[2] = 1;
	t_smInit.m_smObjLook.m_skin[2] = 1;

	t_smInit.m_smObjLook.m_mesh[3] = 1;
	t_smInit.m_smObjLook.m_skin[3] = 1;

	t_smInit.m_smObjLook.m_mesh[4] = 1;
	t_smInit.m_smObjLook.m_skin[4] = 1;

	t_smInit.m_smObjLook.m_mesh[5] = 1;
	t_smInit.m_smObjLook.m_skin[5] = 1;

	t_smInit.m_smObjLook.m_mesh[6] = 1;
	t_smInit.m_smObjLook.m_skin[6] = 1;
	
	t_smInit.m_smObjLook.m_mesh[7] = 1;
	t_smInit.m_smObjLook.m_skin[7] = 1;

	strcpy( t_smInit.m_strSMDir,"character\\xiancyx"  );

# endif 


	t_smInit.m_vec3Pos = t_vec3Pos;

	t_smInit.m_vec3Pos.z += 1.0f;

	t_smInit.m_bShadowSupport = false;

	if( g_ptrNpcSkin == NULL )
	{
		t_smInit.m_iMlFlagIdx = 1;

		g_ptrNpcSkin = g_sceneMgr->create_character( &t_smInit );

		g_ptrNpcSkin->change_mlFlagTexture( NULL,//"character\\gjqizhi\\2_01_01.hon",
			"character\\gjqizhi\\second_01.hon","character\\gjqizhi\\third_01.hon" );
	}
	else
	{
		g_ptrNpcSkin->change_mlFlagTexture( NULL,//"character\\gjqizhi\\2_01_01.hon",
			NULL,"character\\gjqizhi\\third_02.hon" );
	}

	return g_ptrNpcSkin;

}

void change_action( void )
{
	os_newAction  t_act;

	static bool  t_bFirst = true;
	static bool  t_b = false;
	static int   t_iTimes = 0;


# if __GLOSSY_EDITOR__
	static int  t_iLen = 5;
	t_iLen ++;
	if( t_iLen > 12 )
		t_iLen = 6;


	g_cHero.m_pObj->set_sgMoveLength( t_iLen );

	if( t_bFirst )
	{
		g_cHero.m_pObj->change_weaponEffect( "particle\\lineseg.fdp" );
		t_bFirst = false;
	}
	else
	{
		g_cHero.m_pObj->rot_swordGlossyY( 0.1 );
		t_iTimes ++;

		if( t_iTimes >= 6 )
		{
			t_iTimes = 0;
			g_cHero.m_pObj->reset_swordGlossyPosRot();
		}
	}
	static float t_fs = 1.0f;

	//g_cHero.m_pObj->scale_weaponEffect( t_fs );

	t_fs += 0.5f;

# endif 

# if 0

	if( t_bFirst )
	{
		g_cHero.m_pObj->change_weaponEffect( "keyani\\s_BIstar.OSA" );
		t_bFirst = false;
	}
	else
	{
		t_bFirst = true;
		g_cHero.m_pObj->change_weaponEffect( "effTexture\\s_BIstar.OSA" );
	}

	if( t_b )
	{
		t_act.m_iNewAct = -1;
		strcpy( t_act.m_strActName,"单手攻击2" );
		strcpy( t_act.m_strRandActName[0],"单手攻击1" );
		strcpy( t_act.m_strRandActName[1],"单手攻击3" );
		strcpy( t_act.m_strRandActName[2],"单手跑步" );
		t_b = false;
		t_act.m_changeType = OSE_DIS_OLDACT;
		t_act.m_iPlayNum = 1;
	}
	else
	{
		t_act.m_iNewAct = -1;
		strcpy( t_act.m_strActName,"单手攻击1" );
		t_b = true;
		t_act.m_changeType = OSE_DIS_OLDACT;
	}
	g_cHero.m_pObj->change_skinMeshAction( t_act );

# endif 

	g_cHero.m_pObj->playSwordGlossy( false,true );

# if __GLOSSY_EDITOR__
	//g_cHero.m_pObj->save_weaponFile();
# endif 

	int  t_idx = -1;
	float  t_f = g_cHero.m_pObj->get_actUsedTime( "单手攻击2",t_idx );

	t_f = 0;
/*
	if( g_ptrNpcSkin )
		g_ptrNpcSkin->change_skinMeshAction( t_act );
*/
}


//! 测试画人物手部的box.
void                draw_handBox( void )
{
	osMatrix   t_mat;
	static     osMatrix    t_vec3ArrowMat;
	static     int        t_iOsaIdx = -1;
	osVec3D               t_vec3HandPos;

	return;

	if( !g_ptrNpcSkin )
		return;

	g_ptrNpcSkin->get_handPos( t_vec3HandPos );

	osVec3D   t_vec3Pos;
	float     t_f;

	g_ptrNpcSkin->get_posAndAgl( t_vec3Pos,t_f );
	
	//! 鼠标第一次击下时，创建箭的osa文件。　
	static bool t_bFirst = true;



	if( g_bRbtnDown )
	{

		if( t_bFirst )
		{
# if 0
			os_aniMeshCreate  t_mc;
			int              t_k; 
			t_mc.m_bAutoDelete = false;
			t_mc.m_dwAmbientL = 0xfffffff;
			t_mc.m_iPlayFrames = 1;
			t_mc.m_ePlayType = OSE_PLAYFOREVER;//
			t_mc.m_fPlayTime = 5.0f;
			t_mc.m_fAngle = 0.0f;

			// 

			strcpy( t_mc.m_szAMName, "keyAni\\arrow.OSA" );

			t_mc.m_vec3Pos.x = t_mat._41;
			t_mc.m_vec3Pos.y = t_mat._42;
			t_mc.m_vec3Pos.z = t_mat._43;


			t_iOsaIdx = g_sceneMgr->create_osaAmimation( &t_mc );
# else

			t_iOsaIdx = g_sceneMgr->create_billBoard(
				"efftexture\\s_guangqiu.BB",t_vec3HandPos );

# endif 

			t_bFirst = false;
		}
		else
		{
	
			g_sceneMgr->set_billBoardPos( t_iOsaIdx,t_vec3HandPos );
		}

/*
		if( !g_ptrNpcSkin->arrive_stopFrameOrNot() )
		{
			t_vec3ArrowMat = t_mat;

			g_sceneMgr->set_aniMeshTrans( t_iOsaIdx,t_mat );
		}
		else
		{
			osVec3D    t_vec3T,t_vec3X;

			t_vec3T = osVec3D( 1.0f,0.0f,0.0f );
			osVec3TransformNormal( &t_vec3T,&t_vec3T,&t_vec3ArrowMat );
			t_vec3X.x = t_vec3ArrowMat._41;
			t_vec3X.y = t_vec3ArrowMat._42;
			t_vec3X.z = t_vec3ArrowMat._43;

			t_vec3X += ( t_vec3T*0.15f);

			t_vec3ArrowMat._41 = t_vec3X.x;
			t_vec3ArrowMat._42 = t_vec3X.y;
			t_vec3ArrowMat._43 = t_vec3X.z;

			//! 并设置箭的位置。
			g_sceneMgr->set_aniMeshTrans( t_iOsaIdx,t_vec3ArrowMat );

		}
*/

	}


}




void test()
{
	static int n = 0;
	if(n >= 10)
	{
		return;
	}
	else
		n ++;


	os_skinMeshInit    t_init;

	t_init.m_fAngle = 0;
	t_init.m_iActIdx = 4;
	t_init.m_iTotalActTypeNum = 7;
	strcpy( t_init.m_strSMDir, "Character\\nvfasi1" );
										
	t_init.m_smObjLook.m_iPartNum = 7;
	for( int t_i=0;t_i<7;t_i++ )
	{
		t_init.m_smObjLook.m_mesh[t_i] = 1;
		t_init.m_smObjLook.m_skin[t_i] = 1;
	}
	t_init.m_smObjLook.m_mesh[2] = 1;

	t_init.m_smObjLook.m_mesh[6] = 1;
	t_init.m_smObjLook.m_skin[6] = 1;

	t_init.m_vec3Pos = osVec3D( 46,0,74.5 );

	t_init.m_vec3Pos.z -= n;

	g_skinMeshObj = g_sceneMgr->create_character( &t_init );


}


void               rot_character( float _tf )
{
	osVec3D       t_pos;
	float         t_rotAngle;

	g_cHero.m_pObj->get_posAndAgl( t_pos,t_rotAngle );
	t_rotAngle += _tf;
	g_cHero.m_pObj->frame_move( t_pos,t_rotAngle );

}


//! 相机所在的位置是否合法.
bool          camera_posLegal( void )
{
	guard;

	osVec3D    t_vec3;
	bool       t_b;

	static bool  t_bFirst = true;

	return      true;

	if( t_bFirst )
	{
		t_bFirst = false;
		return true;
	}

# if SCENE_EXIST

	g_cCamera.m_pCamera->get_curpos( &t_vec3 );

	// 
	os_pickTileIdx  t_pti;
	t_b = g_sceneMgr->get_sceneTGIdx( t_pti,t_vec3 );
	osassert( t_b );

	if( (t_vec3.y - 1.28f) > t_pti.m_fTileHeight )
		return true;
	else
		return false;

# endif 

	return true;

	unguard;
}




//! 调整相机.
bool          readj_cam( float _rad,float& _fpitch )
{
	

	if( _fpitch > OS_PI )
		osassert( false );

	_fpitch += 0.05f;

	osVec3D   t_vec;
	t_vec = g_cHero.m_verPos;
	t_vec.y += 0;

	picth_cam( 0.05 );

	g_cCamera.m_pCamera->rotate_horizon( t_vec,_rad );
	bool t_b = camera_posLegal();
	g_cCamera.m_pCamera->rotate_horizon( t_vec,-_rad );

	if( t_b )
	{
		picth_cam( -0.05 );
		return true;
	}
	else
	{
		readj_cam( _rad,_fpitch );
	}

	picth_cam( -0.05 );

	return true;
}


/** \brief
 *  旋转相机.
 */
bool          rotate_cam( float _rad )
{

	osVec3D   t_vec;
	t_vec = g_cHero.m_verPos;
	t_vec.y += 0;
	if( OSE_CAMOPE_SUCCESS != 
		g_cCamera.m_pCamera->rotate_horizon( t_vec,_rad ) )
	{
		return true;
	}


	// 得到相机所在位置的格子高度和相机所在的位置,如果相机所在的位置低于
	// 格子高度,则返回相机到原来的位置,并pitch相机,然后再旋转.
	if( camera_posLegal() )
	{
		return true;
	}
	else
	{

		g_cCamera.m_pCamera->rotate_horizon( t_vec,-_rad );

		float   t_fPitch = 0.0f;
		readj_cam( _rad,t_fPitch );

		g_cCamera.m_pCamera->animate_camRotPicth( _rad,t_fPitch,0.4f );

		return true;
	}
	/**/
}

/** \brief
 *  Picth相机.
 */
void          picth_cam( float _rad )
{
	osVec3D   t_vec;
	t_vec = g_cHero.m_verPos;
	t_vec.y += 0;

	static float  t_sfMoveCloseDis = 0.0f;
	if( (t_sfMoveCloseDis>EPSILON)&&(_rad>0) )
	{
		float t_f = -0.1f*(_rad/0.0025);
		t_sfMoveCloseDis += t_f;
		t_moveClose( t_f );

		if(t_sfMoveCloseDis>EPSILON)
			return;
		else
			t_sfMoveCloseDis = 0.0f;
	}


	ose_camOperateRS  t_camOs = g_cCamera.m_pCamera->rotate_vertical( t_vec,_rad );


	if( OSE_CAMIN_ANIMATION == t_camOs )
		return;
		
	if( (OSE_CAMPITCH_TOO_LOW == t_camOs) ||
		(OSE_CAMPITCH_TOO_HIGH == t_camOs )  )
	{
		float  t_f = -0.1f*(_rad/0.0025);
		t_sfMoveCloseDis += t_f;
		t_moveClose( t_f );
	}

	if( 1 )//camera_posLegal() )
		return ;
	else
	{
		g_cCamera.m_pCamera->rotate_vertical( t_vec,-_rad );
	}
}


//
// 瞬移的处理：
// 检测当前瞬移到的目标位置：
// 如果瞬移到的位置和当前的地图位置很远，则释放当前的地图，显示地图的调入画面，
// 然后调入地图。
// 如果不远，则动态的调入和地图。
//! 人物的shun移效果。
void   character_move( void )
{
	osVec3D       t_pos;
	float         t_rotAngle;


	// 瞬移
	g_cHero.m_pObj->get_posAndAgl( t_pos,t_rotAngle );

	t_pos.x = 75.0f;
	t_pos.z = 75.0f;
	g_cHero.m_sBaseMesh.nX = 50;
	g_cHero.m_sBaseMesh.nY = 50;
	g_cHero.m_nFieldX = 0;
	g_cHero.m_nFieldY = 0;

	g_cHero.m_verPos = t_pos;
	g_cHero.m_pObj->frame_move( t_pos,t_rotAngle );
	g_cCamera.Update();



}



//! 测试人物武器附加的特效。
void                test_wEff( void )
{
	static bool  t_b = true;

	g_cHero.m_pObj->set_fadeState( true,0.5f );


	osVec3D  t_vec3Focus;
	osVec3D  t_vec3Pos;

	g_cCamera.m_pCamera->get_camFocus( t_vec3Focus );
	t_vec3Pos = t_vec3Focus;
	t_vec3Pos.y -= 1.4f;

	g_cCamera.m_pCamera->set_curpos( &t_vec3Pos );


# if __GLOSSY_EDITOR__
	if( t_b )
	{
		int     t_iType;
		float   t_fPos = 0.1f;
		char     t_szEffName[128];

		t_iType = 2;

		strcpy( t_szEffName,"particle\\s_daoguang.fdp" );//s_ZZxuetuA

		g_cHero.m_pObj->set_sgEffect( t_iType,t_fPos,t_szEffName );

		//g_cHero.m_pObj->save_weaponFile();

		t_b = false;
	}
	else
	{
/*
		static float   t_f = 0.0f;
		t_f += 0.05f;
		g_cHero.m_pObj->rotate_osaEffect( t_f );
*/

	}

# endif 

}


//@{
//! 特效测试相关的函数.
void               play_effect1( void )
{

	static int  t_idPar = -1;
	static int  t_idDyic = -1;


	//set_particleLineSeg

	osVec3D   t_vecPos;
	g_sceneMgr->get_tilePos( 86,84,t_vecPos );
	//
	// TEST CODE:
	// 测试调入一个粒子系统。
	//os_bbox  t_tmpBox1;
	
	os_particleCreate    t_pcreate;
	t_pcreate.m_bAutoDelete = true;
	t_pcreate.m_bCreateIns = true;


	if( g_iParId < 0 )
		strcpy( t_pcreate.m_szParticleName,"particle\\s_lvwu01.fdp" );//S_Nhuo 

	t_pcreate.m_vec3Offset = g_cHero.m_verPos;
	t_pcreate.m_vec3Offset.y += 2.0f;

# if 1
	
	t_pcreate.m_bCreateIns = true;
	if( t_idPar < 0 )
	{
		static float t_f = 1.0f;

		t_pcreate.m_fSizeScale = t_f;
		t_f += 0.0f;
		t_idPar = g_sceneMgr->create_particle( t_pcreate );

	}
	else
	{		
		g_sceneMgr->delete_particle( t_idPar );
		t_idPar = -1;t_idDyic = -1;
	}

# else
	
	if( g_iParId < 0 )
		g_iParId = g_sceneMgr->create_particle( t_pcreate );
	else
		g_sceneMgr->delete_particle( g_iParId );

	g_vec3ParPos = t_pcreate.m_vec3Offset;

# endif 



}



//  test...
//! 下雨测试。
void   rain_test( void )
{
	static bool   t_b = true;
	if( t_b )
	{

		// 
		// 先设置合局光的颜色。
		g_sceneMgr->change_amibentCoeff( 0.3f,0.3f,0.3f );
		os_rainStruct  t_rs;
		t_rs.m_fRainMoveSpeed /= 10.0f;
		g_sceneMgr->start_sceneRain( &t_rs );
		t_b = false;
	}
	else
	{
		t_b = true;

		// 设置合局光的颜色。
		g_sceneMgr->change_amibentCoeff( 0.4f,0.4f,0.4f );
		g_sceneMgr->stop_sceneRain();
	}
}



// 测试是否签入.


void               play_effect2( void )
{
	//
	// TEST CODE:
	// 测试调入公告板相关数据.
	osVec3D    t_vec3;
	DWORD      t_start,t_end;
	float      t_fNormalTime;

	t_vec3 = g_cHero.m_verPos;
	t_vec3.y += 1.0f;

	static osVec3D  t_vec3Back;
	static float    t_fAgl = OS_PI;

	static bool t_bFirst = true;
	static bool t_b = false;

# if 1

	if( !t_b )
	{
		t_fAgl += 0.1f;
		static s_string t_szEffName[4] = { "speff\\s_jian02.spe",
			"speff\\s_Ahuofang02.spe","speff\\s_Ahuofang03.spe","speff\\s_Ahuofang04.spe" };
		static int      t_idx = 0;


		if( t_bFirst )
		{
			g_sceneMgr->create_effect(
				t_szEffName[t_idx].c_str(),t_vec3,t_fAgl, FALSE);
			t_bFirst = false;
		}

		//zyla_fj_009_07.spe
		g_iBBid = g_sceneMgr->create_effect(
			t_szEffName[t_idx].c_str(),t_vec3,t_fAgl );

		//t_idx ++;
		if( t_idx >=4 )
			t_idx = 0;

		t_b = true;
		t_vec3Back = t_vec3;
	}
	else
	{
		t_b = false;
		if( g_iBBid >= 0 )
		{
			g_sceneMgr->delete_effect( g_iBBid );
			g_iBBid = -1;
		}

	}


# else

	t_start = timeGetTime();
	t_vec3.y += 5.0f;
	g_iBBid = g_sceneMgr->create_billBoard( 
		"efftexture\\taxis.bb",t_vec3,true,2.0f );
	t_end = timeGetTime();

	//t_fNormalTime = g_sceneMgr->set_bbPlayTime( g_iBBid,1.0f );

	/*
os_dlightStruct  t_dl;
	//t_dl.m_fRadius = 6.0f;
	//t_dl.m_lightColor = 0xffffffff;
	//t_dl.m_vec3Pos = t_vec3;
	//t_dl.m_vec3Pos.y -= 1.0f;
	g_sceneMgr->create_dynamicLight( t_dl ); 

	osDebugOut( "The load time is:<%d>,Normal play time is:<%f>..\n",
		t_end - t_start,t_fNormalTime );
*/

# endif 

}


//! 测试使用动作名调用动作。
void                play_actWithName( char* _name )
{
	guard;

	os_newAction  t_act;
	t_act.m_iNewAct = -1;
	strcpy( t_act.m_strActName,"单手攻击1" );
	t_act.m_changeType = OSE_DIS_NEWACT_TIMES;
	t_act.m_iPlayNum = 1;

	g_cHero.m_pObj->change_skinMeshAction( t_act );

	unguard;
}


int   g_iOsaId = -1;
int   g_iOsaId2 = -1;

void               rot_osa( void )
{
	static float   t_f = 0;

	t_f += 0.1f;

# if 0
	if( g_iOsaId < 0 )
		return;


	osMatrix    t_mat;
	osMatrixRotationX( &t_mat,t_f );
	g_sceneMgr->set_aniMeshTrans( g_iOsaId,t_mat );
# else

	I_soundManager*   t_ptrSptr = get_soundManagerPtr();

	DWORD    t_dw = timeGetTime();

	t_ptrSptr->PlayFileMP3( "music\\land.mp3" );

	osDebugOut( "The play time is:<%d>..\n",timeGetTime()-t_dw );

# endif 


}

void               hero_areaMove( void )
{

}


// 测试播放其它的场景中的特效.
void               play_effect3( void )
{

	float            t_f;
	
/*
    g_bAmbiChange = true;
    g_vec3DirChange.x = g_fDirectLR;
	g_vec3DirChange.y = g_fDirectLG;
	g_vec3DirChange.z = g_fDirectLB;

	g_vec3AmbiChange.x = g_fAmbiR;
	g_vec3AmbiChange.y = g_fAmbiG;
	g_vec3AmbiChange.z = g_fAmbiB;

	g_fDirectLR = 1.0f;
	g_fDirectLG = 1.0f;
	g_fDirectLB = 1.0f;

	g_fAmbiR = 0.8f;
	g_fAmbiG = 0.8f;
	g_fAmbiB = 0.8f;
*/

# if 1
	t_f = g_cCamera.m_pCamera->get_farClipPlaneDis();

	osVec3D    t_vec3Pos;
	g_cHero.m_pObj->get_posAndAgl( t_vec3Pos,t_f );

	os_aniMeshCreate  t_mc;
	int              t_k; 
	t_mc.m_bAutoDelete = false;
	t_mc.m_iPlayFrames = 1;
	t_mc.m_ePlayType = OSE_PLAYFOREVER;//
	t_mc.m_fPlayTime = 1.0f;
	t_mc.m_fAngle = 0.0f;
	t_mc.m_bEffectByAmbiL = false;


	g_cHero.m_pObj->get_posAndAgl( t_mc.m_vec3Pos,t_f );
	t_mc.m_vec3Pos.y += 1.8f;


	if( g_iOsaId2 < 0 )
	{
		static float  t_fSizeScale = 1.0f;
		static float  t_fRot = 0.0f;

		t_fRot += 0.5f;
		t_fSizeScale = 1.0f;
		t_mc.m_fAngle = t_fRot;
		t_mc.m_fSizeScale = 0.1f;

		strcpy( t_mc.m_szAMName,"keyAni\\ttt.OSA" );
		g_iOsaId2 = g_sceneMgr->create_osaAmimation( &t_mc );
		//g_sceneMgr->set_osaPlayTime( g_iOsaId2,0.3f );
	}
	else
	{
		g_sceneMgr->delete_animationMesh( g_iOsaId2 );
		g_iOsaId2 = -1;

	}	
# endif

}

void               play_effect4( void )
{
	os_aniMeshCreate  t_mc;
	int              t_k; 
	t_mc.m_bAutoDelete = false;
	t_mc.m_dwAmbientL = 0xfffffff;
	t_mc.m_iPlayFrames = 100;
	t_mc.m_ePlayType = OSE_PLAYFOREVER;//
	t_mc.m_fPlayTime = 5.0f;
	t_mc.m_fAngle = 0.0f;

	// 

	
	strcpy( t_mc.m_szAMName, "keyAni\\tttosa.OSA" );
//	strcpy( t_mc.m_szAMName, "keyAni\\pris_01_03.OSA" );
	t_mc.m_vec3Pos = g_cHero.m_verPos;

	//t_mc.m_vec3Pos.x = t_mat._41;
	//t_mc.m_vec3Pos.y = t_mat._42;
	//t_mc.m_vec3Pos.z = t_mat._43;


	g_sceneMgr->create_osaAmimation( &t_mc );
}
//@} 

//! 测试天气变化.
void               change_weather( void )
{
	os_weather    t_wea;

/*
	g_sceneMgr->get_sceneWeather( t_wea );

	t_wea.m_fFogEnd = 300.0f;
	t_wea.m_fFogStart = 200.0f;
*/
/*
	t_wea.m_bRain = true;
	t_wea.m_fFogEnd = 300.0f;
	t_wea.m_fFogStart = 200.0f;
	t_wea.m_sFogColor.a = 1.0f;
	t_wea.m_sFogColor.r = 0.4f;
	t_wea.m_sFogColor.g = 0.4f;
	t_wea.m_sFogColor.b = 0.7f;

	t_wea.m_sAmbientColor.a = 1.0f;
	t_wea.m_sAmbientColor.r = 0.2f;
	t_wea.m_sAmbientColor.g = 0.2f;
	t_wea.m_sAmbientColor.b = 0.2f;

	t_wea.m_sDirLightColor.a = 1.0f;
	t_wea.m_sDirLightColor.r = 1.0f;
	t_wea.m_sDirLightColor.g = 1.0f;
	t_wea.m_sDirLightColor.b = 1.0f;


	t_wea.m_vec3DirLight = osVec3D( 1.0f,1.0f,1.0f );

	t_wea.m_fSkyMoveSpeed = 0.01f;
	strcpy( t_wea.m_szSkyTex,"sky1" );

	strcpy( t_wea.m_szOsaBkSky,"keyAni\\tsky1.osa" );


	static int t_idxNum = 0;

	FILE*   t_file = NULL;

	if( t_idxNum == 0 )
	{
		t_file = fopen( "weather\\default.wea","rb" );
		t_idxNum ++;
	}
	else
		t_file = fopen( "weather\\vilg00.wea","rb" );

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

# if 0
	t_wea.m_sAmbientColor.a = 1.0f;
	t_wea.m_sAmbientColor.r = 0.2f;
	t_wea.m_sAmbientColor.g = 0.2f;
	t_wea.m_sAmbientColor.b = 0.2f;

	t_wea.m_sDirLightColor.a = 1.0f;
	t_wea.m_sDirLightColor.r = 1.0f;
	t_wea.m_sDirLightColor.g = 1.0f;
	t_wea.m_sDirLightColor.b = 1.0f;
	t_wea.m_vec3DirLight = osVec3D( 1.0f,1.0f,1.0f );
# endif 

	t_wea.m_sFarTerrColor = osColor( 1.0f,0.0f,1.0f,1.0f );
# endif 
	*/
	
	FILE*   t_file = NULL;

	
	
	t_file = fopen( "weather\\default.wea","rb" );
//	t_idxNum ++;
	
	

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
	t_wea.m_fSkyMoveSpeed = 2.0f;
	t_wea.m_fSky1MoveSpeed = 2.0f;
	t_wea.m_sAmbientColor.r = 0.4f;
	t_wea.m_sAmbientColor.g = 0.4f;
	t_wea.m_sAmbientColor.b = 0.4f;

	t_wea.m_sDirLightColor.r = 0.3f;
	t_wea.m_sDirLightColor.g = 0.3f;
	t_wea.m_sDirLightColor.b = 0.3f;
	osVec3D sundir(1.0f,1.0f,1.0f);
	osVec3Normalize(&t_wea.m_vec3DirLight,&sundir);
	t_wea.m_sFogColor = osColor(0.0,1.0,0.0,0);
	t_wea.m_fFogStart = 200;
	t_wea.m_fFogEnd = 300;
	static i = 0 ; 
	if (i%2==0)
	{
		strcpy(t_wea.m_szOsaBkSky,"keyAni\\vilg_dxg.OSA");
	}
	i++;
	



	strcpy(t_wea.m_szOsaBkSkyEffect[0],"keyAni\\tttt.OSA");
	strcpy(t_wea.m_szOsaBkSkyEffect[1],"");
	t_wea.m_fSky1MoveSpeed = 0;
	t_wea.m_fSky2MoveSpeed = 0;
	/*static i = 0;
	if (i==0)
	{
		strcpy(t_wea.m_szOsaBkSkyEffect[0],"");
		i++;
	}
	else
	{
		strcpy(t_wea.m_szOsaBkSkyEffect[0],"");
	}*/
	//strcpy(t_wea.m_szOsaBkSkyEffect[0],"keyAni\\fmnc_effect01.OSA");
//	strcpy(t_wea.m_szOsaBkSkyEffect[1],"");
	strcpy(t_wea.m_szOsaBolt,"keyAni\\s_huanjingshandian01.OSA");
	t_wea.m_iBoltRate = 0.2f;
	t_wea.m_BoltAngle = 0;
	t_wea.m_BoltHeight = 100;
	t_wea.m_fBoltFanAngle = 0.5f;
//	strcpy( t_wea.m_szOsaBkSky,"keyAni\\vilg_cun_01.osa" );

	//	strcpy(t_wea.m_szOsaBkSkyEffect[1],"");
	//	strcpy(t_wea.m_szOsaBolt,"");
	t_wea.m_BoltColor = osColor(1.0f,0,1.0f,0.0f);
	t_wea.m_bSnow = false;
	t_wea.m_TerrFogDistance = 10.0f;
	t_wea.m_SkyFogDistance = 10.0f;
	g_sceneMgr->change_sceneWeather( t_wea,3.0f );
	


	static bool t_b = false;

	g_fSkyTerrAnitime = 5.0f;
	g_fFogStart = 150;
	g_fFogEnd = 512;

	if( !t_b )
	{
		t_b = true;
		//g_sceneMgr->change_sceneFarTerr( "keyani\\nlxq\\nlxq_terr.osa" );
	}
	else
	{
		t_b = false;
		g_sceneMgr->change_sceneFarTerr( "keyani\\vilg\\dbmc\\vilg_dbmc_terr.osa" );
	}

	//os_particleCreate    t_pcreate;
	//t_pcreate.m_bAutoDelete = true;
	//t_pcreate.m_bCreateIns = true;



	//
	//strcpy( t_pcreate.m_szParticleName,"particle\\s_lvwu01.fdp" );//S_Nhuo 

	//t_pcreate.m_vec3Offset = g_cHero.m_verPos;
	//t_pcreate.m_vec3Offset.y += 2.0f;


	//t_pcreate.m_fSizeScale = 1.0f;
	//
	// g_sceneMgr->create_particle( t_pcreate );



}

//! 测试阴影相关。
void   test_shadow( void )
{
	static bool  t_sb = true;

	os_equipment   t_equ;

	if( t_sb )
	{
		t_equ.m_iCPartNum = 1;
	//	t_equ.m_arrId[0] = 6;
	//	t_equ.m_arrMesh[0] = 3;
	//	t_equ.m_arrSkin[0] = 1;

	//	g_cHero.m_pObj->change_equipment( t_equ );
	//

	//	t_sb = false;
	//}
	//else
	//{
	//	t_equ.m_iCPartNum = 1;
	//	t_equ.m_arrId[0] = 6;
	//	t_equ.m_arrMesh[0] = 0;
	//	t_equ.m_arrSkin[0] = 1;

		g_cHero.m_pObj->change_equipment( t_equ );

		t_sb = true;
	}
	
}



//! 从ini文件中读入相应的装备切换数据。
void  equip_change( void )
{
	static int      t_iTimes = 0;

	static int       t_iChangeTimes = 0;
	static VEC_int   t_vecMeshPart;
	static VEC_int   t_vecMeshIdx;
	
	if( (t_iTimes>0) && (t_iTimes >= t_iChangeTimes ) )
		return;

	if( t_iTimes == 0 )
	{
		CIniFile   t_file;

		t_file.OpenFileRead( "tequip.ini" );
		t_file.ReadSection( "equip" );
		t_file.ReadLine( "equipNum",&t_iChangeTimes );

		t_vecMeshPart.resize( t_iChangeTimes );
		t_vecMeshIdx.resize( t_iChangeTimes );

		
		char    t_szLine[128];
		for( int t_i=0;t_i<t_iChangeTimes;t_i ++ )
		{
			sprintf( t_szLine,"%s%d","mpart",t_i );
			t_file.ReadLine( t_szLine,&t_vecMeshPart[t_i] );
			sprintf( t_szLine,"%s%d","midx",t_i );
			t_file.ReadLine( t_szLine,&t_vecMeshIdx[t_i] );
		}
	}

	// 进行切换w
	os_equipment     t_newEquip;

	t_newEquip.m_iCPartNum = 1;
	t_newEquip.m_arrId[0] = t_vecMeshPart[t_iTimes];
	t_newEquip.m_arrMesh[0] = t_vecMeshIdx[t_iTimes];
	t_newEquip.m_arrSkin[0] = 1;

	osDebugOut( "The part <%d> The idx <%d> changed....\n",
		t_newEquip.m_arrId[0],t_newEquip.m_arrMesh[0] );

	g_cHero.m_pObj->change_equipment( t_newEquip );

	t_iTimes ++;

}



//! 更换人物的装备。
void change_weapon( void )
{
	os_equipment     t_newEquip;
	static bool t_b = true;

	if( t_b )
	{
		t_b = false;

		t_newEquip.m_iCPartNum = 1;
		t_newEquip.m_arrId[0] = 3;
		t_newEquip.m_arrMesh[0] = 3;
		t_newEquip.m_arrSkin[0] = 1;

	}
	else
	{
		t_b = true;
		t_newEquip.m_iCPartNum = 1;
		t_newEquip.m_arrId[0] = 3;
		t_newEquip.m_arrMesh[0] = 1;
		t_newEquip.m_arrSkin[0] = 1;

	}

	osassert( g_cHero.m_pObj );
	g_cHero.m_pObj->change_equipment( t_newEquip );

}



//! 场景退出前的处理。
void   process_beforeExit( void )
{
	FILE*  t_file;
	
	t_file = fopen( "default.pos","wb");
	if( !t_file )
		osassert( false );

	int   t_x,t_y;
	t_x = g_cHero.m_nFieldX*128+g_cHero.m_sBaseMesh.nX;
	t_y = g_cHero.m_nFieldY*128+g_cHero.m_sBaseMesh.nY;

	fwrite( &t_x,sizeof( int ),1,t_file );
	fwrite( &t_y,sizeof( int ),1,t_file );

	fclose( t_file );

}


npc_adjust::npc_adjust()
{
	this->m_iCurActiveNpc = 0;
}


//! 从ini文件中调入我们的npc
bool npc_adjust::load_npcFromIni( char* _iniName )
{
	guard;

	CIniFile  t_file;
	char  t_chDir[256];



	return true;

	unguard;
}

//! 写入npc人物的ini文件. 
void npc_adjust::save_npcChrToFile( char* _iniName )
{
	guard;
	CIniFile  t_file;
	char  t_chDir[256];

	t_file.OpenFileWrite( _iniName );

	for( int t_i=0;t_i<6;t_i++ )
	{
		s_string   t_str = "chr";
		char       t_szSection[10];
		sprintf( t_szSection,"%s%d",t_str.c_str(),t_i+1);

		t_file.WriteSection( t_szSection );
		t_file.WriteLine( "dir",m_sNpcStruct[t_i].m_szDir );
		t_file.WriteLine( "act1",m_sNpcStruct[t_i].m_szActName[0] );
		t_file.WriteLine( "act2",m_sNpcStruct[t_i].m_szActName[1] );
		t_file.WriteLine( "act3",m_sNpcStruct[t_i].m_szActName[2] );
		t_file.WriteLine( "insertx",m_sNpcStruct[t_i].m_iInsertX );
		t_file.WriteLine( "insertz",m_sNpcStruct[t_i].m_iInsertZ );
		t_file.WriteLine( "rotagl",m_sNpcStruct[t_i].m_fRotAgl );

		t_file.WriteLine( "xoffset",m_sNpcStruct[t_i].m_vec3Offset.x );
		t_file.WriteLine( "yoffset",m_sNpcStruct[t_i].m_vec3Offset.y );
		t_file.WriteLine( "zoffset",m_sNpcStruct[t_i].m_vec3Offset.z );
		t_file.WriteLine( "cam",m_sNpcStruct[t_i].m_szCamName );
	

	}

	t_file.CloseFile();

	MessageBox( NULL,"存储文件到chr.ini成功","INFORMATION",MB_OK );

	return ;


	unguard;
}

//! 重设人物的动作。
void npc_adjust::reset_npcAction( void )
{
	guard;

	npc_struct*   t_ptrNpc;

	t_ptrNpc = &m_sNpcStruct[m_iCurActiveNpc];

	osassert( t_ptrNpc->m_ptrSm );

	os_newAction   t_newAct;

	t_newAct.m_changeType = OSE_DIS_OLDACT;
	t_newAct.m_iNewAct = -1;
	t_newAct.m_iPlayNum = 1;
	strcpy( t_newAct.m_strActName,t_ptrNpc->m_szActName[0] );

	t_ptrNpc->m_ptrSm->change_skinMeshAction( t_newAct );


	unguard;
}


//! 测试人物的第二三套动作.
void npc_adjust::play_selectAction( void )
{
	guard;

	npc_struct*   t_ptrNpc;

	t_ptrNpc = &m_sNpcStruct[m_iCurActiveNpc];

	osassert( t_ptrNpc->m_ptrSm );

	os_newAction   t_newAct;


# if 1
	t_newAct.m_changeType = OSE_DIS_NEWACT_LASTFRAME;//OSE_DIS_NEWACT_TIMES;
	t_newAct.m_iNewAct = -1;
	t_newAct.m_iPlayNum = 1;
	
	strcpy( t_newAct.m_strActName,t_ptrNpc->m_szActName[1] );
	strcpy( t_newAct.m_strNextActName,t_ptrNpc->m_szActName[2] );
# else

	t_newAct.m_changeType = OSE_DIS_NEWACT_LASTFRAME;
	t_newAct.m_iNewAct = -1;
	t_newAct.m_iPlayNum = 1;
	strcpy( t_newAct.m_strActName,t_ptrNpc->m_szActName[1] );

# endif 

	t_ptrNpc->m_ptrSm->change_skinMeshAction( t_newAct );


	unguard;
}


//! 使下一个人物变为可调的人物.
void npc_adjust::active_nextNpc( void )
{
	m_iCurActiveNpc++;
	if( m_iCurActiveNpc>=6 )
		m_iCurActiveNpc = 0;

	while( (m_sNpcStruct[m_iCurActiveNpc].m_ptrSm == NULL) )
	{
		m_iCurActiveNpc++;
		if( m_iCurActiveNpc>=6 )
			m_iCurActiveNpc = 0;
	}

# if VERSION_SELCHR
	// 
	set_camera( true );
# endif 
	
	return;
}

//! 旋转当前的npc.
void npc_adjust::rotate_curNpc( float _radian )
{
	npc_struct*   t_ptrNpc;

	t_ptrNpc = &m_sNpcStruct[m_iCurActiveNpc];

	osassert( t_ptrNpc->m_ptrSm );

	t_ptrNpc->m_fRotAgl += _radian;
	
	osVec3D   t_vec3;
	float     t_f;

	t_ptrNpc->m_ptrSm->get_posAndAgl( t_vec3,t_f );
	t_ptrNpc->m_ptrSm->frame_move( t_vec3,t_ptrNpc->m_fRotAgl );

}


//! 在y方向上移动npc.
void npc_adjust::move_npcY( float _dis )
{
	npc_struct*   t_ptrNpc;

	t_ptrNpc = &m_sNpcStruct[m_iCurActiveNpc];
	osassert( t_ptrNpc->m_ptrSm );

	t_ptrNpc->m_vec3Offset.y += _dis;
	
	osVec3D   t_vec3;
	float     t_f;

	t_ptrNpc->m_ptrSm->get_posAndAgl( t_vec3,t_f );
	t_vec3.y += _dis;
	t_ptrNpc->m_ptrSm->frame_move( t_vec3,t_f );

}


//! 在X方向上移动npc.
void npc_adjust::move_npcX( float _dis )
{
	npc_struct*   t_ptrNpc;

	t_ptrNpc = &m_sNpcStruct[m_iCurActiveNpc];
	osassert( t_ptrNpc->m_ptrSm );

	t_ptrNpc->m_vec3Offset.x += _dis;
	
	osVec3D   t_vec3;
	float     t_f;

	t_ptrNpc->m_ptrSm->get_posAndAgl( t_vec3,t_f );
	t_vec3.x += _dis;
	t_ptrNpc->m_ptrSm->frame_move( t_vec3,t_f );
}


// ! 在Z方向上移动npc.
void npc_adjust::move_npcZ( float _dis )
{
	npc_struct*   t_ptrNpc;

	t_ptrNpc = &m_sNpcStruct[m_iCurActiveNpc];

	t_ptrNpc->m_vec3Offset.z += _dis;
	
	osVec3D   t_vec3;
	float     t_f;

	t_ptrNpc->m_ptrSm->get_posAndAgl( t_vec3,t_f );
	t_vec3.z += _dis;
	t_ptrNpc->m_ptrSm->frame_move( t_vec3,t_f );

}

//! 设置当前人物相关联的相机。
void npc_adjust::set_camera( bool _ani )
{
	if( _ani )
		g_cCamera.m_pCamera->load_camera( m_sNpcStruct[m_iCurActiveNpc].m_szCamName,true,0.3f );
	else
		g_cCamera.m_pCamera->load_camera( m_sNpcStruct[m_iCurActiveNpc].m_szCamName );
}



//os_skinMeshInit      g_smInit;

//! 测试人物的一次性播放动作。
void    t_action( void )
{
/*
	os_newAction   t_newAct;

	t_newAct.m_changeType = OSE_DIS_NEWACT_TIMES;
	t_newAct.m_iPlayNum = 1;
	t_newAct.m_iNewAct = -1;
	strcpy( t_newAct.m_strActName,"heloo" );
	
	g_cHero.m_pObj->change_skinMeshAction( t_newAct );
*/
/*
	//
	// 测试创建诗人相关。
	g_smInit.m_bShadowSupport = false;
	g_smInit.m_bWriteZbuf = false;
	g_smInit.m_iActIdx = 0;
	g_smInit.m_iTotalActTypeNum = 6;
	strcpy( g_smInit.m_strSMDir,"character\\bianyjs" );

	g_smInit.m_smObjLook.m_iPartNum = 1;
	g_smInit.m_smObjLook.m_mesh[0] = 1;
	g_smInit.m_smObjLook.m_mesh[0] = 1;

	g_smInit.m_smObjLook.m_mesh[1] = 1;
	g_smInit.m_smObjLook.m_mesh[1] = 1;

	g_smInit.m_smObjLook.m_mesh[2] = 1;
	g_smInit.m_smObjLook.m_mesh[2] = 1;
*/

	//float  t_f;
	//g_cHero.m_pObj->get_posAndAgl( g_smInitOne.m_vec3Pos,t_f );
	//	
	//g_smInitOne.m_bShadowSupport = false;
	//if( t_sSmPtr == NULL )
	//{
	//	t_sSmPtr = g_sceneMgr->create_character( &g_smInitOne );
	//}
	//else
	//{
	//	/*  
	//	os_equipment   t_equ;

	//	t_equ.m_iCPartNum = 1;
	//	t_equ.m_arrId[0] = 2;
	//	t_equ.m_arrMesh[0] = 1;
	//	t_equ.m_arrSkin[0] = 1;

	//	t_sSmPtr->change_equipment( t_equ );
	//	*/
	//	osVec3D    t_vec3Scale( 1.2f,1.2f,1.2f );

	//	t_sSmPtr->scale_skinMesh( t_vec3Scale );

	//}
	////g_cHero.m_pObj->set_hideSkinMesh( true );


/*
	g_smInit.m_vec3Pos.x += 1.0f;
	g_smInit.m_smObjLook.m_mesh[2] = 2;
	g_smInit.m_smObjLook.m_skin[2] = 1;

	g_smInit.m_smObjLook.m_mesh[3] = 2;
	g_smInit.m_smObjLook.m_skin[3] = 1;

	g_smInit.m_smObjLook.m_mesh[6] = 10;
	g_smInit.m_smObjLook.m_skin[6] = 1;
	
	g_smInit.m_smObjLook.m_mesh[5] = 2;
	g_smInit.m_smObjLook.m_skin[5] = 4;

	g_smInit.m_smObjLook.m_mesh[4] = 2;
	g_smInit.m_smObjLook.m_skin[4] = 1;

	g_sceneMgr->create_character( &g_smInit );
*/
}


void  play_music( char* _filename )
{
# if MUSIC_TEST

	static bool  t_bFirst = true;
	DWORD        t_dwTimeS,t_dwTimeE;
	if( t_bFirst )
	{
		t_bFirst = false;

		t_dwTimeS = timeGetTime();
		
		g_ptrSMgr->StopMP3();
		g_ptrSMgr->PlayFileMP3( "music\\land.mp3", 128 );
		t_dwTimeE = timeGetTime();
	}
	else
	{
		t_dwTimeS = timeGetTime();

		g_ptrSMgr->StopMP3();
		g_ptrSMgr->PlayFileMP3( _filename );

		t_dwTimeE = timeGetTime();

		t_bFirst = true;
	}

	osDebugOut( "The time is:<%d>...\n",t_dwTimeE-t_dwTimeS );


# endif 
}



void  play_sound( char* _sname )
{
	os_initSoundSrc t_buffer;
	float       t_fAgl;
	osVec3D     t_vec3;


	g_cHero.m_pObj->get_posAndAgl( t_vec3,t_fAgl );

# if MUSIC_TEST	
/*
	// 先设置听者位置.
	os_lisenerParam  t_listen;
	//g_cCamera.m_pCamera->get_curpos( &t_listen.m_vPosition );
	t_listen.m_vPosition = t_vec3;
	g_cCamera.m_pCamera->get_godLookVec( t_listen.m_vOrientFront );
	t_listen.m_vOrientTop = osVec3D( 0,1,0 );

	g_ptrSMgr->SetListenerPara( t_listen );
*/


	// 播放三维声音.
	t_buffer.m_flMaxDistance = 50.0f;
	t_buffer.m_flMinDistance = 20.0f;
	t_buffer.m_lVolume = 0;//!最大音量是0,最小是-5000


	t_buffer.m_vPosition = t_vec3;

# if 0
	osDebugOut( "The pos is:<%f,%f,%f>..The anle is:<%f>...\n",
		t_vec3.x,t_vec3.y,t_vec3.z,t_fAgl );
# endif 

	g_ptrSMgr->PlayFromFile( "sound\\c_run.wav",&t_buffer,false );//_sname

	//g_ptrSMgr->PlayFileMP3( "music\\land.mp3" );

# endif 

}

void  adj_volume( int _mm, OS_SOUNDTYPE type /*= SOUNDTYPE_UI*/ )
{
	I_soundManager*   t_sndPtr = ::get_soundManagerPtr();
	static bool       t_bFirst = true;
	static int        t_iVolume = 0;


	if( t_bFirst )
	{
		t_bFirst = false;
		t_sndPtr->SetSoundVolume( type, 100 );

		t_iVolume = t_sndPtr->GetSoundVolume( type );

		t_sndPtr->PlayFromFile( "sound\\c_run.wav",NULL,false,true );

	}

	t_iVolume += _mm;

	t_sndPtr->SetSoundVolume( type, t_iVolume );

	t_sndPtr->PlayFromFile( "sound\\ui_button.wav",NULL,false,true );

	osDebugOut( "The new Sound volume is:<%d>..\n",t_iVolume );

	//t_sndPtr->SetVolumeMP3( t_iVolume );

}



//! 测试地图的瞬移接口。 
void move_to_fmna( void )
{

	int   t_xPos[2] = { 128,128 };
	int   t_zPos[2] = { 128,128 };
	s_string  t_szMapName[2] = { "zyld","test" };
	static t_idx = 1;

	t_idx ++;
	if( t_idx > 1 )
		t_idx = 0;

	g_sceneMgr->spacemove_process( 
		(char*)t_szMapName[t_idx].c_str(),t_xPos[t_idx],t_zPos[t_idx] );


	osVec3D   t_pos;


	g_sceneMgr->get_tilePos( t_xPos[t_idx],t_zPos[t_idx],t_pos );  
	g_cHero.m_sBaseMesh.nX = t_xPos[t_idx] % 128;
	g_cHero.m_sBaseMesh.nY = t_zPos[t_idx] % 128;
	g_cHero.m_nFieldX = t_xPos[t_idx] / 128;
	g_cHero.m_nFieldY = t_zPos[t_idx] / 128;

	g_cHero.m_verPos = t_pos;
	g_cHero.m_pObj->frame_move( t_pos,0.0f );
	g_cCamera.Update();


}


//! test color.
void               test_color( void )
{

}


//! 测试细节格子.
void                test_detailTile( void )
{
	osVec3D    t_vec3;
	float      t_f;
	static int  t_posx = 0;

# if 0

	t_posx ++;
	if( t_posx >= 15 )
		t_posx = 0;

	g_cHero.m_pObj->get_posAndAgl( t_vec3,t_f );
	g_sceneMgr->get_detailTilePos( 
		(g_cHero.m_nFieldX*128 + g_cHero.m_sBaseMesh.nX)*15+t_posx,
		(g_cHero.m_nFieldY*128 + g_cHero.m_sBaseMesh.nY)*15+3,
		t_vec3 );
	g_cHero.m_pObj->frame_move( t_vec3,t_f );

# else

	osn_mathFunc::osCreateProcess( "dbgView.exe" );

# endif 

}

void                       mouse_rotCam( WPARAM _wParam,int _nX,int _nY )
{


	//@{
	// 处理鼠标右键事件
	static	int	t_iMouseX	= _nX;
	static	int	t_iMouseY	= _nY;
	// 如果鼠标移动是 鼠标的右键处于按下的状态
	if( _wParam == MK_RBUTTON && g_sceneMgr )
	{
		int  t_iOffsetY,t_iOffsetX;
		t_iOffsetY = _nY - t_iMouseY;
		t_iOffsetX = (_nX-t_iMouseX);

		//竖直旋转屏幕
		if( t_iOffsetY != 0 )
			picth_cam( t_iOffsetY*0.0025 );

		//水平旋转屏幕
		if( t_iOffsetX != 0 )
			rotate_cam( t_iOffsetX*0.0025 );
	}
		

	t_iMouseX	= _nX;
	t_iMouseY	= _nY;
	//@}

	// 是否点击中bbox.
	// TEST CODE:
	// 是否和人物的Bounding Box相交。
	osVec3D    t_vecStart,t_vecDir;
	osVec2D    t_vec2D;
	osMatrix   t_mat;
	float      t_f;

	osMatrixIdentity( &t_mat );
	if( g_cCamera.m_pCamera )
	{
		g_cCamera.m_pCamera->get_curpos( &t_vecStart );
		t_vec2D.x = _nX;t_vec2D.y = _nY;
		t_vecDir = g_cCamera.m_pCamera->get_upprojvec( &t_vec2D,t_mat );
	}


	osVec3D    t_heroPos ;
	if( t_bBoxSet && g_cHero.m_pObj )
	{
		t_bBoxSet = false;

		t_testBbox.set_bbMaxVec( osVec3D( 0.8,0.42,-0.09 ) );
		t_testBbox.set_bbMinVec( osVec3D( -0.5,-0.02,-1.53 ) );
		float  t_f;
		g_cHero.m_pObj->get_posAndAgl( t_heroPos,t_f );
		t_testBbox.set_bbPos( t_heroPos );
	}
	if( t_testBbox.ray_intersect( t_vecStart,t_vecDir,t_f ) )
		t_boxColor = 0xff00ffff;
	else
		t_boxColor = 0xffff0000;

}

void                move_pitchTest( void )
{
	g_cCamera.m_pCamera->animate_camRotPicth( 0.0f,0.4f,0.4f );
}



void                light_test( void )
{
	float            t_f;
	os_dlightStruct   t_dl;

	t_dl.m_fRadius = 5.0f;
	t_dl.m_lightColor = 0xffffff00;

	g_cHero.m_pObj->get_posAndAgl( t_dl.m_vec3Pos,t_f );

	t_siDLId = g_sceneMgr->create_dynamicLight( t_dl );


}
void                delete_light( void )
{
	if( t_siDLId >= 0 )
	{
		g_sceneMgr->delete_dynamicLight( t_siDLId );
		t_siDLId = -1;
	}
}

//! 测试人物的刀光设置
void                test_herog( void )
{
	int     t_iActIdx[2] = { 4,5 };    
	DWORD   t_dwColor[2] = { 0xffffffff,0xffffffff };


# if __GLOSSY_EDITOR__

	g_cHero.m_pObj->set_swordGlossyActIdx( 2,t_iActIdx,t_dwColor,"glossy" );

	g_cHero.m_pObj->set_displayWeapon( true );

	g_cHero.m_pObj->save_weaponFile();

# endif 

}



void                t_camera( void )
{
	osVec3D    t_vec3;
	float      t_f;


	g_camera->get_curpos( &t_vec3 );
	t_vec3.x += 0.2f;
	g_camera->set_curpos( &t_vec3 );

	g_camera->get_camFocus( t_vec3 );

	osDebugOut( "The focus Before is:<%f,%f,%f>..\n",
		t_vec3.x,t_vec3.y,t_vec3.z );

	t_vec3.x += 0.2f;
	g_camera->set_camFocus( t_vec3 );
	
	g_camera->get_camFocus( t_vec3 );
	osDebugOut( "The focus After is:<%f,%f,%f>..\n",
		t_vec3.x,t_vec3.y,t_vec3.z );

	g_cHero.m_pObj->get_posAndAgl( t_vec3,t_f );
	t_vec3.x += 0.2f;
	g_cHero.m_pObj->frame_move( t_vec3,t_f );

}


void  test_bolt1( void )
{
# if 1
	osVec3D   t_vec3,t_vec3tmp;
	float     t_f;

	static int t_siIdx = -1;

	if( t_siIdx == -1 )
	{

		g_cHero.m_pObj->get_posAndAgl( t_vec3,t_f );
		t_vec3.y += 2.0f;
		t_vec3tmp = t_vec3;
		t_vec3tmp.x -= 32.0f;

		os_boltInit   t_bi[2];

		t_bi[0].m_dwColor = 0xffff0000;
		t_bi[0].m_fWidth = 0.10f;
		t_bi[0].m_fWildness = 0.08f;
		t_bi[0].m_iNumPt = 32;
		t_bi[0].m_iBoltFreq = 10;
		t_bi[0].m_vec3Start = t_vec3;
		t_bi[0].m_vec3End = t_vec3tmp;

		t_bi[1].m_dwColor = 0x4444ffff;
		t_bi[1].m_fWidth = 0.05f;
		t_bi[1].m_fWildness = 0.14f;
		t_bi[1].m_iNumPt = 32;
		t_bi[1].m_iBoltFreq = 20;
		t_bi[1].m_vec3Start = t_vec3;
		t_bi[1].m_vec3End = t_vec3tmp;


	
		t_siIdx = g_sceneMgr->create_eleBolt( t_bi,2 );
	}
	else
	{
		g_cHero.m_pObj->get_posAndAgl( t_vec3,t_f );
		t_vec3.y += 2.0f;
		t_vec3tmp = t_vec3;
		t_vec3tmp.x += 4.0f;

		os_boltInit   t_bi[2];

		t_bi[0].m_dwColor = 0xffffffff;
		t_bi[0].m_fWidth = 0.15f;
		t_bi[0].m_fWildness = 0.25f;
		t_bi[0].m_iNumPt = 16;
		t_bi[0].m_iBoltFreq = 10;
		t_bi[0].m_vec3Start = t_vec3;
		t_bi[0].m_vec3End = t_vec3tmp;

		t_bi[1].m_dwColor = 0xffffffff;
		t_bi[1].m_fWidth = 0.05f;
		t_bi[1].m_fWildness = 0.7f;
		t_bi[1].m_iNumPt = 16;
		t_bi[1].m_iBoltFreq = 20;
		t_bi[1].m_vec3Start = t_vec3;
		t_bi[1].m_vec3End = t_vec3tmp;

		g_sceneMgr->delete_eleBolt( t_siIdx );
		t_siIdx = -1;
	
	}

# else
	static bool t_b = false;


	g_bDisplayStaticMesh = t_b ;
	t_b = !t_b;

# endif 


}


void test_actionSuspend( void )
{
	static bool t_b = true;

# if __GLOSSY_EDITOR__
	g_cHero.m_pObj->suspend_curAction( t_b );
	t_b = !t_b;
# endif 

}

void next_actPose( void )
{
# if __GLOSSY_EDITOR__
	g_cHero.m_pObj->next_actFramePose( true );
# endif 
}

void bef_actPose( void )
{
# if __GLOSSY_EDITOR__
	g_cHero.m_pObj->next_actFramePose( false );
# endif 	
}


s_string   g_szChrName;

//! 装备列表文件的文件头数据。
# define    EQUIPLIST_MAGIC   "equ"
# define    EQUIPLIST_VERSION  1


//! 动作改变相关的数据结构。
struct gcs_actionEle
{
	char      m_szActName[32];
	int       m_iActNum;
	int       m_vecActId[4];

	//! 从内存中调入当前的动作信息。
	void       load_fromMem( BYTE*& _mstart )
	{
		READ_MEM_OFF( m_szActName,_mstart,32 );
		READ_MEM_OFF( &this->m_iActNum,_mstart,sizeof( int ) );
		osassert( m_iActNum < 4 );
		READ_MEM_OFF( m_vecActId,_mstart,sizeof( int )*m_iActNum );
	}
};
typedef std::vector<gcs_actionEle>  VEC_actEle;


/** \brief
 *  装备的显示数据，可以在人物身上体现出来的数据。
 *  
 */
struct   gcs_equipDis
{
	//  ATTENTION TO OPP: 去掉名字相关的数据结构。
	//! 当前装备的名字。
	char          m_szEquipName[64];

	//! 需要多少个部分更换显示相关资源。
	int           m_iCPartNum;

	//! 需要替换的部位的id.
	int           m_arrId[MAX_SKINPART];
	//! 需要替换的meshId.
	int           m_arrMesh[MAX_SKINPART];
	//! 需要替换的skinId.
	int           m_arrSkin[MAX_SKINPART];

	//@{
	//! 需要变化的动作数目。
	int           m_iCActNum;
	VEC_actEle     m_vecActNeedChange;
	//@}

};

//! 调入显示相关的数据。
void load_disEquip( const char* _name,gcs_equipDis* _dis )
{
	guard;

	int     t_size,t_iv;
	BYTE*   t_fstart;
	char    t_str[4];

	if ( !file_exist( (char*)_name ) )
	{
		osassert( false );
		return;
	}

	t_fstart = START_USEGBUF;
	t_size = read_fileToBuf( (char*)_name, t_fstart, TMP_BUFSIZE );
	osassert( t_size>=0 );

	//
	// 读入文件头.
	READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
	if( strcmp( t_str, EQUIPLIST_MAGIC ) )
		osassert( false );

	READ_MEM_OFF( &t_iv, t_fstart, sizeof( int ) );

	//
	// ATTENTION TO OPP:去掉这个数据结构。
	// 读入装备的名字,使用64个字节没有意思，改正。
	READ_MEM_OFF( _dis->m_szEquipName,t_fstart,64 );

	//
	// 读入当前装备的id号，没有意义，上层已经处理过。
	READ_MEM_OFF( &t_size,t_fstart,sizeof( int ) );


	//
	// 读入动作相关数据。
	if( t_iv < 102 )
	{
		READ_MEM_OFF( &_dis->m_iCActNum,t_fstart,sizeof( int ) );
		osassert( _dis->m_iCActNum >= 0 );
		_dis->m_vecActNeedChange.resize( _dis->m_iCActNum );
		for( int t_i=0;t_i<_dis->m_iCActNum;t_i++ )
			_dis->m_vecActNeedChange[t_i].load_fromMem( t_fstart );
	}


	//
	// 读入人物身体部位替换的相关数据。
	READ_MEM_OFF( &_dis->m_iCPartNum,t_fstart,sizeof( int ) );
	osassert( _dis->m_iCPartNum>=0 );
	t_size = sizeof( int )*_dis->m_iCPartNum;
	READ_MEM_OFF( _dis->m_arrId,t_fstart,t_size );
	READ_MEM_OFF( _dis->m_arrMesh,t_fstart,t_size );
	READ_MEM_OFF( _dis->m_arrSkin,t_fstart,t_size );

	if( t_iv < 103 )
	{
		for( int t_i=0;t_i<_dis->m_iCPartNum;t_i ++ )
		{
			_dis->m_arrId[t_i] = _dis->m_arrId[t_i] + 1;
			_dis->m_arrMesh[t_i] = _dis->m_arrMesh[t_i] + 1; 
			_dis->m_arrSkin[t_i] = _dis->m_arrSkin[t_i] + 1;
		}
	}

	END_USEGBUF;


	unguard;
}


//! 调入装备文件
void load_equFile( void )
{
	static int  t_idx = 0;
	gcs_equipDis    t_dis;
	
	s_string   t_szName ;

	t_szName = SHADEDIR;
	t_szName += g_szChrName;
	t_szName += "\\";
	t_szName += g_equFile[t_idx].c_str();

	load_disEquip( t_szName.c_str(),&t_dis );

	os_equipment   t_equip;

	t_equip.m_iCPartNum = t_dis.m_iCPartNum;
	for( int t_i=0;t_i<t_equip.m_iCPartNum;t_i ++ )
	{
		t_equip.m_arrId[t_i] = t_dis.m_arrId[t_i];
		t_equip.m_arrMesh[t_i] = t_dis.m_arrMesh[t_i];
		t_equip.m_arrSkin[t_i] = t_dis.m_arrSkin[t_i];
	}

	g_cHero.m_pObj->change_equipment( t_equip );

	t_idx ++;
	if( t_idx >= g_equFile.size() )
		t_idx = 0;
}


// TEST CODE:
void   test_chunk( void )
{
	guard;
	
	static float t_fRot = 0.0f;

	os_decalInit   t_init;

	static int t_id = 0;
	static osVec3D   t_pos;

	t_init.m_vec3Pos = g_cHero.m_verPos;
	t_init.m_fRotAgl = t_fRot;
	t_init.m_fSize = 4.0f;
	strcpy( t_init.m_szDecalTex,"groupAttack" );
	t_init.m_fVanishTime = 0.f;



	if( t_id == 0 )
	{	
		t_pos = t_init.m_vec3Pos;

		t_id = g_sceneMgr->create_decal( t_init );
	}
	else
	{
		t_pos.x += 0.5f;
		g_sceneMgr->reset_decalPos( t_id,t_pos );
	}


	t_fRot += 0.5f;

	unguard;
}




/** \brief
*  得到顶点投影到一个面上得到的顶点.
*
*  算法描述:
*
*  找出当前顶点和面之间的距离,使当前面的法向量乘以这个距离,
*  以当前的顶点加上乘后的向量,就得到这个顶点在面的投影点.
*
*  使用这个函数，也可以得到一个向量在一个面的投影向量。
*  
*/
void project_PtToPlane( osPlane& _plane,osVec3D& _srcpt,osVec3D& _respt )
{
	guard;

	osVec3D      t_vec;
	float        t_f;

	t_f = osVec3Dot( &_srcpt,(osVec3D*)&_plane ) + _plane.d;
	t_vec.x = -_plane.a*t_f;t_vec.y = -_plane.b*t_f;t_vec.z = -_plane.c*t_f;
	_respt = _srcpt + t_vec;

	return;

	unguard;
}


/** \brief
*  得到顶点是不是在同一条直线上.
*/
bool get_coline( osVec3D* _vec,int _vecnum )
{
	guard;

	osVec3D      tmpvec1,tmpvec2,tmpvec3;
	int          tmpi;

	osassert( (_vecnum>=3) );

	tmpvec1 = _vec[1] - _vec[0];
	osVec3Normalize( &tmpvec1,&tmpvec1 );

	for( tmpi=2;tmpi<_vecnum;tmpi++ )
	{
		tmpvec2 = _vec[tmpi]-_vec[tmpi-1];
		osVec3Normalize( &tmpvec2,&tmpvec2 );

		tmpvec3 = -tmpvec2;
		if( !(  ( vec3_equal( tmpvec1,tmpvec2 ) )||
			    ( vec3_equal( tmpvec1,tmpvec3 ) ) )  )
				return false;

	}

	return true;

	unguard;
}

/** \brief
*  得到一个顶点在一条直线上的投影顶点
*
*  \param osVec3D& _v 要投影的顶点
*  \param osVec3D& _res 返回的投影顶点。
*  \param osVec3D& _s   直线的开始点顶点。
*  \param osVec3D& _e   直线的结速顶点。
*  
*  算法描述：
*  先找出和_v顶点“平行”的包含_s和_e的面。然后投影_v顶点到这个面上，即为我们要找到的顶点。
*/
void project_PtToLine( osVec3D& _res,osVec3D& _v,osVec3D& _s,osVec3D& _e )
{
	osVec3D   t_vec3Tmp1,t_vec3Tmp2,t_vec3Right;
	osPlane   t_sPlane;

	osVec3D   t_vec3Tmp[3];

	t_vec3Tmp[0] = _v;
	t_vec3Tmp[1] = _s;
	t_vec3Tmp[2] = _e;
	if( get_coline( t_vec3Tmp,3 ) )
	{
		_res = _v;
		return;
	}

	t_vec3Tmp1 = _v - _s;
	t_vec3Tmp2 = _e - _s;
	osVec3Cross( &t_vec3Right,&t_vec3Tmp1,&t_vec3Tmp2 );
	osVec3Normalize( &t_vec3Right,&t_vec3Right );
	t_vec3Right += _s;
	osPlaneFromPoints( &t_sPlane,&_s,&_e,&t_vec3Right );

	project_PtToPlane( t_sPlane,_v,_res );

}


/** \brief
 *  对两个盒子进行对齐.是_b2盒子移动到_np2的位置上，跟_b1对齐。
 *
 *  _snap1: 在旋转轴上矩离最近的面对齐。
    _snap2: 中心点对齐到旋转轴上，
 *  _snap3: 最后是高度对齐,在高度上把两个盒子的底线保持到一个位置上。
 *
 *  算法描述：
 *  1: 计算出两个bbox要对齐的轴线。
 *  2: 计算第二个盒子在轴线向量上的可以跟第一个子接触的向量长度。对齐_snap1
 *  3: 把第二个盒子的中心点移动到两个bbox要对齐的轴线上，对齐_snap2
 *  4: 移动第二个盒子的中心点，把两个的盒子的Y方向最小值放到相同的Y方向水平线上。
 *   
 */
bool snap_bbox( sc_BBox _b1, sc_BBox _b2, D3DXVECTOR3& _np2,bool _snap1,bool _snap2,bool _snap3 )
{
	guard;

	osVec3D   t_vec3SnapAxis( 1.0f,0.0f,0.0f );
	osVec3D   t_vec3Box2ToBox1;
	osMatrix  t_sRotMat;
	osVec3D   t_vec3Box1Center,t_vec3Box2Center,t_vec3Box2COffset;
	osVec3D   t_vec3Box1Max,t_vec3Box1Min;
	osVec3D   t_vec3Box2Max,t_vec3Box2Min;


	//
	// 如果只需要在高度上对齐，则使用最简单的方式：
	if( (!_snap1)&&(!_snap2)&&(_snap3) )
	{
		float  t_fOffset = (_b2.vecmin.y + _b2.m_vec3BoxPos.y) - 
			               (_b1.vecmin.y + _b1.m_vec3BoxPos.y );

		_np2 = _b2.m_vec3BoxPos;
		_np2.y -= t_fOffset;

		return true;
	}

	osassertex( float_equal( _b1.m_fBoxRot,_b2.m_fBoxRot ),
		va( "两个盒子的旋转方向不一致,_b1<%f>,_b2<%f>..\n",_b1.m_fBoxRot,_b2.m_fBoxRot ) );
	osMatrixRotationY( &t_sRotMat,_b1.m_fBoxRot );

	//
	// 由于两个盒子都是绕盒子的位置进行旋转，先计算这两个盒子的中心点位置。
	t_vec3Box1Max = _b1.vecmax;
	t_vec3Box1Min = _b1.vecmin;
	t_vec3Box2Max = _b2.vecmax;
	t_vec3Box2Min = _b2.vecmin;

	t_sRotMat._41 = _b1.m_vec3BoxPos.x;
	t_sRotMat._42 = _b1.m_vec3BoxPos.y;
	t_sRotMat._43 = _b1.m_vec3BoxPos.z;
	osVec3Transform( &t_vec3Box1Max,&t_vec3Box1Max,&t_sRotMat );
	osVec3Transform( &t_vec3Box1Min,&t_vec3Box1Min,&t_sRotMat );

	t_sRotMat._41 = _b2.m_vec3BoxPos.x;
	t_sRotMat._42 = _b2.m_vec3BoxPos.y;
	t_sRotMat._43 = _b2.m_vec3BoxPos.z;
	osVec3Transform( &t_vec3Box2Max,&t_vec3Box2Max,&t_sRotMat );
	osVec3Transform( &t_vec3Box2Min,&t_vec3Box2Min,&t_sRotMat );

	t_vec3Box1Center = (t_vec3Box1Max + t_vec3Box1Min)/2.0f;
	t_vec3Box2Center = (t_vec3Box2Max + t_vec3Box2Min)/2.0f;


	// 
	// 重设盒子两个顶点值,用于接下来的长度等相关计算
	t_vec3Box1Max = _b1.vecmax;
	t_vec3Box1Min = _b1.vecmin;
	t_vec3Box2Max = _b2.vecmax;
	t_vec3Box2Min = _b2.vecmin;

	t_sRotMat._41 = t_sRotMat._42 = t_sRotMat._43 = 0.0f;
	osVec3Transform( &t_vec3SnapAxis,&t_vec3SnapAxis,&t_sRotMat );

	// 计算出要移动盒子的顶点位置和中心位置的偏移量
	t_vec3Box2COffset = t_vec3Box2Center - _b2.m_vec3BoxPos;


	t_vec3Box2ToBox1 = t_vec3Box2Center - t_vec3Box1Center;
	osVec3Normalize( &t_vec3Box2ToBox1,&t_vec3Box2ToBox1 );
	if( osVec3Dot( &t_vec3SnapAxis,&t_vec3Box2ToBox1 ) < 0.0f )
		t_vec3SnapAxis = -t_vec3SnapAxis;


	// 第一个层次的对齐
	osassertex( _snap1,"必须有最基本的对齐.." );

	float   t_fDis1 = (t_vec3Box1Max.x - t_vec3Box1Min.x)/2.0f;
	float   t_fDis2 = (t_vec3Box2Max.x - t_vec3Box2Min.x)/2.0f;

	osVec3D   t_vec3SnapAxisV1,t_vec3SnapAxisV2,t_vec3Tmp;
	t_vec3SnapAxisV1 = t_vec3Box1Center;
	t_vec3SnapAxisV2 = t_vec3SnapAxis + t_vec3SnapAxisV1;

	osn_mathFunc::project_PtToLine( t_vec3Tmp,t_vec3Box2Center,t_vec3SnapAxisV1,t_vec3SnapAxisV2 );
	_np2 = t_vec3Box1Center + t_vec3SnapAxis*(t_fDis1+t_fDis2);
	_np2 -= t_vec3Tmp;
	_np2 = t_vec3Box2Center + _np2;

	// 
	// 第二个层次的对齐:
	if( _snap2 )
	{
		osn_mathFunc::project_PtToLine( t_vec3Tmp,_np2,t_vec3SnapAxisV1,t_vec3SnapAxisV2 );
		_np2 = t_vec3Tmp;
	}

	// 最后的高度对齐
	if( _snap3 )
	{
		t_fDis1 = (t_vec3Box1Max.y - t_vec3Box1Min.y)/2.0f;
		t_fDis2 = (t_vec3Box2Max.y - t_vec3Box2Min.y)/2.0f;

		t_fDis2 -= t_fDis1;
	
		// 如果没有第二步的对齐，则需要在位置的高度也对齐.ATTENTION TO FIX:
		if( !_snap2 )
		{
			t_fDis1 = t_vec3Box2Center.y - t_vec3Box1Center.y;
			t_fDis2 -= t_fDis1;
		}

		_np2.y += t_fDis2;
	}

	// 计算中心点到位置的误差
	_np2 -= t_vec3Box2COffset;

	return true;

	unguard;
}

void   test_snapBbox( void )
{

	osVec3D   t_vec3BPos;
	os_bbox   t_box1,t_box2;
	
	t_box1.set_bbMaxVec( t_b1.vecmax );
	t_box1.set_bbMinVec( t_b1.vecmin );
	t_box1.set_bbPos( t_b1.m_vec3BoxPos );
	t_box1.set_bbYRot( t_b1.m_fBoxRot );

	t_box2.set_bbMaxVec( t_b2.vecmax );
	t_box2.set_bbMinVec( t_b2.vecmin );
	t_box2.set_bbPos( t_b2.m_vec3BoxPos );
	t_box2.set_bbYRot( t_b2.m_fBoxRot );

	//osn_mathFunc::snap_bbox( t_box1,t_box2,t_vec3BPos,true,true,true );

	snap_bbox( t_b1,t_b2,t_vec3BPos,false,false,true );

	t_b2.m_vec3BoxPos = t_vec3BPos;

}


void spacemove_test( void )
{
	osVec2D   t_vec2D;

	t_vec2D = g_sceneMgr->get_mapSize();

	int    t_x,t_z;

	t_x = getrand_fromintscope( 64,int((t_vec2D.x/1.5f)-64) );
	t_z = getrand_fromintscope( 64,int((t_vec2D.y/1.5f)-64) );

	osDebugOut( "The move pos is:<%d,%d>..\n",t_x,t_z );

	g_sceneMgr->spacemove_process( "zyla",t_x,t_z );

	osVec3D   t_pos;

	g_sceneMgr->get_tilePos( t_x,t_z,t_pos );  
	g_cHero.m_sBaseMesh.nX = t_x % 128;
	g_cHero.m_sBaseMesh.nY = t_z % 128;
	g_cHero.m_nFieldX = t_x / 128;
	g_cHero.m_nFieldY = t_z / 128;

	g_cHero.m_verPos = t_pos;
	g_cHero.m_pObj->frame_move( t_pos,0.0f );
	g_cCamera.Update();

}



