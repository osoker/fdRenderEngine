//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: mainEngine.cpp
 *
 *  Desc:     ��������������,�ڴ˴�ʵ�ֵõ�����ӿڵĺ���.
 *
 *  His:      River created @ 4/16 2003    
 *
 *  �ɹ���һ������Ľ�Ա,����ʹ��������Ϊ���ǲ���ʧ��,������һλ
 *  ������������δ���Ŀɿ�����.
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/mainEngine.h"
# include "../../mfpipe/include/osShader.h"

//@{
/** \brief
 *  �Ƿ�ʹ��ȫ�ֵķ����,�����ʹ��ȫ�ֵķ����Ļ����򳡾�����Ⱦ��Ʒ��ʱ�� 
 *  ���򿪵ƹ⣬��ʱ��������Ҫ���е���Ʒ��ʹ�õ���ɫ������ʾ���ƹ�Ч����
 */
OSENGINE_API BOOL         g_bUseGDirLight = TRUE;
/** \brief
 *  ȫ�ֹ��λ�á�
 *  
 *  ���㳡����ʹ�õ���ɫ����Ʒ��Ҳ��Ҫ�������λ�ã����λ��������Ⱦʱ�ƹ��Ĭ��λ�á�
 */
OSENGINE_API osVec3D      g_vec3LPos = osVec3D( 1.0f,1.0f,1.0f );;
//! ������ȫ�ֵķ�������ɫ��
OSENGINE_API float        g_fDirectLR = 0.8f;
OSENGINE_API float        g_fDirectLG = 0.8f;
OSENGINE_API float        g_fDirectLB = 0.8f;

//@} 
OSENGINE_API float        g_fBoltFanAngle = 0.25f;
OSENGINE_API char		  g_CurFarTerrain[255];
OSENGINE_API BOOL		  g_bHasBolt = false;


//! �Ƿ�ʹ��HDR Light
OSENGINE_API BOOL         g_bUseHDRLight = FALSE; 

OSENGINE_API BOOL		  g_SceneSoundBox = false;

# define    ENGINE_DIS_VERSION       __DATE__
# define    ENGINE_DIS_TIME          __TIME__




os_newAction::os_newAction()
{
	m_strActName[0] = NULL;
	m_strNextActName[0] = NULL;

	// ������ŵĶ��������б�
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

	// �м��Ĭ��Ϊ0.
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


//! ����������ʾ�ִ���Ҫ������.
os_stringDisp::os_stringDisp()
{
	z = 0.0f;
	// Ĭ��Ϊ��ȫû�б�����
	m_dwBackColor = 0;
	//! Ĭ��Ϊ��
	dstring = NULL;
	m_szStatic[0] = NULL;
}


//! Ϊ��ǰ�Ľṹ��һ��string��ֵ.
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



// ��ʼ��blend_mode Ϊ3,����uvΪ��0��1.
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



//! Ĭ�ϲ�������
os_skinMeshInit::os_skinMeshInit()
{
	m_bWriteZbuf = true;
	// ...

	// �����΢�����ݡ�
	m_fAdjRotAgl = 0.0f;
	m_vec3AdjPos = osVec3D( 0.0f,0.0f,0.0f );
	m_vec3Max = osVec3D( 1.0f,2.0f,1.0f );
	m_vec3Min = osVec3D( -1.0f,0.0f,-1.0f );

	//! Ĭ��ʹ������������Ĭ�϶���.
	m_iActIdx = -1;

	m_fAngle = 0.0f;

	m_bShadowSupport = false;

	// Ĭ��ʹ�������Ĺ�
	m_bUseDirLAsAmbient = false;
	//@{
	// Windy mod @ 2005-8-30 19:07:19
	//Desc: ride Ĭ�ϲ�������
	//osDebugOut("windy:��ʼ�����ʼ��@=@");
	m_bIsRide = false;
	m_vec3RideOffset = osVec3D(0.0f,0.0f,0.0f);

	//@}

	// Ĭ�ϴ����Ĳ���������ͼ������
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

	//! river added @ 2009-12-30:������Ⱦ��ά�����ϵ�������Ч
	m_bUseInInterface = false;

	m_bPlayWeaponEffect = false;

}

//! ����
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
//! �ر�����shader.
int osc_engine::m_iTileShaId = -1;

//! ��ͨ��Ʒshader.
int osc_engine::m_iObjectShaId = -1;

//! skinMesh��Ʒ��Shader.
int osc_engine::m_iCharacterShaId = -1;

//! ��������͡�
int osc_engine::m_iBillBoardShaId = -1;

//! ������͡�
int osc_engine::m_iSkyShaId = -1;

//! �����˻�����ͼ��skinMeshShader.
int osc_engine::m_iEnvCharacterShaId = -1;

//! �����˵ڶ������������skinMeshShader.
int osc_engine::m_iSecCharacterShaId = -1;

//! River @ 2010-2-25:�������������Է���
int osc_engine::m_iSecCharacterShineId = -1;

//! River @ 2010-3-11: ��������ĸ������ͼЧ����
int osc_engine::m_iSecCharRotEnvId = -1;
int osc_engine::m_iSecCharShineRotId = -1;
int osc_engine::m_iSecCharShineEnvId = -1;



//! �ڶ�������İ�͹��ͼskinMeshShader
int osc_engine::m_iBumpCharacterShaID = -1;

//@} 

//! ������ʹ�õļ���Ӱ�õ�������ͼ���ӰĬ�ϴ�С
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
		// Ϊ����MFC�е��ڴ�й©��ⲻ����,�����ѽ���ĳ�ָ��
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
//! ��ʼ��ȫ�ֵ�Shader�����Ϣ��
void osc_engine::initialize_shader( void )
{
	if(m_shaderCfgDoc == NULL){
		// Ϊ����MFC�е��ڴ�й©��ⲻ����,�����ѽ���ĳ�ָ��
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
	osassertex( t_iSize > 0,va("���ļ�<%s>ʧ��..\n","ini\\shaderInit.ini" ) );
	
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
			// ���ݲ�ͬ��mapName����ʹ�ò�ͬĿ¼�µ�����
			*t_iShaderId[t_i] = g_shaderMgr->register_shaderTemplate( 
				t_szFileName,t_szDDir,g_szMapName );
		}
		else
		{
			*t_iShaderId[t_i] = g_shaderMgr->
				register_shaderTemplate( t_szFileName,t_szDDir );
		}
	}

	// ����fakeShadow��ص�����
	t_file.ReadSection( "fakeShadow" );
	if( !t_file.ReadLine( "fsTexture",g_szFakeShadowTexture,128 ) )
		osassertex( false,"�������Ӱ�������ļ�����..\n" );
	t_file.ReadLine( "fsSize",&g_fFakeShadowSize );

	t_file.CloseFile();

	END_USEGBUF( t_iGBufIdx );

	return;

}



/** \brief
*  Ϊ���еĽӿ�ʵ��������ڴ�.
*/
void osc_engine::Initialize( void )
{
	guard;

	m_3dDeviceMgr = new osc_d3dManager();
	m_inputMgr = new osc_inputmanager();
	
	m_resMgr = new osc_resourceManager();

	//
	// ��ʼ����ӿ���ء�
	m_socketMgr = new osc_fdSocket();
	int t_i=D3DX_SDK_VERSION;
	if( !m_socketMgr->WSAInitialize() )
		osassert( false );

	m_sceneMgr = new osc_TGManager();

	// 
	// ��ȡ�����ļ�,����ȫ��Shader�Ĵ���
	initialize_shader();


	unguard;
}



/** \brief
*  �ͷ�engine�з�����ڴ�.
*/
void osc_engine::release( void )
{

	SAFE_DELETE( m_inputMgr );
	
	SAFE_DELETE( m_resMgr );
	
	// �鿴osc_TGManager::ReleaseStatic����Ľ��͡�
	osc_TGManager::ReleaseStatic();
	SAFE_DELETE( m_sceneMgr );
	osDebugOut( "�����������ͷųɹ�....\n" );

	SAFE_DELETE( m_socketMgr );

	// ����ͷ�d3dDeviceMgr.
	SAFE_DELETE( m_3dDeviceMgr );
	osDebugOut( "3D�豸�������ͷųɹ�....\n" );

	SAFE_DELETE(m_shaderCfgDoc);

	
}

/** \brief
*  �õ�3d DeviceManager�Ľӿ�.
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
*  �õ�Input Manager�Ľӿ�.
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
*  �õ�Resource Manager�Ľӿڼ�.
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
*  �õ������Ľӿڼ���
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

		// ��ʾ�������ʱ�䡣
		osDebugOut( "EngCompile Version:<%s><%s>\n",ENGINE_DIS_VERSION,ENGINE_DIS_TIME );

		t_bFirst = false;
		if( !m_3dDeviceMgr->get_d3ddevice() )
			osassertex( false,"Please initialize Device Manager!" );

		m_sceneMgr->init_sceneWork();
	}

	return (I_fdScene*)m_sceneMgr;

}

/** \brief
*  �õ�ȫ�ֵ�������ؽӿڡ�
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
//  ȫ���Ծ�̬����,�������������ʼ��ȫ���Ե�����ӿ���֯.
//
//*******************************************************************************************/
/**
 *  ������֯����,�����ȫ�ֱ����ṩ������Ҫ�Ŀ��Բ���������ӿ�.
 */
static osc_engine g_osokEngine;


//*******************************************************************************************/
//
//  ʵ�����ϲ㹫���ĵõ����ýӿڵĺ���.�ϲ����⼸���ӿڵõ�
//  ������ʹ���ڲ��๦��.
//
//*******************************************************************************************/

/** \brief 
 *  �õ�ȫ���Ե�device Managerָ��ĺ���.
 *
 *  ʹ������������õ�I_deviceManager�Ľӿڼ�,ʹ������ӿڼ�������
 *  3D DEVICE.
 */
OSENGINE_API I_deviceManager*   get_deviceManagerPtr( void )
{
	return g_osokEngine.get_3dDeviceMgr();
}


/** \brief
 *  �õ�InputManager�ӿڵĺ���.
 */
OSENGINE_API I_inputMgr*      get_inputManagerPtr( void )
{
	return g_osokEngine.get_inputMgr();
}


/** \brief
 *  �õ�I_reource�ӿڼ��ĺ���.
 */
OSENGINE_API I_resource*        get_resourceMgr( void )
{
	return g_osokEngine.get_resourceMgr();
}



/** \brief
 *  �õ����������ӿڼ��ĺ�����
 */
OSENGINE_API I_fdScene*         get_sceneMgr( void )
{

	return g_osokEngine.get_sceneMgr();
}



/** \brief
 *  �õ�ȫ�ֵ�������ؽӿڡ�
 */
OSENGINE_API I_socketMgr*       get_socketMgr( void )
{
	return g_osokEngine.get_socketMgr();
}

//! �����˳�ʱ���ͷ�ȫ�ֵľ�̬�������,����Ⱦ�����Ϊ��̬���ӿ��
//! ������ֶ�����ǰ�ͷŶ�̬���ӿ��е������Դ???
OSENGINE_API void              release_renderRes( void )
{
	g_osokEngine.release_engine();
	
	// �ͷ�����������
	release_soundManager();

}





