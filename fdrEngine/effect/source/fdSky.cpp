//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSky.cpp
 *
 *  Desc:     处理引擎中的天空显示, 用于显示不同的天空.
 *
 *  His:      River created @ 2004-3-15
 *
 *  "世界上最虚弱最不牢靠的东西，莫过于不以自己力量为基础的权力带来的声誉了。"
 *
 */
//--------------------------------------------------------------------------------------------------------

# include "stdafx.h"
# include "../include/fdSky.h"
# include "../../mfpipe/include/middlePipe.h"
# include "../../backpipe/include/osCamera.h"
# include "../../backpipe/include/mainEngine.h"
 
//! 定义天空的大小,就是天空半径的大小,HalfSkySize用于远山的处理。
# define  SKY_SIZE          512.0f
# define  FAR_TERRSIZE       246.0f


//! 远山的最低位置。
# define  LOWFT_POS          30.0f


//! 从天空的哪一部分开始有alpha的顶点.
# define  ALPHA_DIV          0.6f

//! 定义中间顶点需要降低的高度.
# define  MIDDLE_SUB         2.0f

//! 天空用到的顶点颜色.
# define  SKY_VERDIFFUSE      D3DCOLOR_ARGB( 188,255,255,255 )
# define  SKY_EDGEDIFFUSE     D3DCOLOR_ARGB( 0,255,255,255 )

//! 天空的最低位置由天空的最高位置除于这个值得到,
# define  LOW_SKYHORDIR       2.0f


//! 远山和天空远景显示的颜色.
OSENGINE_API DWORD   g_dwFarTerrColor = 0xff888888;
//! 远山受雾影响的距离,不能小于0
OSENGINE_API int   g_dwTerrFogDistance = 10;
//! 天空受雾影响的距离,不能小于0
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
*  初始化我们的skyMgr.
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
	osDebugOut( "初始化天空的初始化shader.....\n" );

	m_iSId = g_shaderMgr->add_shader( m_szShaderName, osc_engine::get_shaderFile( "sky" ).c_str() );
	if( m_iSId < 0 )
		m_iSId = g_shaderMgr->add_shadertomanager( m_szShaderName,m_szIniName,osc_engine::m_iSkyShaId );
	osassert( m_iSId >= 0 );
	osDebugOut( "完成天空初始化shader的处理....\n" );


	strcpy(g_CurFarTerrain,_sinit.m_szBkTerrOsa);
	// 初始化相应的背景天空和远景.
	osc_osaSkyTerr::Instance()->init_skyTerr( 
		g_ptrMeshMgr,_sinit.m_szBkSkyOsa,_sinit.m_szBkTerrOsa);

	return;

	unguard;
}


//! 重设天空用到的贴图。
void osc_fdSkyMgr::reset_skyTexture( const char* _bkSky,const char* _bkSky1,const char* _bkSky2,const char* _bolt /*= NULL*/,float rate /*= 1.0f*/ ,float _angle/*=0.0f*/,osColor _color /*= osColor(1.0f,1.0f,1.0f,1.0f)*/,float _boltheight /*= 20.0f*/)
{
	guard;
	
	// 只有新的天空字串合法才替换
	if( _bkSky && _bkSky[0] )
	{
		if( !osc_osaSkyTerr::Instance()->create_newSky( _bkSky,_bkSky1,_bkSky2 ,_bolt,rate,_angle,_color,_boltheight) )
			osassert( false );
	}

	unguard;
}

//! 重设远景的osa文件.
void osc_fdSkyMgr::reset_osaFarTerr( const char* _bkTerr )
{
	guard;

	if( !osc_osaSkyTerr::Instance()->create_newTerr( _bkTerr ) )
		osassert( false );

	unguard;
}




//! 重设天空的移动速度。
void osc_fdSkyMgr::reset_skyMoveSpeed( float _spd )
{
	guard;

	if( _spd < 0 )
		m_fMovSpeed = 0.0f;
	else
		m_fMovSpeed = _spd;

	unguard;
}

//! 得到天空相关的参数。
const char* osc_fdSkyMgr::get_skyTexture( void )
{
	return m_szShaderName;
}

//! 得到背景天空相关的参数.
const char* osc_fdSkyMgr::get_bkSkyOsa( void )
{
	return osc_osaSkyTerr::Instance()->get_skyOsaName();
}


//! 得到天空的移速度。
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
*  使用中间管道对天空进行渲染.
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

	// 如果没有初始化，返回
	if( !m_mdPipe )
		return ;

	m_mdPipe->set_renderState( D3DRS_FOGENABLE,true );
	
	

	t_vec3SkyCen.y = 0.0f;

	// 渲染第一层的背景天空和远山。这一层的天空和远山使用osa模型文件。
	osc_osaSkyTerr::Instance()->
		render_skyTerr( t_vec3SkyCen,_fardis,_reflect );

	// 这些状态设置为
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

