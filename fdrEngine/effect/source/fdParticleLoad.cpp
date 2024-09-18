//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdParticle.cpp
 *
 *  Desc:     调入场景中粒子特效相关数据的实现类。
 *
 *  His:      River created @ 2004-5-20.
 *
 *  "命运是我们行动的半个主宰，另一半归我们自己来支配。
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////

# include "stdafx.h"
# include "../include/fdParticle.h"
# include "../../backpipe/include/mainEngine.h"

# define   PARTICLE_MAGIC     "ospales"

//! River mod @ 2008-6-13:加入粒子的旋转，环形带产生粒子，吸引等效果。
//# define   PARTICLE_VERSION   0x00010002
# define PARTICLE_VERSION 0x00010003


os_particleRunCtl::os_particleRunCtl()
{
	m_szTextureName[0] = NULL;
}

os_particleRunCtl::~os_particleRunCtl()
{

}


//! 从内存中调入当前的particle数据.
bool os_particleRunCtl::load_particleRunCtl( BYTE*& _sdata,int _ver )
{
	guard;

	READ_MEM_OFF( &m_fGravity,_sdata,sizeof( float ) );
	READ_MEM_OFF( &m_fSpeedNoResis,_sdata,sizeof( float ) );
	READ_MEM_OFF( &m_iAreaSizeResisType,_sdata,sizeof( int ) );
	READ_MEM_OFF( &m_fAreaSizeCons,_sdata,sizeof( float ) );
	READ_MEM_OFF( m_szTextureName,_sdata,sizeof( char )*128 );
	
	// River mod @ 2008-6-13:加入旋转.
	if( _ver < 0x00010003 )
	{
		int t_m;
		READ_MEM_OFF( &t_m,_sdata,sizeof( int ) );
		m_fParticleYRot = 0.0f;
	}
	else
		READ_MEM_OFF( &m_fParticleYRot,_sdata,sizeof( float ) );

	READ_MEM_OFF( &m_iFrameNum,_sdata,sizeof( int ) );
	READ_MEM_OFF( &m_iFrameSets,_sdata,sizeof( int ) );
	READ_MEM_OFF( &m_iBlendMode,_sdata,sizeof( int ) );
	READ_MEM_OFF( &m_fSizeMulti,_sdata,sizeof( float ) );

	//
	// 处理每一个graph数据。
	READ_MEM_OFF( m_vecParSize,_sdata,sizeof( float )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vecRotation,_sdata,sizeof( float )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vecAlpha,_sdata,sizeof( float )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vecColor,_sdata,sizeof( osVec3D )*MAX_DIVISIONNUM );

	m_bInuse = true;

	return true;

	unguard;
}


//! 创建当前particle的设备相关数据，纹理id.
bool os_particleRunCtl::create_shader( void )
{
	guard;

	// 使用中间管理创建纹理的shaderId
	m_iShaderId = g_shaderMgr->add_shader( m_szTextureName, osc_engine::get_shaderFile( "billBoard" ).c_str() );
	if( m_iShaderId < 0 )
		m_iShaderId = g_shaderMgr->add_shadertomanager( m_szTextureName,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
	osassert( m_iShaderId >= 0 );

	return true;

	unguard;
}






//===================================================================================================
//
// Particle Emitter用到的函数。
os_particleEmitter::os_particleEmitter()
{
	m_iTrackId  = -1;
}

os_particleEmitter::~os_particleEmitter()
{

}

//! 从内存中调入particleEmitter的数据。
void os_particleEmitter::load_emitterFromMem( BYTE*& _ps )
{
	guard;

	READ_MEM_OFF( &m_vec3RelativePosition,_ps,sizeof( osVec3D ) );

	READ_MEM_OFF( &m_vec3EmissionSpeedScope,_ps,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_iEmitterShape,_ps,sizeof( int ) );
	READ_MEM_OFF( &m_sbox.m_vecMax,_ps,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_sbox.m_vecMin,_ps,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_fHotPt,_ps,sizeof( float ) );
	READ_MEM_OFF( &m_fFallOff,_ps,sizeof( float ) );
	READ_MEM_OFF( m_szCSFname,_ps,sizeof( char )*64 );
	READ_MEM_OFF( &m_fCycleLength,_ps,sizeof( float ) );
	READ_MEM_OFF( &m_bLoopCycle,_ps,sizeof( BOOL ) );
	READ_MEM_OFF( &m_fGenSpeedMulti,_ps,sizeof( float ) );

	//
	// 读入各个graph相关的数据。
	READ_MEM_OFF( m_vec3Dir,_ps,sizeof( osVec3D )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vec2Rand,_ps,sizeof( osVec2D )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vec2EmissionRate,_ps,sizeof( osVec2D )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vec2EleLife,_ps,sizeof( osVec2D )*MAX_DIVISIONNUM );


	unguard;
}



//===================================================================================================
//
// emisson Control用到的函数。
os_particleEmissionCtl::os_particleEmissionCtl()
{
	m_iParticleRunIdx = -1;
	m_iParEmitterIdx = -1;
	m_iReplaceStyle = -1;
	this->m_fCycleLength = 0.0f;

}

os_particleEmissionCtl::~os_particleEmissionCtl()
{

}

//! 从内存调入emissionControl的数据。
void os_particleEmissionCtl::load_emiCtlFromMem( BYTE*& _ps )
{
	guard;

	READ_MEM_OFF( &m_iParticleRunIdx,_ps,sizeof( int ) );
	READ_MEM_OFF( &m_iParEmitterIdx,_ps,sizeof( int ) );

	// 
	// 得到当前emissionCtl的生命周期。


	unguard;
}


//===================================================================================================
//
// Particle Scene 用到的场景。
os_particleScene::os_particleScene()
{
	m_bInuse = false;
	m_bCanSwap = true;
	m_iEmissionCtlNum = 0;

	m_dwHash = 0;

}

os_particleScene::~os_particleScene()
{

}


//! 从文件中读入我们的Particle数据。
bool os_particleScene::load_parSceneFromFile( char * _fname )
{
	guard;

	BYTE*     t_start;
	int       t_size,t_idx,t_version;
	char      t_str[32];

	osassert( _fname&&(_fname[0]) );

	m_szParSceneName = _fname;
	this->m_dwHash = string_hash( _fname );

	// 读入文件到内存。
	int   t_iGBufIdx = -1;
	t_start = START_USEGBUF( t_iGBufIdx );
	t_size = (DWORD)::read_fileToBuf( _fname,t_start,TMP_BUFSIZE );
	osassert( t_size>0 );

	// 
	//　读入场景文件头.
	READ_MEM_OFF( t_str,t_start,sizeof( char )*8 );
	if( strcmp( t_str,PARTICLE_MAGIC ) )
		osassert( false );
	READ_MEM_OFF( &t_version,t_start,sizeof( DWORD ) );
	osassert( t_version <= PARTICLE_VERSION );


	//
	// 读入场景相关的数据。
	READ_MEM_OFF( &this->m_iMaxParticleNum,t_start,sizeof( int ) );
	READ_MEM_OFF( &this->m_iEmitterTo,t_start,sizeof( int ) );
	READ_MEM_OFF( &this->m_iNpwmr,t_start,sizeof( int ) );

	//
	// 读入multisize和各种控制的数目。
	READ_MEM_OFF( &this->m_fMultiSize,t_start,sizeof( float ) );
	READ_MEM_OFF( &this->m_iParRunCtlNum,t_start,sizeof( int ) );
	READ_MEM_OFF( &this->m_iEmitterNum,t_start,sizeof( int ) );
	READ_MEM_OFF( &this->m_iEmissionCtlNum,t_start,sizeof( int ) );

	m_vecParRunCtl.resize( m_iParRunCtlNum );
	m_vecParEmitter.resize( m_iEmitterNum );
	m_vecEmssionCtl.resize( m_iEmissionCtlNum );

	for( t_idx=0;t_idx<m_iParRunCtlNum;t_idx ++ )
		m_vecParRunCtl[t_idx].load_particleRunCtl( t_start,t_version );
	for( t_idx=0;t_idx<m_iEmitterNum;t_idx++ )
		m_vecParEmitter[t_idx].load_emitterFromMem( t_start );
	for( t_idx=0;t_idx<m_iEmissionCtlNum;t_idx++ )
		m_vecEmssionCtl[t_idx].load_emiCtlFromMem( t_start );

	//
	// 读入track信息。
	READ_MEM_OFF( &this->m_bUseTrack,t_start,sizeof( bool ) );
	if( m_bUseTrack )
	{
		READ_MEM_OFF( m_szTrackFileName,t_start,sizeof( char )*128 );
		READ_MEM_OFF( &m_fRunSpeed,t_start,sizeof( float ) );
	}
	
	END_USEGBUF( t_iGBufIdx );

	// 
	// River @ 2006-5-18:
	// 如果当前的particle场景内的emitter用到的track,则创建
	for( t_idx=0;t_idx<m_iEmitterNum;t_idx++ )
	{
		if( m_vecParEmitter[t_idx].m_iEmitterShape != 2 )
			continue;

		m_vecParEmitter[t_idx].m_iTrackId = g_ptrTrackMgr->
			create_track( m_vecParEmitter[t_idx].m_szCSFname,1.0f,TRUE );

		// 
		//  ATTENTION TO FIX: River mod @ 2007-4-6:修改为不出错.
		if( m_vecParEmitter[t_idx].m_iTrackId < 0 )
		{
			m_vecParEmitter[t_idx].m_iEmitterShape = 1;
			m_vecParEmitter[t_idx].m_fHotPt = 0.3f;
			m_vecParEmitter[t_idx].m_fFallOff = 0.5f;
		}
	}
	

	// 
	for( int t_i=0;t_i<m_iParRunCtlNum;t_i ++ )
		m_vecParRunCtl[t_i].create_shader();


	//
	// 设置当前的粒子场景为使用中。
	m_bInuse = true;

	return true;

	unguard;
}


//! 从内存中释放一个粒子.
void os_particleScene::release_parScene( void )
{
	guard;

	int t_idx;
	for( t_idx=0;t_idx<m_iEmitterNum;t_idx++ )
	{
		if( m_vecParEmitter[t_idx].m_iEmitterShape != 2 )
			continue;

		if( m_vecParEmitter[t_idx].m_iTrackId >= 0 )
		{
			g_ptrTrackMgr->delete_track( m_vecParEmitter[t_idx].m_iTrackId );
			m_vecParEmitter[t_idx].m_iTrackId = -1;
		}
	}
	
	for( int t_i=0;t_i<m_iParRunCtlNum;t_i ++ )
	{
		g_shaderMgr->release_shader( m_vecParRunCtl[t_i].m_iShaderId );
		m_vecParRunCtl[t_i].m_iShaderId = -1;
	}

	m_bInuse = false;

	unguard;
}

	






//===================================================================================================
//  以下为PatcileMgr用到的函数实现。
// 

