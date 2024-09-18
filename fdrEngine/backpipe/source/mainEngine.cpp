//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: mainEngine.cpp
 *
 *  Desc:     处理引擎主体框架,在此处实现得到引擎接口的函数.
 *
 *  His:      River created @ 4/16 2003    
 *
 *  成功是一个讨厌的教员,它诱使聪明人认为他们不会失败,它不是一位
 *  引导我们走向未来的可靠的向导.
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/mainEngine.h"
# include "../../mfpipe/include/osShader.h"

//@{
/** \brief
 *  是否使用全局的方向光,如果不使用全局的方向光的话，则场景中渲染物品的时候 
 *  不打开灯光，此时场景中需要所有的物品都使用点着色才能显示出灯光效果。
 */
OSENGINE_API BOOL         g_bUseGDirLight = TRUE;
/** \brief
 *  全局光的位置。
 *  
 *  就算场景中使用点着色的物品，也需要设置这个位置，这个位置人物渲染时灯光的默认位置。
 */
OSENGINE_API osVec3D      g_vec3LPos = osVec3D( 1.0f,1.0f,1.0f );;
//! 场景中全局的方向光的颜色。
OSENGINE_API float        g_fDirectLR = 0.8f;
OSENGINE_API float        g_fDirectLG = 0.8f;
OSENGINE_API float        g_fDirectLB = 0.8f;

//@} 
OSENGINE_API float        g_fBoltFanAngle = 0.25f;
OSENGINE_API char		  g_CurFarTerrain[255];
OSENGINE_API BOOL		  g_bHasBolt = false;


//! 是否使用HDR Light
OSENGINE_API BOOL         g_bUseHDRLight = FALSE; 

OSENGINE_API BOOL		  g_SceneSoundBox = false;

# define    ENGINE_DIS_VERSION       __DATE__
# define    ENGINE_DIS_TIME          __TIME__




os_newAction::os_newAction()
{
	m_strActName[0] = NULL;
	m_strNextActName[0] = NULL;

	// 随机播放的动作名字列表
	m_strRandActName[0][0] = NULL;
	m_strRandActName[1][0] = NULL;
	m_strRandActName[2][0] = NULL;

	m_bPlayLoopAct = false;

	m_fFrameTime = -1.0f;

	m_bStopAtStopFrame = false;
}




os_colorInterpretStr::os_colorInterpretStr()
{
	m_fZ = 0.0f;
	m_iPosX = 0;
	m_iPosY = 0;

	m_iMaxPixelX = -1;
	m_iMaxPixelY = -1;
	m_bForceDis = FALSE;

	// 行间矩默认为0.
	m_iRowDis = 0;

	m_szStaticSzBuf[0] = NULL;
	m_szCIStr = m_szStaticSzBuf;
}
os_colorInterpretStr& os_colorInterpretStr::operator = ( const char* _dstr )
{
	guard;

	osassert( _dstr );

	if( !_dstr[0] )
	{
		m_szCIStr = m_szStaticSzBuf;
		m_szStaticSzBuf[0] = NULL;
		return *this;
	}

	int   t_iSzSize = strlen( _dstr );
	if( t_iSzSize >= 256 )
	{
		m_szDynamicBuf = _dstr;
		m_szCIStr = m_szDynamicBuf.c_str();
	}
	else
	{
		m_szCIStr = m_szStaticSzBuf;
		strcpy( m_szStaticSzBuf,_dstr );
	}

	return *this;

	unguard;
}


//! 加入其它显示字串需要的数据.
os_stringDisp::os_stringDisp()
{
	z = 0.0f;
	// 默认为完全没有背景。
	m_dwBackColor = 0;
	//! 默认为空
	dstring = NULL;
	m_szStatic[0] = NULL;
}


//! 为当前的结构赋一个string的值.
os_stringDisp& os_stringDisp::operator = ( const char* _dstr )
{
	guard;

	osassert( _dstr );
	

	if( !_dstr[0] )
	{
		dstring = m_szStatic;
		m_szStatic[0] = NULL;
		return *this;
	}

	int   t_iSzSize = strlen( _dstr );
	if( t_iSzSize >= 32 ) 
	{
		m_szDynamic = _dstr;
		dstring = m_szDynamic.c_str();
	}
	else
	{
		dstring = m_szStatic;
		strcpy( m_szStatic,_dstr );
	}

	return *this;

	unguard;
}



// 初始化blend_mode 为3,顶点uv为从0到1.
os_screenPicDisp::os_screenPicDisp()
{
	blend_mode = 3;
	tex_id = -1;

	m_scrVer[0].m_dwDiffuse = 0xffffffff;
	m_scrVer[1].m_dwDiffuse = 0xffffffff;
	m_scrVer[2].m_dwDiffuse = 0xffffffff;
	m_scrVer[3].m_dwDiffuse = 0xffffffff;

	m_scrVer[0].m_vecUv = osVec2D( 0.0f,0.0f );
	m_scrVer[1].m_vecUv = osVec2D( 1.0f,0.0f );
	m_scrVer[2].m_vecUv = osVec2D( 1.0f,1.0f );
	m_scrVer[3].m_vecUv = osVec2D( 0.0f,1.0f );

	m_bUseZBuf = FALSE;

}



//! 默认参数设置
os_skinMeshInit::os_skinMeshInit()
{
	m_bWriteZbuf = true;
	// ...

	// 人物的微调数据。
	m_fAdjRotAgl = 0.0f;
	m_vec3AdjPos = osVec3D( 0.0f,0.0f,0.0f );
	m_vec3Max = osVec3D( 1.0f,2.0f,1.0f );
	m_vec3Min = osVec3D( -1.0f,0.0f,-1.0f );

	//! 默认使用名字来播放默认动作.
	m_iActIdx = -1;

	m_fAngle = 0.0f;

	m_bShadowSupport = false;

	// 默认使用正常的光
	m_bUseDirLAsAmbient = false;
	//@{
	// Windy mod @ 2005-8-30 19:07:19
	//Desc: ride 默认不是坐骑
	//osDebugOut("windy:开始坐骑初始化@=@");
	m_bIsRide = false;
	m_vec3RideOffset = osVec3D(0.0f,0.0f,0.0f);

	//@}

	// 默认创建的不是三层贴图的旗帜
	m_iMlFlagIdx = -1;
	memset(m_szspeFileName,0,sizeof(char)*64);
	m_szspeFileName[0] = '\0';

	m_Scale = 1.0f;

# if GALAXY_VERSION
	m_bSupportProjectShadow = false;
# endif 

# if __QTANG_VERSION__
	m_bInitFade = true;
# else
	m_bInitFade = false;
# endif 

	//! river added @ 2009-12-30:用于渲染二维界面上的人物特效
	m_bUseInInterface = false;

	m_bPlayWeaponEffect = false;

}

//! 设置
void os_skinMeshInit::load_equipment(const os_equipment* _equ)
{
	guard;

	osassert(_equ);
	osassert(_equ->m_iCPartNum <= MAX_SKINPART);
	osassert(m_smObjLook.m_iPartNum && m_smObjLook.m_iPartNum <= MAX_SKINPART);

	for(int i = 0;i < m_smObjLook.m_iPartNum;i++){
		m_smObjLook.m_mesh[i] = m_smObjLook.m_skin[i] = 1;
	}

	for(int i = 0;i < _equ->m_iCPartNum;i++){
		m_smObjLook.m_mesh[_equ->m_arrId[i]] = _equ->m_arrMesh[i];
		m_smObjLook.m_skin[_equ->m_arrId[i]] = _equ->m_arrSkin[i];
	}

	unguard;
}



//@{
//! 地表纹理shader.
int osc_engine::m_iTileShaId = -1;

//! 普通物品shader.
int osc_engine::m_iObjectShaId = -1;

//! skinMesh物品的Shader.
int osc_engine::m_iCharacterShaId = -1;

//! 公告板类型。
int osc_engine::m_iBillBoardShaId = -1;

//! 天空类型。
int osc_engine::m_iSkyShaId = -1;

//! 加入了环境贴图的skinMeshShader.
int osc_engine::m_iEnvCharacterShaId = -1;

//! 加入了第二层流动纹理的skinMeshShader.
int osc_engine::m_iSecCharacterShaId = -1;

//! River @ 2010-2-25:加入人物纹理自发光
int osc_engine::m_iSecCharacterShineId = -1;

//! River @ 2010-3-11: 加入人物的更多层贴图效果。
int osc_engine::m_iSecCharRotEnvId = -1;
int osc_engine::m_iSecCharShineRotId = -1;
int osc_engine::m_iSecCharShineEnvId = -1;



//! 第二层纹理的凹凸贴图skinMeshShader
int osc_engine::m_iBumpCharacterShaID = -1;

//@} 

//! 引擎内使用的假阴影用到的纹理和假阴影默认大小
char                     g_szFakeShadowTexture[128] = { NULL };
float                    g_fFakeShadowSize = 0.2f;


TiXmlDocument* osc_engine::m_shaderCfgDoc = NULL;

osc_engine::osc_engine()
{
	guard;

	m_3dDeviceMgr = NULL;

	m_socketMgr = NULL;

	m_bInit = false;

	unguard;
}

osc_engine::~osc_engine()
{
	release();
}


s_string osc_engine::get_shaderFile( const char* name )
{
	guard;
	
	if(m_shaderCfgDoc == NULL){
		// 为了在MFC中的内存泄漏检测不出错,不得已将其改成指针
		m_shaderCfgDoc = new TiXmlDocument;
	}
	
	TiXmlElement* elem;
	elem = m_shaderCfgDoc->FirstChildElement( name );
	if( !elem )
		return "";
	else
	{
		const char* file = elem->Attribute( "file" );
		if( !file )
			return "";
		else
			return file;
	}

	unguard;
}



# define MAX_SHADER_TEMPLATE   12
//! 初始化全局的Shader相关信息。
void osc_engine::initialize_shader( void )
{
	if(m_shaderCfgDoc == NULL){
		// 为了在MFC中的内存泄漏检测不出错,不得已将其改成指针
		m_shaderCfgDoc = new TiXmlDocument;
	}

	char* shaderCfg = "ini\\shaderInit.cfg";

	int		t_i = 0;
	int   t_iGBufIdx = -1;
	BYTE*	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	t_i = ::read_fileToBuf( shaderCfg, t_ptrStart, TMP_BUFSIZE );
	if( t_i > 0 )
	{
		t_ptrStart[t_i] = 0;
		m_shaderCfgDoc->Parse( (const char*)t_ptrStart );
		END_USEGBUF( t_iGBufIdx );
		//return ;
	}
	else
	{
		END_USEGBUF( t_iGBufIdx );
	}


	CIniFile     t_file;
	char         t_szFileName[128];
	char         t_szDDir[128];
	s_string     t_section[MAX_SHADER_TEMPLATE] = 
	{ "tile","object","character","billBoard","sky","charEnv","charSec",
	  "charShine","charRotEnv","charShineRot","charShineEnv","charBump" };
	int          *t_iShaderId[MAX_SHADER_TEMPLATE] = { &m_iTileShaId,&m_iObjectShaId,
		                         &m_iCharacterShaId,&m_iBillBoardShaId,
								 &m_iSkyShaId,&m_iEnvCharacterShaId,
								 &m_iSecCharacterShaId,
								 &m_iSecCharacterShineId,
								 &m_iSecCharRotEnvId,
								 &m_iSecCharShineRotId,
								 &m_iSecCharShineEnvId,
								 &m_iBumpCharacterShaID };


	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	

	int t_iSize = ::read_fileToBuf( "ini\\shaderInit.ini",t_ptrStart,TMP_BUFSIZE );
	osassertex( t_iSize > 0,va("打开文件<%s>失败..\n","ini\\shaderInit.ini" ) );
	
	t_file.OpenMemRead( t_ptrStart,t_iSize );
	for( int t_i=0;t_i<MAX_SHADER_TEMPLATE;t_i ++ )
	{
		if( !t_file.ReadSection( (char*)t_section[t_i].c_str() ) )
			osassert( false );
		if( !t_file.ReadLine( "fxFile",t_szFileName,128 ) )
			osassert( false );
		if( !t_file.ReadLine( "defaultDir",t_szDDir,128 ) )
			osassert( false );

		if( t_i == 0 )
		{
			//
			// 根据不同的mapName，来使用不同目录下的纹理
			*t_iShaderId[t_i] = g_shaderMgr->register_shaderTemplate( 
				t_szFileName,t_szDDir,g_szMapName );
		}
		else
		{
			*t_iShaderId[t_i] = g_shaderMgr->
				register_shaderTemplate( t_szFileName,t_szDDir );
		}
	}

	// 读入fakeShadow相关的数据
	t_file.ReadSection( "fakeShadow" );
	if( !t_file.ReadLine( "fsTexture",g_szFakeShadowTexture,128 ) )
		osassertex( false,"读入假阴影的配置文件出错..\n" );
	t_file.ReadLine( "fsSize",&g_fFakeShadowSize );

	t_file.CloseFile();

	END_USEGBUF( t_iGBufIdx );

	return;

}



/** \brief
*  为所有的接口实现类分配内存.
*/
void osc_engine::Initialize( void )
{
	guard;

	m_3dDeviceMgr = new osc_d3dManager();
	m_inputMgr = new osc_inputmanager();
	
	m_resMgr = new osc_resourceManager();

	//
	// 初始网络接口相关。
	m_socketMgr = new osc_fdSocket();
	int t_i=D3DX_SDK_VERSION;
	if( !m_socketMgr->WSAInitialize() )
		osassert( false );

	m_sceneMgr = new osc_TGManager();

	// 
	// 读取配置文件,用于全局Shader的处理。
	initialize_shader();


	unguard;
}



/** \brief
*  释放engine中分配的内存.
*/
void osc_engine::release( void )
{

	SAFE_DELETE( m_inputMgr );
	
	SAFE_DELETE( m_resMgr );
	
	// 查看osc_TGManager::ReleaseStatic定义的解释。
	osc_TGManager::ReleaseStatic();
	SAFE_DELETE( m_sceneMgr );
	osDebugOut( "场景管理器释放成功....\n" );

	SAFE_DELETE( m_socketMgr );

	// 最后释放d3dDeviceMgr.
	SAFE_DELETE( m_3dDeviceMgr );
	osDebugOut( "3D设备管理器释放成功....\n" );

	SAFE_DELETE(m_shaderCfgDoc);

	
}

/** \brief
*  得到3d DeviceManager的接口.
*/
I_deviceManager* osc_engine::get_3dDeviceMgr( void )
{
	if( !m_bInit )
	{
		m_bInit = true;
		Initialize();
	}
	return (I_deviceManager*)m_3dDeviceMgr;
}

/** \brief
*  得到Input Manager的接口.
*/
I_inputMgr* osc_engine::get_inputMgr( void )
{
	if( !m_bInit )
	{
		m_bInit = true;
		Initialize();
	}

	return (I_inputMgr*)m_inputMgr;
}




/** \brief
*  得到Resource Manager的接口集.
*/
I_resource* osc_engine::get_resourceMgr( void )
{
	if( !m_bInit )
	{
		m_bInit = true;
		Initialize();
	}

	return (I_resource*)m_resMgr;
}

/** \brief
*  得到场景的接口集。
*/
I_fdScene* osc_engine::get_sceneMgr( void )
{
	static bool  t_bFirst = true;

	if( !m_bInit )
	{
		m_bInit = true;
		Initialize();
	}

	if( t_bFirst )
	{
# ifdef _DEBUG
		osDebugOut( "EngCompile DEBUG Version....\n" );
# else
		osDebugOut( "EngCompile RELEASE Version....\n" );
# endif 

		// 显示引擎编译时间。
		osDebugOut( "EngCompile Version:<%s><%s>\n",ENGINE_DIS_VERSION,ENGINE_DIS_TIME );

		t_bFirst = false;
		if( !m_3dDeviceMgr->get_d3ddevice() )
			osassertex( false,"Please initialize Device Manager!" );

		m_sceneMgr->init_sceneWork();
	}

	return (I_fdScene*)m_sceneMgr;

}

/** \brief
*  得到全局的网络相关接口。
*/
I_socketMgr* osc_engine::get_socketMgr( void )
{
	if( !m_bInit )
	{
		m_bInit = true;
		Initialize();
	}

	return (I_socketMgr*)m_socketMgr;
}





//*******************************************************************************************/
//
//  全局性静态变量,由这个变量来初始化全局性的引擎接口组织.
//
//*******************************************************************************************/
/**
 *  引擎组织变量,由这个全局变量提供我们需要的可以操作的引擎接口.
 */
static osc_engine g_osokEngine;


//*******************************************************************************************/
//
//  实现向上层公开的得到常用接口的函数.上层由这几个接口得到
//  引擎中使用内部类功能.
//
//*******************************************************************************************/

/** \brief 
 *  得到全局性的device Manager指针的函数.
 *
 *  使用这个函数来得到I_deviceManager的接口集,使用这个接口集来操作
 *  3D DEVICE.
 */
OSENGINE_API I_deviceManager*   get_deviceManagerPtr( void )
{
	return g_osokEngine.get_3dDeviceMgr();
}


/** \brief
 *  得到InputManager接口的函数.
 */
OSENGINE_API I_inputMgr*      get_inputManagerPtr( void )
{
	return g_osokEngine.get_inputMgr();
}


/** \brief
 *  得到I_reource接口集的函数.
 */
OSENGINE_API I_resource*        get_resourceMgr( void )
{
	return g_osokEngine.get_resourceMgr();
}



/** \brief
 *  得到场景操作接口集的函数。
 */
OSENGINE_API I_fdScene*         get_sceneMgr( void )
{

	return g_osokEngine.get_sceneMgr();
}



/** \brief
 *  得到全局的网络相关接口。
 */
OSENGINE_API I_socketMgr*       get_socketMgr( void )
{
	return g_osokEngine.get_socketMgr();
}

//! 程序退出时，释放全局的静态引擎变量,把渲染引擎变为动态链接库后，
//! 必须得手动的提前释放动态链接库中的相关资源???
OSENGINE_API void              release_renderRes( void )
{
	g_osokEngine.release_engine();
	
	// 释放声音管理器
	release_soundManager();

}





