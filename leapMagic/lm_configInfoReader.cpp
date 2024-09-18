///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_configInfoReader.cpp
 *
 *  Desc:     ����ħ��demo������������ļ���ıȡ
 * 
 *  His:      River created @ 2006-4-13 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

# include "lm_configinforeader.h"
# include "gc_command.h"
# include "gc_camera.h"

# define INIT_ININAME   "magicInit.ini"

lm_configInfoReader*          g_ptrConfigInfoReader = NULL;
static lm_configInfoReader    t_sConfigInfoReader;

lm_configInfoReader::lm_configInfoReader(void)
{
	g_ptrConfigInfoReader = this;
}

lm_configInfoReader::~lm_configInfoReader(void)
{

}

//! �����ʼ�����������
bool lm_configInfoReader::read_engineInitVar( void )
{
	guard;

	CIniFile   t_file;
	int        t_iTmp;


# if 1
	int t_iGBufIdx;
	BYTE*  t_ptrStart = START_USEGBUF( t_iGBufIdx );
	int t_i = read_fileToBuf( INIT_ININAME,t_ptrStart,TMP_BUFSIZE );
	t_file.OpenMemRead( t_ptrStart,t_i );
# else
	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<magicInit.ini>�ļ�ʧ��...\n" ); 
# endif 

	//
	// ����ȫ�ֵĵ�������
	if( !t_file.ReadSection( "debugInfo" ) )
		osassert( false );
	t_file.ReadLine( "UsePackSystem",&t_iTmp );
	g_bUsePackSystem = t_iTmp;
	t_file.ReadLine( "PackIniName",m_szPackFileIni,256 );

	t_file.ReadLine( "logToLogFile",&t_iTmp );
	g_bLogToFile = t_iTmp;

	// �Ƿ���ȫ�ֵ���������
	t_file.ReadLine( "testSound",&t_iTmp );
	g_bUseSound = t_iTmp;

	// 
	// ��ʼ���������������
	float    t_fR,t_fG,t_fB;
	if( !t_file.ReadSection( "engine" ) )
		osassert( false );
	t_file.ReadLine( "fogenable",&t_iTmp );
	g_bUseFog = t_iTmp;
	t_file.ReadLine( "fogcolorR",&t_fR );
	t_file.ReadLine( "fogcolorG",&t_fG );
	t_file.ReadLine( "fogcolorB",&t_fB );
	g_dwFogColor = D3DXCOLOR( t_fR,t_fG,t_fB,1.0f );
	t_file.ReadLine( "fogstart",&g_fFogStart );
	t_file.ReadLine( "fogend",&g_fFogEnd );
	t_file.ReadLine( "texdiv",(int*)&g_dwTexDiv );

	//
	// ��ʼ����ȫ�ֹ������
	if( !t_file.ReadSection( "dlight" ) )
		osassert( false );
	t_file.ReadLine( "ambientR",&g_fAmbiR );
	t_file.ReadLine( "ambientG",&g_fAmbiG );
	t_file.ReadLine( "ambientB",&g_fAmbiB );

	t_file.ReadLine( "lightCR",&g_fDlR );
	t_file.ReadLine( "lightCG",&g_fDlG );
	t_file.ReadLine( "lightCB",&g_fDlB );

	t_file.ReadLine( "hdrEnable",&t_iTmp );
	g_bUseHDRLight = t_iTmp;


	// �ر�ini�ļ�
	t_file.CloseFile();

# if 1
	END_USEGBUF( t_iGBufIdx );
# endif 

	return true;

	unguard;
}

//! 3d�豸��ʼ����������� 
bool lm_configInfoReader::read_d3dDevInitVar( os_deviceinit& _di )
{
	guard;

	CIniFile   t_file;
	int        t_iTmp;

	char   t_sz[256];
	::getcwd( t_sz,256 );

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<init.ini>�ļ�ʧ��...\n" ); 

	if( !t_file.ReadSection( "screen" ) )
		osassert( false );

	t_file.ReadLine( "fullscr",&t_iTmp );
	_di.m_bFullScr = t_iTmp;
	t_file.ReadLine( "width",(int*)&_di.m_dwScrWidth );
	t_file.ReadLine( "height",(int*)&_di.m_dwScrHeight );
	t_file.ReadLine( "bpp",(int*)&_di.m_dwColorBits );
	t_file.ReadLine( "shaderdebug",(int*)&g_bShaderDebug );

	_di.m_bHwCursor = true;
	_di.m_dwDepthBits = 24;
	_di.m_dwStencilBits = 8;

	_di.m_szDisTexName[0] = NULL;
	_di.m_fStartX = 0;
	_di.m_fStartY = 0;
	_di.m_displayFrequency = 0;

	// �ر�ini�ļ�
	t_file.CloseFile();

	return true;

	unguard;
}

//! �����ʼ�����������
bool lm_configInfoReader::read_camInitData( os_cameraInit& _camInit )
{
	guard;

	CIniFile   t_file;

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<init.ini>�ļ�ʧ��...\n" ); 

	if( !t_file.ReadSection( "caminit" ) )
		osassert( false );

	t_file.ReadLine( "fov",&_camInit.fov );
	t_file.ReadLine( "farclip",&_camInit.farp );
	_camInit.nearp = 0.1f;

	t_file.CloseFile();

	return true;

	unguard;
}
//! ������Ϸ���������������
bool lm_configInfoReader::read_cameraData( gc_camera* _cam )
{
	guard;

	osassert( _cam );
	CIniFile   t_file;

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<init.ini>�ļ�ʧ��...\n" ); 

	if( !t_file.ReadSection( "caminit" ) )
		osassert( false );

	if( !t_file.ReadLine( "camFocus",&gc_camera::m_fFocusOffset ) )
		osassert( false );
	if( !t_file.ReadLine( "toFocusSpeed",&_cam->m_fToFocusSpeed ) )
		osassert( false );
	if( !t_file.ReadLine( "camMinDis",&_cam->m_fMinFocusDis ) )
		osassert( false );
	if( !t_file.ReadLine( "camMaxDis",&_cam->m_fMaxFocusDis ) )
		osassert( false );

	if( !t_file.ReadLine( "camRotSpeed",&gc_camera::m_fRotSpeed ) )
		osassert( false );
	if( !t_file.ReadLine( "camLimit",&gc_camera::m_fCamPitchLimit ) )
		osassert( false );

	//! �Ѷ�ת��Ϊ����
	gc_camera::m_fCamPitchLimit =
		(gc_camera::m_fCamPitchLimit/180.0f)*OS_PI;

	t_file.CloseFile();

	return true;

	unguard;
}


//! �����ͼ��ʼ����Ҫ�����ݡ�
bool lm_configInfoReader::read_mapInitData( os_sceneLoadStruct& _sl )
{
	guard;

	CIniFile   t_file;

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<init.ini>�ļ�ʧ��...\n" ); 

	if( !t_file.ReadSection( "testmapChar" ) )
		osassert( false );

	t_file.ReadLine( "map",_sl.m_szMapName,8 );
	t_file.ReadLine( "initx",&_sl.m_iX );
	t_file.ReadLine( "initz",&_sl.m_iY );

	t_file.CloseFile();

	return true;

	unguard;
}

//! ���������ʼ����Ҫ������
bool lm_configInfoReader::read_heroInsertData( lm_heroInsert* _hi )
{
	guard;

	osassert( _hi );
	CIniFile   t_file;

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<init.ini>�ļ�ʧ��...\n" ); 

	if( !t_file.ReadSection( "testmapChar" ) )
		osassert( false );

	t_file.ReadLine( "initx",&_hi->m_iXPos );
	t_file.ReadLine( "initz",&_hi->m_iZPos );
	t_file.ReadLine( "heroDir",_hi->m_szChrDir,64 );

	t_file.CloseFile();

	return true;
	unguard;
}

//! �õ����������������ٶ�
float lm_configInfoReader::read_heroWalkSpeed( void )
{
	guard;

	CIniFile   t_file;
	float      t_fSpeed = 0;

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<init.ini>�ļ�ʧ��...\n" ); 

	if( !t_file.ReadSection( "testmapChar" ) )
		osassert( false );

	t_file.ReadLine( "heroMoveSpeed",&t_fSpeed );

	t_file.CloseFile();

	return t_fSpeed;

	unguard;
}

//! �õ�ȫ�ֵĵ���ر�Ч��������
void lm_configInfoReader::read_cursorDrawInScene( os_sceneCursorInit& _cursor )
{
	guard;

	CIniFile    t_file;
	BOOL        t_bPlayOsa;
	int         t_iTmp;

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassert( false );
	
	if( !t_file.ReadSection( "cursor" ) )
		osassert( false );

	t_file.ReadLine( "osaCursor",&t_bPlayOsa );
	if( t_bPlayOsa )
	{
		t_file.ReadLine( "osaName",_cursor.m_szOsaCursorName,64 );
		_cursor.m_iPlayOsaNum = 1;
	}
	else
	{
		t_file.ReadLine( "decalCursor",_cursor.m_szCursorName,64 );
		t_file.ReadLine( "cursortime",&_cursor.m_fFrameTime );
		t_file.ReadLine( "cursorX",&t_iTmp );
		_cursor.m_wXFrameNum = t_iTmp;
		t_file.ReadLine( "cursorY",&t_iTmp );
		_cursor.m_wYFrameNum = t_iTmp;
	}

	// �����Ƿ������ײ��Ϣ�����ı���
	if( !t_file.ReadSection( "adjcoll" ) )
		osassert( false );

	if( !t_file.ReadLine( "adjcoll",&lm_gvar::g_bAdjMapCollInfo ) )
		osassert( false );

	t_file.CloseFile();

	return;

	unguard;
}


//! �õ���������footmark��ص�����
void lm_configInfoReader::read_heroFootMarkInfo( lm_footMarkInfo* _info )
{
	guard;

	CIniFile   t_file;
	float      t_fSpeed = 0;

	osassert( _info );

	if( !t_file.OpenFileRead( INIT_ININAME ) )
		osassertex( false,"��<init.ini>�ļ�ʧ��...\n" ); 

	if( !t_file.ReadSection( "footmark" ) )
		osassert( false );

	t_file.ReadLine( "footmarkTex",_info->m_szFootMarkTex,32 );
	t_file.ReadLine( "footmarkRotAdj",&_info->m_fFootMarkRotAdj );
	t_file.ReadLine( "footmarkVanish",&_info->m_fMarkVanishTime );
	t_file.ReadLine( "footmarkSize",&_info->m_fMarkSize );
	t_file.ReadLine( "footmark1Percent",&_info->m_fMark1Percent );
	t_file.ReadLine( "footmark2Percent",&_info->m_fMark2Percent );
	t_file.ReadLine( "footmarkOffset",&_info->m_fMarkOffset );

	t_file.CloseFile();

	return;

	unguard;
}



