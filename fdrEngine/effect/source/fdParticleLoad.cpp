//////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdParticle.cpp
 *
 *  Desc:     ���볡����������Ч������ݵ�ʵ���ࡣ
 *
 *  His:      River created @ 2004-5-20.
 *
 *  "�����������ж��İ�����ף���һ��������Լ���֧�䡣
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////

# include "stdafx.h"
# include "../include/fdParticle.h"
# include "../../backpipe/include/mainEngine.h"

# define   PARTICLE_MAGIC     "ospales"

//! River mod @ 2008-6-13:�������ӵ���ת�����δ��������ӣ�������Ч����
//# define   PARTICLE_VERSION   0x00010002
# define PARTICLE_VERSION 0x00010003


os_particleRunCtl::os_particleRunCtl()
{
	m_szTextureName[0] = NULL;
}

os_particleRunCtl::~os_particleRunCtl()
{

}


//! ���ڴ��е��뵱ǰ��particle����.
bool os_particleRunCtl::load_particleRunCtl( BYTE*& _sdata,int _ver )
{
	guard;

	READ_MEM_OFF( &m_fGravity,_sdata,sizeof( float ) );
	READ_MEM_OFF( &m_fSpeedNoResis,_sdata,sizeof( float ) );
	READ_MEM_OFF( &m_iAreaSizeResisType,_sdata,sizeof( int ) );
	READ_MEM_OFF( &m_fAreaSizeCons,_sdata,sizeof( float ) );
	READ_MEM_OFF( m_szTextureName,_sdata,sizeof( char )*128 );
	
	// River mod @ 2008-6-13:������ת.
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
	// ����ÿһ��graph���ݡ�
	READ_MEM_OFF( m_vecParSize,_sdata,sizeof( float )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vecRotation,_sdata,sizeof( float )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vecAlpha,_sdata,sizeof( float )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vecColor,_sdata,sizeof( osVec3D )*MAX_DIVISIONNUM );

	m_bInuse = true;

	return true;

	unguard;
}


//! ������ǰparticle���豸������ݣ�����id.
bool os_particleRunCtl::create_shader( void )
{
	guard;

	// ʹ���м�����������shaderId
	m_iShaderId = g_shaderMgr->add_shader( m_szTextureName, osc_engine::get_shaderFile( "billBoard" ).c_str() );
	if( m_iShaderId < 0 )
		m_iShaderId = g_shaderMgr->add_shadertomanager( m_szTextureName,BILLBOARD_SHADERINI,osc_engine::m_iBillBoardShaId );
	osassert( m_iShaderId >= 0 );

	return true;

	unguard;
}






//===================================================================================================
//
// Particle Emitter�õ��ĺ�����
os_particleEmitter::os_particleEmitter()
{
	m_iTrackId  = -1;
}

os_particleEmitter::~os_particleEmitter()
{

}

//! ���ڴ��е���particleEmitter�����ݡ�
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
	// �������graph��ص����ݡ�
	READ_MEM_OFF( m_vec3Dir,_ps,sizeof( osVec3D )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vec2Rand,_ps,sizeof( osVec2D )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vec2EmissionRate,_ps,sizeof( osVec2D )*MAX_DIVISIONNUM );
	READ_MEM_OFF( m_vec2EleLife,_ps,sizeof( osVec2D )*MAX_DIVISIONNUM );


	unguard;
}



//===================================================================================================
//
// emisson Control�õ��ĺ�����
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

//! ���ڴ����emissionControl�����ݡ�
void os_particleEmissionCtl::load_emiCtlFromMem( BYTE*& _ps )
{
	guard;

	READ_MEM_OFF( &m_iParticleRunIdx,_ps,sizeof( int ) );
	READ_MEM_OFF( &m_iParEmitterIdx,_ps,sizeof( int ) );

	// 
	// �õ���ǰemissionCtl���������ڡ�


	unguard;
}


//===================================================================================================
//
// Particle Scene �õ��ĳ�����
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


//! ���ļ��ж������ǵ�Particle���ݡ�
bool os_particleScene::load_parSceneFromFile( char * _fname )
{
	guard;

	BYTE*     t_start;
	int       t_size,t_idx,t_version;
	char      t_str[32];

	osassert( _fname&&(_fname[0]) );

	m_szParSceneName = _fname;
	this->m_dwHash = string_hash( _fname );

	// �����ļ����ڴ档
	int   t_iGBufIdx = -1;
	t_start = START_USEGBUF( t_iGBufIdx );
	t_size = (DWORD)::read_fileToBuf( _fname,t_start,TMP_BUFSIZE );
	osassert( t_size>0 );

	// 
	//�����볡���ļ�ͷ.
	READ_MEM_OFF( t_str,t_start,sizeof( char )*8 );
	if( strcmp( t_str,PARTICLE_MAGIC ) )
		osassert( false );
	READ_MEM_OFF( &t_version,t_start,sizeof( DWORD ) );
	osassert( t_version <= PARTICLE_VERSION );


	//
	// ���볡����ص����ݡ�
	READ_MEM_OFF( &this->m_iMaxParticleNum,t_start,sizeof( int ) );
	READ_MEM_OFF( &this->m_iEmitterTo,t_start,sizeof( int ) );
	READ_MEM_OFF( &this->m_iNpwmr,t_start,sizeof( int ) );

	//
	// ����multisize�͸��ֿ��Ƶ���Ŀ��
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
	// ����track��Ϣ��
	READ_MEM_OFF( &this->m_bUseTrack,t_start,sizeof( bool ) );
	if( m_bUseTrack )
	{
		READ_MEM_OFF( m_szTrackFileName,t_start,sizeof( char )*128 );
		READ_MEM_OFF( &m_fRunSpeed,t_start,sizeof( float ) );
	}
	
	END_USEGBUF( t_iGBufIdx );

	// 
	// River @ 2006-5-18:
	// �����ǰ��particle�����ڵ�emitter�õ���track,�򴴽�
	for( t_idx=0;t_idx<m_iEmitterNum;t_idx++ )
	{
		if( m_vecParEmitter[t_idx].m_iEmitterShape != 2 )
			continue;

		m_vecParEmitter[t_idx].m_iTrackId = g_ptrTrackMgr->
			create_track( m_vecParEmitter[t_idx].m_szCSFname,1.0f,TRUE );

		// 
		//  ATTENTION TO FIX: River mod @ 2007-4-6:�޸�Ϊ������.
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
	// ���õ�ǰ�����ӳ���Ϊʹ���С�
	m_bInuse = true;

	return true;

	unguard;
}


//! ���ڴ����ͷ�һ������.
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
//  ����ΪPatcileMgr�õ��ĺ���ʵ�֡�
// 

