//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSky.cpp
 *
 *  Desc:     ���������е������ʾ, ������ʾ��ͬ�����.
 *
 *  His:      River created @ 2004-3-15
 *
 *  "��������������ο��Ķ�����Ī���ڲ����Լ�����Ϊ������Ȩ�������������ˡ�"
 *
 */
//--------------------------------------------------------------------------------------------------------

# include "stdafx.h"
# include "../include/fdSky.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../backpipe/include/osCamera.h"
# include "../../backpipe/include/mainEngine.h"
 
//! ������յĴ�С,������հ뾶�Ĵ�С,HalfSkySize����Զɽ�Ĵ���
# define  SKY_SIZE          512.0f
# define  FAR_TERRSIZE       246.0f


//! Զɽ�����λ�á�
# define  LOWFT_POS          30.0f


//! ����յ���һ���ֿ�ʼ��alpha�Ķ���.
# define  ALPHA_DIV          0.6f

//! �����м䶥����Ҫ���͵ĸ߶�.
# define  MIDDLE_SUB         2.0f

//! ����õ��Ķ�����ɫ.
# define  SKY_VERDIFFUSE      D3DCOLOR_ARGB( 188,255,255,255 )
# define  SKY_EDGEDIFFUSE     D3DCOLOR_ARGB( 0,255,255,255 )

//! ��յ����λ������յ����λ�ó������ֵ�õ�,
# define  LOW_SKYHORDIR       2.0f


//! Զɽ�����Զ����ʾ����ɫ.
OSENGINE_API DWORD   g_dwFarTerrColor = 0xff888888;
//! Զɽ����Ӱ��ľ���,����С��0
OSENGINE_API int   g_dwTerrFogDistance = 10;
//! �������Ӱ��ľ���,����С��0
OSENGINE_API int   g_dwSkyFogDistance = 10;



osc_fdSkyMgr::osc_fdSkyMgr()
{
	m_mdPipe = NULL;
	m_szShaderName[0] = NULL;
}
osc_fdSkyMgr::~osc_fdSkyMgr()
{

}






/** \brief
*  ��ʼ�����ǵ�skyMgr.
*/
void osc_fdSkyMgr::init_skyMgr( os_skyInit& _sinit )
{
	guard;

	osVec3D      t_oriVec( 1,0,0 );
	osMatrix      t_mat;
	osVec3D      t_curVec;
	osColor       t_color;

	m_mdPipe = _sinit.m_mdPipe;
	m_fMovSpeed = _sinit.m_fSkyMoveSpeed;
	m_vec3MovSpd = _sinit.m_vec3CloudDir;
	osVec3Normalize( &m_vec3MovSpd,&m_vec3MovSpd );

	m_fSkyHeight = _sinit.m_fSkyHeight;
	strcpy( m_szShaderName,_sinit.m_szSkyName );
	strcpy( m_szIniName,_sinit.m_szIni );
	osDebugOut( "��ʼ����յĳ�ʼ��shader.....\n" );

	m_iSId = g_shaderMgr->add_shader( m_szShaderName, osc_engine::get_shaderFile( "sky" ).c_str() );
	if( m_iSId < 0 )
		m_iSId = g_shaderMgr->add_shadertomanager( m_szShaderName,m_szIniName,osc_engine::m_iSkyShaId );
	osassert( m_iSId >= 0 );
	osDebugOut( "�����ճ�ʼ��shader�Ĵ���....\n" );


	strcpy(g_CurFarTerrain,_sinit.m_szBkTerrOsa);
	// ��ʼ����Ӧ�ı�����պ�Զ��.
	osc_osaSkyTerr::Instance()->init_skyTerr( 
		g_ptrMeshMgr,_sinit.m_szBkSkyOsa,_sinit.m_szBkTerrOsa);

	return;

	unguard;
}


//! ��������õ�����ͼ��
void osc_fdSkyMgr::reset_skyTexture( const char* _bkSky,const char* _bkSky1,const char* _bkSky2,const char* _bolt /*= NULL*/,float rate /*= 1.0f*/ ,float _angle/*=0.0f*/,osColor _color /*= osColor(1.0f,1.0f,1.0f,1.0f)*/,float _boltheight /*= 20.0f*/)
{
	guard;
	
	// ֻ���µ�����ִ��Ϸ����滻
	if( _bkSky && _bkSky[0] )
	{
		if( !osc_osaSkyTerr::Instance()->create_newSky( _bkSky,_bkSky1,_bkSky2 ,_bolt,rate,_angle,_color,_boltheight) )
			osassert( false );
	}

	unguard;
}

//! ����Զ����osa�ļ�.
void osc_fdSkyMgr::reset_osaFarTerr( const char* _bkTerr )
{
	guard;

	if( !osc_osaSkyTerr::Instance()->create_newTerr( _bkTerr ) )
		osassert( false );

	unguard;
}




//! ������յ��ƶ��ٶȡ�
void osc_fdSkyMgr::reset_skyMoveSpeed( float _spd )
{
	guard;

	if( _spd < 0 )
		m_fMovSpeed = 0.0f;
	else
		m_fMovSpeed = _spd;

	unguard;
}

//! �õ������صĲ�����
const char* osc_fdSkyMgr::get_skyTexture( void )
{
	return m_szShaderName;
}

//! �õ����������صĲ���.
const char* osc_fdSkyMgr::get_bkSkyOsa( void )
{
	return osc_osaSkyTerr::Instance()->get_skyOsaName();
}


//! �õ���յ����ٶȡ�
float osc_fdSkyMgr::get_skyMoveSpeed( void )
{
	return m_fMovSpeed;
}

const char*		osc_fdSkyMgr::get_bkSkyOsa1()
{
	return osc_osaSkyTerr::Instance()->get_skyOsaName1();
}
const char*		osc_fdSkyMgr::get_bkSkyOsa2()
{
	return osc_osaSkyTerr::Instance()->get_skyOsaName2();
}
const char*		osc_fdSkyMgr::get_bolt()
{
	return osc_osaSkyTerr::Instance()->get_Bolt();
}
float			osc_fdSkyMgr::get_boltRate()
{
	return osc_osaSkyTerr::Instance()->get_BoltRate();
}





/** \brief
*  ʹ���м�ܵ�����ս�����Ⱦ.
*/
void osc_fdSkyMgr::render_sky( osc_middlePipe* _pipe,
		float _tm,osVec3D _center,float _yaw,float _fardis,bool _reflect )
{
	guard;

	static  float   t_sfLastX = 0.0f;
	static  float   t_sfLastZ = 0.0f;
	osVec3D         t_vec3SkyCen = _center;

	osassert( _pipe==m_mdPipe );
	//osassert( (_fardis >= 0)&&(_fardis<=300) );

	// ���û�г�ʼ��������
	if( !m_mdPipe )
		return ;

	m_mdPipe->set_renderState( D3DRS_FOGENABLE,true );
	
	

	t_vec3SkyCen.y = 0.0f;

	// ��Ⱦ��һ��ı�����պ�Զɽ����һ�����պ�Զɽʹ��osaģ���ļ���
	osc_osaSkyTerr::Instance()->
		render_skyTerr( t_vec3SkyCen,_fardis,_reflect );

	// ��Щ״̬����Ϊ
	m_mdPipe->set_renderState( D3DRS_CULLMODE,D3DCULL_CCW );

	m_mdPipe->set_renderState( D3DRS_FOGENABLE,FALSE );
	m_mdPipe->set_renderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	m_mdPipe->set_renderState( D3DRS_ALPHABLENDENABLE,TRUE );
	m_mdPipe->set_renderState( D3DRS_SRCBLEND ,D3DBLEND_SRCALPHA );
	m_mdPipe->set_renderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	osMatrix  t_idMat;
	osMatrixIdentity( &t_idMat );
	m_mdPipe->set_worldMatrix( t_idMat );

	m_mdPipe->set_sampleState( 0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR );
	m_mdPipe->set_sampleState( 0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR );
	m_mdPipe->set_sampleState( 0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR );
    
	unguard;
}

