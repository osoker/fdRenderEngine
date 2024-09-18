/////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdSpecialEffect.cpp
 *
 *  Desc:     场景内的特效文件，包括了所有特效的集合，可以更方便的在场景编辑器
 *            内放置各种动画的组合，这些组合在单独的物品在场景中编辑在比以多个
 *            特效在场景内编辑要容易的多。
 *
 *  His:      River created @ 2005-12-23
 *
 *  
 *  
 */
///////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdSpecialEffect.h"
# include "../../Terrain/include/fdFieldMgr.h"

# define SOUNDGROUP_FILEEXT   "snd"

# define DEFAULT_SNDPLAYTIME   1.0f

//! 所有的特效声音使用相同的声音参数？
# define EFFECT_MAXSNDDIS     50.0f
# define EFFECT_MINSNDDIS     8.0f


//! 播放特效的声音相关的接口
I_soundManager* osc_specEffectIns::m_ptrSoundMgr = NULL;

//! 核心特效数据的管理器
CSpaceMgr<osc_specEffect> osc_specEffMgr::m_arrSEffect;


//! River @ 2009-10-12:用于创建一个特效元素的时候，设置此元半透明
void os_effObjCluser::alpha_initProcess( int _id,ose_effctType _etype )
{
	guard;

	osassert( _id < (int)m_fCreatedWithAlpha.size() );

	if( _etype != OSE_SEFFECT_OSA )
		return;

	if( m_fCreatedWithAlpha[_id] >= 0.999f )
		return;

	osc_specEffMgr::m_ptrScene->start_fadeOsa(
		m_dwEffObjId[_id],1.0f,true,m_fCreatedWithAlpha[_id] );
	m_fCreatedWithAlpha[_id] = 1.0f;

	unguard;
}


osc_effectObj::osc_effectObj()
{
	m_fSizeScale = 1.0f;
}


/** \brief
* 从内存中调入当前的effect元素数据。
*	
*\param BYTE*	_fstart 
*\param int		_iVersion
*\return void*/
void osc_effectObj::load_effectFromMem( BYTE*& _fstart,DWORD _fver )
{
	guard;

	osassert( _fstart );

	READ_MEM_OFF( &m_eEffType,_fstart,sizeof( DWORD ) );
	READ_MEM_OFF( m_szEffectName,_fstart,sizeof( char )*MAX_EFFNAMELENGTH );
	READ_MEM_OFF( &m_vec3OffsetPos,_fstart,sizeof( osVec3D ) );
	READ_MEM_OFF( &m_fStartPlayTime,_fstart,sizeof( float ) );
	READ_MEM_OFF( &m_ePlayStyle,_fstart,sizeof( ose_amPlayType ) );

	// 场景内的特效只有循环播放的数据
	switch( m_ePlayStyle )
	{
	case OSE_PLAYFOREVER:
		//!循环播放不保存数据
		break;
	case OSE_PLAYWITHFRAME:
		//!播放帧数
		READ_MEM_OFF( &m_uPlayData.m_iPlayFrames,_fstart,sizeof( int ) );
		break;
	case OSE_PLAYWITHTIME:
		//!播放时间
		READ_MEM_OFF( &m_uPlayData.m_fPlayTime, _fstart, sizeof( float ) );
		break;
	}

	// 
	// 208的版本
	if( _fver >= 208 )
	{
		float   t_fAgl;

		// 旋转需要跟上层传入的旋转配合使用，暂不支持
		READ_MEM_OFF( &t_fAgl,_fstart,sizeof( float ) );

		// River added @ 2--7-6-5:加入旋转支持
		m_fObjRot = t_fAgl;

		// 暂只支持缩放
		READ_MEM_OFF( &m_fSizeScale,_fstart,sizeof( float ) );
	}
	else
		m_fObjRot = 0.0f;

	unguard;
}


//! 验证当前的effectObj是否处于可用的更新状态
BOOL osc_effectObj::validate_effObj( int _id )
{
	guard;

	if( _id == -1 )
		return FALSE;

	switch( m_eEffType )
	{
	case OSE_SEFFECT_OSA:
		// osa有自删除功能，此处可能已经把osa删除
		return osc_specEffMgr::m_ptrScene->validate_aniMeshId( _id );
	case OSE_SEFFECT_BB:
		return osc_specEffMgr::m_ptrScene->validate_bbId( _id );
	case OSE_SEFFECT_FDP:
		// 粒子也有自删除的功能...
		return osc_specEffMgr::m_ptrScene->validate_parId( _id );
	}

	return FALSE;

	unguard;
}


/** \brief
* 停止播放当前的动画。
*
* \param Int _id 要删除的设备相关数据的id.
*/
void osc_effectObj::delete_devdepEffect( int _id )
{
	guard;

	osassert( _id != -1 );
	osassert( _id >= 0 );

	// 根据不同的特效元素类型调入设备相关的特效数据。
	switch( m_eEffType )
	{
	case OSE_SEFFECT_OSA:
		{	
			// osa有自删除功能，此处可能已经把osa删除
			if( osc_specEffMgr::m_ptrScene->validate_aniMeshId( _id ) )
				osc_specEffMgr::m_ptrScene->delete_animationMesh( _id );

			break;
		}
	case OSE_SEFFECT_BB:
		{
			if( osc_specEffMgr::m_ptrScene->validate_bbId( _id ) )
				osc_specEffMgr::m_ptrScene->delete_billBoard( _id );
			break;
		}
	case OSE_SEFFECT_FDP:
		{
			// 粒子也有自删除的功能...
			if( osc_specEffMgr::m_ptrScene->validate_parId( _id ) )
				osc_specEffMgr::m_ptrScene->delete_particle( _id );

			break;
		}
	}

	unguard;
}


//! 预创建相关的设备相关数据
void osc_effectObj::pre_createDevdevEff( void )
{
	guard;

	// 根据不同的特效元素类型调入设备相关的特效数据。
	switch( m_eEffType )
	{
	case OSE_SEFFECT_OSA:
		{			
			os_aniMeshCreate   t_c;

			t_c.m_bAutoDelete = true;
			t_c.m_dwAmbientL = 0xffffffff;
			t_c.m_ePlayType = m_ePlayStyle;
			t_c.m_fAngle = 0.0f;
			t_c.m_fPlayTime = m_uPlayData.m_fPlayTime;
			t_c.m_iPlayFrames = m_uPlayData.m_iPlayFrames;
			strcpy( t_c.m_szAMName,m_szEffectName );
			t_c.m_bCreateIns = false;

			// 如果是场景内的特效，则osa受环境光的影响
			t_c.m_bEffectByAmbiL = false;

			osc_specEffMgr::m_ptrScene->create_osaAmimation( &t_c );

            
			break;
		}
	case OSE_SEFFECT_BB:
		{
			osc_specEffMgr::m_ptrScene->create_billBoard( 
				m_szEffectName,osVec3D( 0.0f,0.0f,0.0f ),false );

			break;
		}
	case OSE_SEFFECT_FDP:
		{
			os_particleCreate  t_pc;
			
			t_pc.m_vec3Offset = osVec3D( 0.0f,0.0f,0.0f );
			strcpy( t_pc.m_szParticleName,m_szEffectName );
			t_pc.m_bAutoDelete = true;
			t_pc.m_bCreateIns = false;

			// 场景内的粒子特效，如果使用规道，则初始化为随机开始
			osc_specEffMgr::m_ptrScene->create_particle( t_pc,false );

			break;
		}
		
	}

	return ;

	unguard;
}


/** \brief
*  根据调入的特效元素信息，创建对应的引擎设备相关id.
*/
int osc_effectObj::create_devdepEffect( osVec3D& _woffset,float _agl/* = 0.0f*/,
					     float _fSpeedScale/* = 1.0f*/,
						 bool _sceneEff/* = false*/,float _sizeScale/* = 1.0f*/ )
{
	guard;

	float       fPlayTime = 0.0f;
	int         t_iIdx = -1;
	osVec3D     t_finalPos,t_vec3OffsetScale;
	bool        t_bTopLayer = false;//!_sceneEff;


	t_vec3OffsetScale = m_vec3OffsetPos*_sizeScale;
	if( float_equal( _agl,0.0f ) )
		t_finalPos = _woffset + t_vec3OffsetScale;
	else
	{
		osMatrix   t_sMat;
		osVec3D    t_vec3Transform;

		osMatrixRotationY( &t_sMat,_agl );
		osVec3Transform( &t_vec3Transform,&t_vec3OffsetScale,&t_sMat );
		t_finalPos = _woffset + t_vec3Transform;
	}

	// 根据不同的特效元素类型调入设备相关的特效数据。
	switch( m_eEffType )
	{
	case OSE_SEFFECT_OSA:
		{			
			os_aniMeshCreate   t_c;

			t_c.m_vec3Pos = t_finalPos;
			t_c.m_bAutoDelete = true;
			t_c.m_dwAmbientL = 0xffffffff;
			t_c.m_ePlayType = m_ePlayStyle;
			t_c.m_fAngle = _agl;
			t_c.m_fPlayTime = m_uPlayData.m_fPlayTime;
			t_c.m_iPlayFrames = m_uPlayData.m_iPlayFrames;
			strcpy( t_c.m_szAMName,m_szEffectName );
			t_c.m_bCreateIns = true;

			// 如果是场景内的特效，则osa受环境光的影响
			// River mod @ 2010-4-1:
			t_c.m_bEffectByAmbiL = _sceneEff;

			t_c.m_fSizeScale = m_fSizeScale*_sizeScale;

			t_c.m_fAngle += m_fObjRot;

			t_iIdx = osc_specEffMgr::m_ptrScene->create_osaAmimation( &t_c );
            
			break;
		}
	case OSE_SEFFECT_BB:
		{
			t_iIdx = osc_specEffMgr::m_ptrScene->create_billBoard( 
				m_szEffectName,t_finalPos,true,m_fSizeScale*_sizeScale );
			osassert( t_iIdx != -1 );
			break;
		}
	case OSE_SEFFECT_FDP:
		{
			os_particleCreate  t_pc;
			
			t_pc.m_vec3Offset = t_finalPos;
			strcpy( t_pc.m_szParticleName,m_szEffectName );
			t_pc.m_bAutoDelete = true;
			t_pc.m_bCreateIns = true;
			t_pc.m_fSizeScale = m_fSizeScale*_sizeScale;

			// 场景内的粒子特效，如果使用规道，则初始化为随机开始
			osc_TGManager*  t_ptrTgMgr = (osc_TGManager*)osc_specEffMgr::m_ptrScene;
			t_iIdx = t_ptrTgMgr->create_particle( t_pc,_sceneEff );

			osassert( t_iIdx != -1 );
			break;
		}
		
	}

	return t_iIdx;

	unguard;
}







osc_specEffect::osc_specEffect()
{
	m_vecEffect = NULL;
	m_fPlayTime = DEFAULT_SNDPLAYTIME;
	m_szDecalName[0] = NULL;

	m_fKeyTime = 0.0f;

}

osc_specEffect::~osc_specEffect()
{
	SAFE_DELETE_ARRAY( m_vecEffect );
}

/** \brief
*  释放当前资源的纯虚函数，由release_ref函数调用。
*  
*/
void osc_specEffect::release_obj( DWORD _ptr/* = NULL*/ )
{
	guard;

	SAFE_DELETE_ARRAY( m_vecEffect );

	unguard;
}


/** \brief
*  调入当前的effect文件。
*/
BOOL osc_specEffect::load_effectFromFile( 
	const char* _fname,int _size/* = 0*/,BYTE*  _fileData/* = NULL*/ )
{
	guard;

	int               t_size;
	DWORD             t_dwVersion;
	BYTE*             t_fstart;
	char              t_szMagic[12];

	osassert( 64 > strlen( _fname ) );
	strcpy( m_szEffectName,_fname );
	m_dwHashId = string_hash( _fname );

	osassert( (_fname)&&(_fname[0]!=NULL) );
	
	int   t_iGBufIdx = -1;
	if( _fileData )
	{
		t_fstart = _fileData;
		t_size = _size;
	}
	else
	{
		t_fstart = START_USEGBUF( t_iGBufIdx );
		t_size = read_fileToBuf( (char*)_fname,t_fstart,TMP_BUFSIZE );
		osassert( t_size>=0 );
	}

	// 文件头的处理
	READ_MEM_OFF( t_szMagic,t_fstart,sizeof( char )*4 );
	if( 0 != strcmp( t_szMagic,EFFECTFILE_MAGIC ) )
	{
		osassert( false );
		return FALSE;
	}
	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	if( EFFECTFILE_VERSION > t_dwVersion )
	{
		osassertex( false,
			va( "File<%s>,version<%d>..\n",_fname,t_dwVersion ) );
		return FALSE;
	}

	// 光照信息
	READ_MEM_OFF( &m_bUseGLight,t_fstart,sizeof( bool ) );
	if( !m_bUseGLight )
	{
		READ_MEM_OFF( &m_bGenDLight,t_fstart,sizeof( bool ) );
		READ_MEM_OFF( &m_dwDLight,t_fstart,sizeof( DWORD ) );
		READ_MEM_OFF( &m_fDLightRadius,t_fstart,sizeof( float ) );
	}

	// bbox信息.
	READ_MEM_OFF( &m_bUseBbox,t_fstart,sizeof( bool ) );
	if( m_bUseBbox )
	{
		READ_MEM_OFF( &m_vec3BboxPos,t_fstart,sizeof( osVec3D ) );
		READ_MEM_OFF( &m_vec3BboxSize,t_fstart,sizeof( osVec3D ) );

		// TEST CODE:
		if( m_vec3BboxSize.x < 0.0f )
			m_vec3BboxSize.x = -m_vec3BboxSize.x;
		if( m_vec3BboxSize.y < 0.0f )
			m_vec3BboxSize.y = -m_vec3BboxSize.y;
		if( m_vec3BboxSize.z < 0.0f )
			m_vec3BboxSize.z = -m_vec3BboxSize.z;
	}

	// 特效的元素信息
	READ_MEM_OFF( &m_iEffectItem,t_fstart,sizeof( int ) );
	m_vecEffect = new osc_effectObj[m_iEffectItem];
	for( int t_i=0;t_i<m_iEffectItem;t_i ++ )
		m_vecEffect[t_i].load_effectFromMem( t_fstart,t_dwVersion );

	// 调入声音相关的信息，208版本或是以后，由其它的地方调入处理
	if( t_dwVersion < 208 )
		READ_MEM_OFF( &m_sSndGroup,t_fstart,sizeof( os_effectSndGroup ) );
	// River added @ 2006-6-16: 不使用以前版本的声音信息
	m_sSndGroup.m_szSound[0] = NULL;

	// River @ 2007-6-28:加入特效的播放位置。
	if( t_dwVersion >= 209 )
	{
		READ_MEM_OFF( &m_dwPlayPos,t_fstart,sizeof( DWORD ) );
		READ_MEM_OFF( &m_fPlayBoxScale,t_fstart,sizeof( DWORD ) );

		//! 暂时做为保留数据。
		DWORD   t_dwReserved[8];
		READ_MEM_OFF( t_dwReserved,t_fstart,sizeof( DWORD )*8 );

		if( t_dwReserved[0] == 0 )
		{
			m_fPlayTime = -1.0f;
			osDebugOut( 
				"<%s>特效，没有处理过播放时间信息,请在特效编辑器里调入...\n",_fname );
		}
		else
			m_fPlayTime = *((float*)(&t_dwReserved[0]));		

		if( m_fPlayTime > 100.0f )
			m_fPlayTime = -1.0f;

	}

	//! River @ 2009-5-23:加入了特效在地表decal的播放效果
	if( t_dwVersion >= 210 )
	{
		// 
		//! 读取decal相关的内容
		READ_MEM_OFF( m_szDecalName,t_fstart,sizeof( char )*24 );
		READ_MEM_OFF( &m_fDecalStartTime,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &m_fDecalEndTime,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &m_fDecalSize,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &m_bDecalAlphaAdd,t_fstart,sizeof( BOOL ) );
	}

	//! River added @ 2010-1-14:加入keyTime.
	if( t_dwVersion >= 211 )
	{
		//! 
		READ_MEM_OFF( &m_fKeyTime,t_fstart,sizeof(float) );
	}
	else
		m_fKeyTime = 0.0f;


	// Load Data Success.
	if( !_fileData ) END_USEGBUF( t_iGBufIdx );

	add_ref();
	
	return TRUE;

	unguard;
}

//! 预创建特效，使用特效存在于内存中
void osc_specEffect::pre_createEffectWithNoIns( void )
{
	guard;

	for( int t_i=0;t_i<m_iEffectItem;t_i ++ )
		m_vecEffect[t_i].pre_createDevdevEff();

	return;

	unguard;
}




/////////////////////////////////////////////////////////////////////////////////
//
// 特效实例.
//
/////////////////////////////////////////////////////////////////////////////////
osc_specEffectIns::osc_specEffectIns()
{
	m_ptrEffect = NULL;
	m_iDLightId = -1;
	m_bSceneEffect = false;
	m_fEffInsScale = 1.0f;
	m_bInScaleState = false;
	
	m_bWillBeDelete = false; 

	reset_ins();
}

osc_specEffectIns::~osc_specEffectIns()
{
	delete_curEffect();
}

void osc_specEffectIns::delete_curEffect( bool _finalRelease/* = false*/ )
{
	guard;

	if( !m_ptrEffect )
		return;

	if( m_ptrEffect->m_bGenDLight )
	{
		if( m_iDLightId >= 0 )
		{
			::get_sceneMgr()->delete_dynamicLight( m_iDLightId,0.2f );
			m_iDLightId = -1;
		}
	}

	//! 如果有声音，删除声音
	if( m_sDevdepEffId.m_iSoundId != -1 )
	{
		m_ptrSoundMgr->StopSound( m_sDevdepEffId.m_iSoundId );
		m_sDevdepEffId.m_iSoundId = -1;
	}

	m_ptrEffect->delete_devdepEffIns( m_sDevdepEffId );
	m_ptrEffect->release_ref();

	// River @ 2010-3-20:加入最终释放此特效的效果
	if( _finalRelease )
	{
		while( m_ptrEffect->get_ref() > 0 )
			m_ptrEffect->release_ref();
	}

	m_ptrEffect = NULL;

	m_fEffInsScale = 1.0f;

	reset_ins();

	unguard;
}


//! 创建一个effectIns.
WORD osc_specEffectIns::create_specialEffect( osc_specEffect* _eff,
			                      float _timeScale,osVec3D& _pos,
			                      float _agl,bool _insce/* = false*/,float _scale/* = 1.0f*/ )
{
	guard;

	osassert( _eff );
	osassert( _timeScale >= 0.0f );

	m_sDevdepEffId.reset( _eff->m_iEffectItem );
	
	m_ptrEffect = _eff;
	m_fSpeedScale = _timeScale;
	m_vec3EffPos = _pos;
	m_fAgl = _agl;

	m_bEffectHideState = false;

	m_bWillBeDelete = false;
	
	// 
	if( m_ptrEffect->m_sSndGroup.m_szSound[0] )
		m_bSoundPlayed = FALSE;
	else
		m_bSoundPlayed = TRUE;

	m_fEleTime = 0.0f;

	// River added @ 2009-5-31;初始化为还没有创建地表decal.
	m_bEffectDecalInit = false;

	// 
	// 创建动态光,如果是场景内创建的特效,则不创建动态光
	if( _eff->m_bGenDLight && (!_insce) )
	{
		os_dlightStruct   t_dl;

		t_dl.m_lightColor = m_ptrEffect->m_dwDLight;
		t_dl.m_vec3Pos = _pos;
		t_dl.m_fRadius = m_ptrEffect->m_fDLightRadius;
		t_dl.m_ltype = OSE_DLIGHT_POINT;

		m_iDLightId = get_sceneMgr()->create_dynamicLight( t_dl );
	}

	increase_val();

	set_insInit();

	// River @ 2008-5-7:加入特效的缩放
	osassertex( _scale > 0.0f,"不能出现小于零的特效缩放....\n" );
	m_fEffInsScale = _scale;
	m_fEffInsCreateBaseScale = _scale;
	
	m_bInScaleState = false;

	// 对于场景内的特效，进行特殊的标注
	m_bSceneEffect = _insce;

	return this->get_val();

	unguard;
}

//! 对此特效内声音的处理
void osc_specEffectIns::framemove_sound( void )
{
	guard;

	// 声音正在播放中
	if( m_sDevdepEffId.m_iSoundId != -1 ) 
		return;

	// 如果有需要播放的声音
	if( m_ptrEffect->m_sSndGroup.m_szSound[0] )
	{
		float   t_fStartTime;

		t_fStartTime = m_ptrEffect->get_effPlayTime()*
			        m_ptrEffect->m_sSndGroup.m_fStartTime ;
		if( m_fEleTime >= t_fStartTime )
		{
			//
			os_initSoundSrc   t_sInit;
			DWORD             t_wId;
			t_sInit.m_flMaxDistance = EFFECT_MAXSNDDIS;
			t_sInit.m_flMinDistance = EFFECT_MINSNDDIS;
			t_sInit.m_vPosition = m_vec3EffPos;

			t_wId = m_ptrSoundMgr->PlayFromFile(
				m_ptrEffect->m_sSndGroup.m_szSound,
				&t_sInit,(bool)m_ptrEffect->m_sSndGroup.m_dwType );

			m_bSoundPlayed = TRUE;

			// 
			// 如果是循环播放，则设置声音部分的ID.否则，不需要处理
			if( m_ptrEffect->m_sSndGroup.m_dwType )
				m_sDevdepEffId.m_iSoundId = t_wId;
		}
	}

	return;

	unguard;
}

//! framemove当前的缩放状态
void osc_specEffectIns::scale_frameMove( void )
{
	guard;

	if( !m_bInScaleState )
		return ;

	float t_f = sg_timer::Instance()->get_lastelatime();
	m_fScaleEleTime += t_f;

	if( m_fScaleEleTime >= this->m_fScaleTime )
	{
		m_fEffInsScale = m_fBaseScale * this->m_fNewScale;
		m_bInScaleState = false;
		return;
	}
	else
	{
		float t_fScale = this->m_fScaleEleTime / this->m_fScaleTime;
		m_fEffInsScale  = m_fBaseScale * ((m_fNewScale - 1.0f)*t_fScale + 1.0f);
	}

	return;


	unguard;
}

//! framemove decal的创建
void osc_specEffectIns::framemove_decal( void )
{
	guard;

	// 时间到，创建decal,decalmanager会自动的删除此特效对应的decal.
	if( m_fEleTime < m_ptrEffect->m_fDecalStartTime  )
		return;

	os_decalInit   t_sDecal;
	t_sDecal.m_fVanishTime = 
		m_ptrEffect->m_fDecalEndTime - m_ptrEffect->m_fDecalStartTime;
	t_sDecal.m_fSize = m_ptrEffect->m_fDecalSize;
	if( m_ptrEffect->m_bDecalAlphaAdd )
		t_sDecal.m_iAlphaType = 1;
	else
		t_sDecal.m_iAlphaType = 0;
	strcpy( t_sDecal.m_szDecalTex,m_ptrEffect->m_szDecalName );
	t_sDecal.m_vec3Pos = m_vec3EffPos;
	::get_sceneMgr()->create_decal( t_sDecal );
	m_bEffectDecalInit = true;

	unguard;
}


bool osc_specEffectIns::frame_move( float _time )
{
	guard;

	osassert( _time>=0.0f );

	osc_effectObj*  t_ptrEffObj;
	bool           t_bKeepMove = false;

	// River mod @ 2008-7-16:先是effect内部的scale.
	scale_frameMove();


	// 
	// TEST CODE:
	// RIVER MOD 2006-5-24: 出错的情形可能如下描述：
	// 1: 多线程地图调入程序，正在调入一个特效，查找到一个可以使用的特效实例，
	//    并且把这个特效实例设置为使用,此时特效实例还没有初始化。
	// 2: 此是场景内主线程运行特效更新代码，运行至此，但effect为空...
	// 3: 
	osassertex( m_ptrEffect,
		"请程序调试人员,查看地图调入线程是否正在创建这个特效的实例,以验证错误的根源.....\n" );	

	m_fEleTime += _time;

	//　对声音部分的处理
	if( !m_bSoundPlayed )
		framemove_sound();

	//! River @ 2009-5-23:处理特效对应的decal数据
	if( (!m_bEffectDecalInit) &&
		(m_ptrEffect->m_szDecalName[0] != NULL) )
	{
		framemove_decal();
	}

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( m_sDevdepEffId.can_skipProcess( t_i ) )
			continue;
		else
			t_bKeepMove = true;

		t_ptrEffObj = &m_ptrEffect->m_vecEffect[t_i];

		// 如果播放时间到达，删除单独的小特效
		if( m_sDevdepEffId.used_id( t_i )&&
			(OSE_PLAYWITHTIME == t_ptrEffObj->get_effPlayStyle()) )
		{
			if( (m_fEleTime - t_ptrEffObj->get_startPlayTime()) > 
				t_ptrEffObj->get_effCusPlayData().m_fPlayTime )
			{
				// 即使用对于有自删除功能的实例，比如osa和particle,此处也要删除确认一次
				t_ptrEffObj->delete_devdepEffect(
					m_sDevdepEffId.m_dwEffObjId[t_i] );

				m_sDevdepEffId.m_dwEffObjId[t_i] = -1;
				continue;
			}
		}

		//
		// River added 2006-6-15: 如果是按帧数播放,则创建后每一帧确认此Id是否删除
		if( m_sDevdepEffId.used_id( t_i )&&
			(OSE_PLAYWITHFRAME == t_ptrEffObj->get_effPlayStyle()) )
		{
			if( !t_ptrEffObj->validate_effObj( m_sDevdepEffId.m_dwEffObjId[t_i] ) )
			{
				m_sDevdepEffId.m_dwEffObjId[t_i] = -1;
				continue;
			}
		}

		//
		// River added @ 2006-8-29:对于非循环播放的特效，如果内部删除，则重新创建
		if( m_sDevdepEffId.used_id( t_i )&&
			(OSE_PLAYFOREVER == t_ptrEffObj->get_effPlayStyle() ) )
		{
			if( !t_ptrEffObj->validate_effObj( m_sDevdepEffId.m_dwEffObjId[t_i] ) )
			{
				int   t_id;
				t_id = m_ptrEffect->m_vecEffect[t_i].create_devdepEffect( 
					m_vec3EffPos,m_fAgl,m_fSpeedScale,m_bSceneEffect,m_fEffInsScale );
				m_sDevdepEffId.set_id( t_i,t_id ); 
				// River added @ 2009-10-12:初始化即可以设置osa半透明
				m_sDevdepEffId.alpha_initProcess( t_i,
					m_ptrEffect->m_vecEffect[t_i].get_effObjType() );

				//！River added @ 2009-5-10:如果是创建前隐藏了特效，则此处隐藏。
				if( m_bEffectHideState )
					hide_effectObj( t_i,true );

				continue;
			}
		}


		// 如果已经创建，或是不到创建时间，则不处理此特效元素
		if( (m_fEleTime < m_ptrEffect->m_vecEffect[t_i].get_startPlayTime()) ||
			m_sDevdepEffId.used_id( t_i ) || 
			m_sDevdepEffId.is_created( t_i )  )
			continue;

		// 创建要播放的第t_i个特效元素
		int   t_id;
		t_id = m_ptrEffect->m_vecEffect[t_i].create_devdepEffect( 
			m_vec3EffPos,m_fAgl,m_fSpeedScale,m_bSceneEffect,m_fEffInsScale );
		m_sDevdepEffId.set_id( t_i,t_id ); 
		// River added @ 2009-10-12:初始化即可以设置osa半透明
		m_sDevdepEffId.alpha_initProcess( t_i,
			m_ptrEffect->m_vecEffect[t_i].get_effObjType() );


		//！River added @ 2009-5-10:如果是创建前隐藏了特效，则此处隐藏。
		if( m_bEffectHideState )
			hide_effectObj( t_i,true );


	}

	return t_bKeepMove;

	unguard;
}

//! 重设三维声音的位置
void osc_specEffectIns::reset_effSound( osVec3D& _newpos,float _speed/* = 1.0f*/ )
{	
	guard;

	osVec3D     t_vec3Velocity = _newpos - m_vec3EffPos;

	osVec3Normalize( &t_vec3Velocity,&t_vec3Velocity );
	t_vec3Velocity *= _speed;

	memset( &t_vec3Velocity,0,sizeof( osVec3D ) );
	/**/
	// 如果声音已经在内部被删除，则也不需要处理
	if( !m_ptrSoundMgr->FrameMoveSndPos( 
		m_sDevdepEffId.m_iSoundId,_newpos,&t_vec3Velocity ) )
		m_sDevdepEffId.m_iSoundId = -1;



	unguard;
}


//! 得到当前effect的box.
const os_bbox* osc_specEffectIns::get_effectBox( void )
{
	guard;
	
	static os_bbox   t_sBBox;

	if( m_ptrEffect->m_bUseBbox )
	{
		osVec3D  t_vec3Pos = m_vec3EffPos + m_ptrEffect->m_vec3BboxPos;
		t_sBBox.set_bbPos( t_vec3Pos );
		t_sBBox.set_bbYRot( m_fAgl );
		t_sBBox.set_bbMaxVec( m_ptrEffect->m_vec3BboxSize );
		t_sBBox.set_bbMinVec( -m_ptrEffect->m_vec3BboxSize ); 

		//! 设置bbox的缩放
		t_sBBox.scale_bb( osVec3D( m_fEffInsScale,m_fEffInsScale,m_fEffInsScale) );

		return &t_sBBox;
	}

	return NULL;

	unguard;
}

//! 得到当前effect的box.
bool osc_specEffectIns::get_effectBox( osVec3D& _pos,osVec3D& _size )
{
	guard;

	osassert( m_ptrEffect );

	if( m_ptrEffect->m_bUseBbox )
	{
		_pos = m_vec3EffPos;
		_pos += m_ptrEffect->m_vec3BboxPos;
		_size = m_ptrEffect->m_vec3BboxSize;
		_size *= this->m_fEffInsScale;

		return true;
	}
	else
		return false;

	unguard;
}

//! 隐藏特效内部某一元素。
void osc_specEffectIns::hide_effectObj( int _idx,BOOL _hide )
{
	guard;

	if( !m_sDevdepEffId.is_created( _idx ) )
		return;

	// 已经创建过，并从生命周期内删除.
	if( m_sDevdepEffId.m_dwEffObjId[ _idx ] == 0xffffffff )
		return;


	switch( m_ptrEffect->m_vecEffect[_idx].get_effObjType() )
	{
	case OSE_SEFFECT_OSA:
		{
			osc_specEffMgr::m_ptrScene->hide_osa(
				m_sDevdepEffId.m_dwEffObjId[_idx],_hide );
			break;
		}
	case OSE_SEFFECT_BB:
		{
			osc_specEffMgr::m_ptrScene->hide_billBoard( 
				m_sDevdepEffId.m_dwEffObjId[_idx],_hide );
			break;
		}
	case OSE_SEFFECT_FDP:
		{
			osc_specEffMgr::m_ptrScene->hide_particle(
				m_sDevdepEffId.m_dwEffObjId[_idx],_hide );
			break;
		}		
	default:
		break;
	}

	return;

	unguard;
}


//! 隐藏当前的effect.
void osc_specEffectIns::hide_effect( int _id,bool _hide )
{
	guard;

	m_bEffectHideState = _hide;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// 已经创建过，并从生命周期内删除.
		if( m_sDevdepEffId.m_dwEffObjId[t_i] == 0xffffffff )
			continue;


		switch( m_ptrEffect->m_vecEffect[t_i].get_effObjType() )
		{
		case OSE_SEFFECT_OSA:
			{
				osc_specEffMgr::m_ptrScene->hide_osa(
					m_sDevdepEffId.m_dwEffObjId[t_i],_hide );
				break;
			}
		case OSE_SEFFECT_BB:
			{
				osc_specEffMgr::m_ptrScene->hide_billBoard( 
					m_sDevdepEffId.m_dwEffObjId[t_i],_hide );
				break;
			}
		case OSE_SEFFECT_FDP:
			{
				osc_specEffMgr::m_ptrScene->hide_particle(
					m_sDevdepEffId.m_dwEffObjId[t_i],_hide );
				break;
			}		
		default:
			break;
		}
	}

	unguard;
}


//! 设置effectIns的alpha值，只对osa特效有效
void osc_specEffectIns::set_effectAlpha( int _id,float _alpha )
{
	guard;

	osassert( _alpha >= 0.0f );
	osassert( _alpha <= 1.0f );

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
		{
			m_sDevdepEffId.set_alpha( t_i,_alpha );
			continue;
		}

		// 已经创建过，并从生命周期内删除.
		if( m_sDevdepEffId.m_dwEffObjId[t_i] == 0xffffffff )
			continue;

		switch( m_ptrEffect->m_vecEffect[t_i].get_effObjType() )
		{
		case OSE_SEFFECT_OSA:
			{
				if( _alpha >= 1.0f )
				{
					osc_specEffMgr::m_ptrScene->start_fadeOsa(
						m_sDevdepEffId.m_dwEffObjId[t_i],1.0f,false,1.0f );
				}
				else
				{
					osc_specEffMgr::m_ptrScene->start_fadeOsa(
						m_sDevdepEffId.m_dwEffObjId[t_i],1.0f,true,_alpha );
				}
				break;
			}
		default:
			break;
		}
	}

	unguard;
}


//! fade当前的effect.
void osc_specEffectIns::fade_effIns( float _fadeTime,bool _fadeOut/* = true*/ )
{
	guard;

	osassert( _fadeTime >= 0.0f );

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// 已经创建过，并从生命周期内删除.
		if( m_sDevdepEffId.m_dwEffObjId[t_i] == 0xffffffff )
			continue;


		switch( m_ptrEffect->m_vecEffect[t_i].get_effObjType() )
		{
		case OSE_SEFFECT_OSA:
			{
				osc_specEffMgr::m_ptrScene->start_fadeOsa(
					m_sDevdepEffId.m_dwEffObjId[t_i],_fadeTime,_fadeOut );
				break;
			}
		default:
			break;
		}
	}

	unguard;
}


//! 控制特效的播放速度。
void osc_specEffectIns::scale_effectPlaySpeed( float _speedRate )
{
	guard;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
		{
			// 如果没有创建，则先创建当前的osaObject.
			frame_move( 0.0001f );
			if( !m_sDevdepEffId.is_created(t_i) )
				continue;
		}

		// 已经创建过，并从生命周期内删除.
		if( m_sDevdepEffId.m_dwEffObjId[t_i] == 0xffffffff )
			continue;

		switch( m_ptrEffect->m_vecEffect[t_i].get_effObjType() )
		{
		case OSE_SEFFECT_OSA:
			{
				osc_specEffMgr::m_ptrScene->set_osaPlayTime( 
					m_sDevdepEffId.m_dwEffObjId[t_i],_speedRate );
				break;
			}
		default:
			break;
		}
	}

	return;

	unguard;
}



//! 缩放当前的effect.
void osc_specEffectIns::scale_effIns( float _rate,float _time )
{
	guard;

	osassert( _time >= 0.0f );
	osassert( _rate > 0.0f );

	//! 先设置effect自己的相关数据 
	//  River @ 2010-1-7:以创建时的缩放为基准大小
	m_fBaseScale = m_fEffInsCreateBaseScale;//m_fEffInsScale;
	m_fNewScale = _rate;
	m_fScaleTime = _time;
	m_fScaleEleTime = 0.0f;
	m_bInScaleState = true;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// 已经创建过，并从生命周期内删除.
		if( m_sDevdepEffId.m_dwEffObjId[t_i] == 0xffffffff )
			continue;


		switch( m_ptrEffect->m_vecEffect[t_i].get_effObjType() )
		{
		case OSE_SEFFECT_OSA:
			{
				osc_specEffMgr::m_ptrScene->scale_aniMesh(
					m_sDevdepEffId.m_dwEffObjId[t_i],_rate,_time );
				break;
			}
		case OSE_SEFFECT_BB:
			{
				osc_specEffMgr::m_ptrScene->scale_billBoard(
					     m_sDevdepEffId.m_dwEffObjId[t_i],_rate );
				break;
			}
		default:
			break;
		}
	}

	return;

	unguard;
}


//! 设置当前Effect实例的位置
void osc_specEffectIns::set_effInsPos( osVec3D& _pos,float _agl,
				float _speed/* = 1.0f*/,bool _forceUpdate/* = false*/ )
{
	guard;

	osassert( m_ptrEffect );

	osVec3D    t_vec3FinalPos;
	osMatrix   t_sMat;
	osVec3D    t_vec3Transform;


	// 对声音部分的设置
	if( -1 != m_sDevdepEffId.m_iSoundId )
		reset_effSound( _pos,_speed );

	// 动态光的部分.
	if( -1 != m_iDLightId )
		osc_specEffMgr::m_ptrScene->set_dLightPos( m_iDLightId,_pos );

	// 
	// 如果整个特效旋转的话，相应的相对子特效，需要对应位置的旋转
	m_vec3EffPos = _pos;
	m_fAgl = _agl;

	//! 用于即时的更新场景内的特效位置
	osc_TGManager*    t_ptrScene = (osc_TGManager*)osc_specEffMgr::m_ptrScene;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// 已经创建过，并从生命周期内删除.
		if( m_sDevdepEffId.m_dwEffObjId[t_i] == 0xffffffff )
			continue;

		// River mod @ 2007-7-12:加入设置位置和旋转的时候，也能实时显示旋转。
		t_vec3Transform = m_ptrEffect->m_vecEffect[t_i].m_vec3OffsetPos;	
		
		// River mod @ 2008-7-22:使特效内部元素的位移在缩放发生时，整个特效显示正确
		t_vec3Transform *= m_fEffInsScale;

		if( float_equal( m_fAgl,0.0f ) )
			t_vec3FinalPos = m_vec3EffPos + t_vec3Transform; 
		else
		{
			osMatrix   t_sMat;

			osMatrixRotationY( &t_sMat,m_fAgl );
			osVec3Transform( &t_vec3Transform,&t_vec3Transform,&t_sMat );
			t_vec3FinalPos = m_vec3EffPos + t_vec3Transform; 
		}


		switch( m_ptrEffect->m_vecEffect[t_i].get_effObjType() )
		{
		case OSE_SEFFECT_OSA:
			{
				// zrw修改的bug,此处应该加入旋转的偏移.
				float  t_fNewAngle = 
					_agl + m_ptrEffect->m_vecEffect[t_i].m_fObjRot;

				osc_specEffMgr::m_ptrScene->set_aniMeshPos(
					m_sDevdepEffId.m_dwEffObjId[t_i],t_vec3FinalPos );
				osc_specEffMgr::m_ptrScene->set_aniMeshRot(
					m_sDevdepEffId.m_dwEffObjId[t_i],t_fNewAngle );

				if( _forceUpdate )
					t_ptrScene->update_osaPosNRot( m_sDevdepEffId.m_dwEffObjId[t_i] );

				break;
			}
		case OSE_SEFFECT_BB:
			{
				osc_specEffMgr::m_ptrScene->set_billBoardPos(
					m_sDevdepEffId.m_dwEffObjId[t_i],t_vec3FinalPos );

				if( _forceUpdate )
				{
					t_ptrScene->update_bbPos( 
						m_sDevdepEffId.m_dwEffObjId[t_i],g_ptrCamera );
				}
				break;
			}
		case OSE_SEFFECT_FDP:
			{
				t_ptrScene->set_particlePos( 
					m_sDevdepEffId.m_dwEffObjId[t_i],t_vec3FinalPos,_forceUpdate );
				break;
			}
		default:
			osassertex( false,"错误的特效类型..." );
		}
	}


	unguard;
}


/////////////////////////////////////////////////////////////////////////////////
//
// 特效管理器
//
/////////////////////////////////////////////////////////////////////////////////
I_fdScene* osc_specEffMgr::m_ptrScene = NULL;

//! 声音相关的数据
VEC_speSound osc_specEffMgr::m_vecSpeSound;
VEC_dword osc_specEffMgr::m_dwFileHashVal;


osc_specEffMgr::osc_specEffMgr()
{

	m_arrSEffect.resize( INIT_SEFFECTNUM );
	m_arrSEffectIns.resize( INIT_SEFFECTNUM*4 );

}

osc_specEffMgr::~osc_specEffMgr()
{
	// 
	// 先释放EffectIns,再释放Effect实例
	m_arrSEffectIns.destroy_mgr();
	m_arrSEffect.destroy_mgr();
	m_vecSpeSound.clear();
	m_dwFileHashVal.clear();
}


# define  EFFECT_DIRNAME   "speff\\"

//! 一次性的调入所有的spe文件。
BOOL osc_specEffMgr::load_allEffectFile( const char* _fname )
{
	guard;

	// 
	// 调入场景内所有的特效使用的声音数据
	int               t_size;
	BYTE*             t_fstart;

	SFileHead         t_sHead;
	SFileBuf          t_sFilebuf;


	int  t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( (char*)_fname,t_fstart,TMP_BUFSIZE );
	osassert( t_size>=0 );

	READ_MEM_OFF( &t_sHead,t_fstart,sizeof( SFileHead ) );

	osc_specEffect*    t_ptrEffect;
	BYTE*              t_sBuf = new BYTE[1024*8];
	s_string           t_sFilename;

	for( int t_i = 0 ;t_i<(int)t_sHead.dwFileNum;t_i ++ )
	{
		READ_MEM_OFF( &t_sFilebuf,t_fstart,sizeof( SFileBuf ) );
		osassertex( t_sFilebuf.dwBufLenth < (1024*8),t_sFilebuf.szFileName );
		READ_MEM_OFF( t_sBuf,t_fstart,t_sFilebuf.dwBufLenth );
		
		t_sFilename = EFFECT_DIRNAME;
		t_sFilename += t_sFilebuf.szFileName;

		m_arrSEffect.get_freeNodeAUse( &t_ptrEffect );
		t_ptrEffect->load_effectFromFile( 
			(char*)t_sFilename.c_str(),t_sFilebuf.dwBufLenth,t_sBuf );

		// 
		// 调入特效的场景相关，并设置此特效的声音信息
		os_effectSndGroup   t_snd;
		if( load_effectSndFile( t_sFilename.c_str(),t_snd ) )
			t_ptrEffect->set_effectSndInfo( t_snd );

	}

	END_USEGBUF( t_iGBufIdx );

	delete[] t_sBuf;

	return true;

	unguard;
}


//!
void osc_specEffMgr::onceinit_specEffMgr( void )
{
	guard;

	m_ptrScene = ::get_sceneMgr();
	osc_specEffectIns::m_ptrSoundMgr = ::get_soundManagerPtr();
	osassert( osc_specEffectIns::m_ptrSoundMgr );

	// 如果此文件不存在，返回，播放特效时，没有声音
	if( !file_exist( EFFECT_SND_FILE ) )
		return;


	// 
	// 调入场景内所有的特效使用的声音数据
	int               t_size;
	DWORD             t_dwVersion;
	BYTE*             t_fstart;
	char              t_szMagic[4];

	int   t_iGBufIdx = -1;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( EFFECT_SND_FILE,t_fstart,TMP_BUFSIZE );
	osassert( t_size>=0 );

	READ_MEM_OFF( t_szMagic,t_fstart,sizeof( char )*4 );
	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );

	osassertex( strcmp( t_szMagic,"spe" ) == 0,
		        va("<%s>文件格式错误...\n",EFFECT_SND_FILE )  );
	osassert( t_dwVersion == 0x100 );

	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	m_vecSpeSound.resize( t_dwVersion );

	// 
	READ_MEM_OFF( &m_vecSpeSound[0],t_fstart,sizeof(gcs_speSound)*t_dwVersion );

	END_USEGBUF( t_iGBufIdx );

	// 哈希每一个特效的文件名
	m_dwFileHashVal.resize( t_dwVersion );
	for( DWORD t_i=0;t_i<t_dwVersion;t_i ++ )
		m_dwFileHashVal[t_i] = ::string_hash( m_vecSpeSound[t_i].m_szEffectFile );

# if __PACKFILE_LOAD__
	// River @ 2011-2-15:一次性的调入游戏内用到的所有的spe文件，使运行时更快速
	load_allEffectFile( EFFECT_FILE_LIST );
# endif 

	unguard;
}


//! 调入跟一个特效名同名的特效声音文件
BOOL osc_specEffMgr::load_effectSndFile( const char* _fname,os_effectSndGroup& _snd )
{
	guard;

	int       t_i;
	DWORD     t_dwHashVal;
	s_string  t_sz = _fname;

	std::transform( t_sz.begin(),t_sz.end(),t_sz.begin(),tolower );
	t_dwHashVal = ::string_hash( t_sz.c_str() );

	for( t_i=0;t_i<(int)m_dwFileHashVal.size();t_i ++ )
	{
		if( t_dwHashVal != m_dwFileHashVal[t_i] )
			continue;

		// 如果文件名相同，则返回，否则继续比较
		if( t_sz == m_vecSpeSound[t_i].m_szEffectFile )
		{
			strcpy( _snd.m_szSound,m_vecSpeSound[t_i].m_szSoundFile );
			_snd.m_fStartTime = m_vecSpeSound[t_i].m_fStartTime;
			_snd.m_dwType = m_vecSpeSound[t_i].m_dwPlayType;

			return TRUE;
		}
	}

	return FALSE;
	
	unguard;
}


//! 从现有的Effect查找是否有已经调入的Effect指针
osc_specEffect* osc_specEffMgr::search_effectFromName( const char* _name )
{
	guard;

	DWORD  t_dwHash = string_hash( _name );

	CSpaceMgr<osc_specEffect>::NODE   t_ptrNode;

	for( t_ptrNode = m_arrSEffect.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrSEffect.next_validNode( &t_ptrNode ) )
	{
		if( t_dwHash == t_ptrNode.p->get_effectHashId() )
			return t_ptrNode.p;
	}

	// River @ 2011-2-15:
	// 再次查找，把名字都变成小写，如果还不成功，就真的没有这个effect文件了.
	s_string   t_s1,t_s2;
	t_s1 = _name;
	std::transform( t_s1.begin(), t_s1.end(), t_s1.begin(), tolower );
	for( t_ptrNode = m_arrSEffect.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrSEffect.next_validNode( &t_ptrNode ) )
	{
		t_s2 = t_ptrNode.p->get_effectFName();
		std::transform( t_s2.begin(), t_s2.end(), t_s2.begin(), tolower );
		if( t_s1 == t_s2 )
			return t_ptrNode.p;
	}

	// 
	// River test code @ 2011-3-24:明日热恋内测，此处为调试信息.
	for( t_ptrNode = m_arrSEffect.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrSEffect.next_validNode( &t_ptrNode ) )
	{
		t_s2 = t_ptrNode.p->get_effectFName();
		osDebugOut( "The Effect name is:<%s>...\n",t_s2.c_str() );
	}

	return NULL;

	unguard;
}



//! 得到effect的包围盒，有旋转方向的包围盒.
const os_bbox* osc_specEffMgr::get_effectBox( DWORD _id )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );
	WORD   t_wId = (WORD)_id;
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );
			
			return m_arrSEffectIns[t_wId].get_effectBox();
		}
	}
	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	return false;

	unguard;
}


//! 得到一个effect的bounding box.
bool osc_specEffMgr::get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize  )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );
			
			return m_arrSEffectIns[t_wId].get_effectBox( _pos,_boxSize );
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );


	return false;

	unguard;
}



/** \brief
*  从文件名创建一个场景的effect
*
*  \param _fname 要创建的特效文件名.
*  \param _pos   要创建的特效在场景中的位置。
*  \param _agl   要创建的特效绕Y轴的旋转角度
*/
int osc_specEffMgr::create_sEffect( const char* _fname,osVec3D& _pos,
		                  float _agl,bool _inscene/* = false*/,
						  BOOL _instance/* = TRUE*/,float _scale,/* = 1.0f*/
						  float* _effectPlaytime/* = NULL*/  )
{
	guard;

	osc_specEffect*    t_ptrEffect = NULL;
	osc_specEffectIns*  t_ptrEffectIns = NULL;

	// 得到可用的Effect指针
	t_ptrEffect = search_effectFromName( _fname );

	
# if __PACKFILE_LOAD__
	osassertex( t_ptrEffect,va( "找不到文件<%s>...\n",_fname ) );
# else
	//! River @ 2011-3-10:如果无法找到文件，则创建
	if( !t_ptrEffect )
	{
		m_arrSEffect.get_freeNodeAUse( &t_ptrEffect );
		t_ptrEffect->load_effectFromFile( _fname );

		// 
		// 调入特效的场景相关，并设置此特效的声音信息
		os_effectSndGroup   t_snd;
		if( load_effectSndFile( _fname,t_snd ) )
			t_ptrEffect->set_effectSndInfo( t_snd );
	}
# endif 

	// 预调入数据的处理
	if( !_instance )
	{
		t_ptrEffect->pre_createEffectWithNoIns();
		//! River added @ 2009-6-22:用于上层得到特效的播放时间
		if( _effectPlaytime )
			*_effectPlaytime = t_ptrEffect->get_effPlayTime();
		return -1;
	}
	t_ptrEffect->add_ref();

	// 得到可用的EffectIns指针
	int   t_idx = -1;
	WORD  t_wCV;

	// River @ 2011-2-15: 最小化CS代码，确保效率
	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );
	t_idx = m_arrSEffectIns.get_freeNodeAUse( &t_ptrEffectIns );
	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	t_wCV = t_ptrEffectIns->create_specialEffect( t_ptrEffect,1.0f,_pos,_agl,_inscene,_scale );
	//! River added @ 2009-6-22:用于上层得到特效的播放时间
	if( _effectPlaytime )
		*_effectPlaytime = t_ptrEffect->get_effPlayTime();

	// 
	return osn_mathFunc::syn_dword( t_wCV,t_idx );

	unguard;
}

//! 对EffectMgr进行FrameMove
void osc_specEffMgr::frame_move( void )
{
	guard;

	float   t_fTime = sg_timer::Instance()->get_lastelatime();

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	CSpaceMgr<osc_specEffectIns>::NODE  t_ptrNode;
	for( t_ptrNode = m_arrSEffectIns.begin_usedNode();
		t_ptrNode.p != NULL;
		t_ptrNode = m_arrSEffectIns.next_validNode( &t_ptrNode ) )
	{
		// 确认初始化之后的effect可以frameMove.
		if( t_ptrNode.p->get_insInit() )
		{
			if( !t_ptrNode.p->frame_move( t_fTime ) )
				m_arrSEffectIns.release_node( t_ptrNode.idx );

			//  River @ 2011-2-15:
			//! 如果在其它的地方设置了删除这个effect,则在此处删除.
			if( t_ptrNode.p->get_delete() )
			{
				t_ptrNode.p->delete_curEffect();
				m_arrSEffectIns.release_node( t_ptrNode.idx );
			}
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	unguard;
}

//! 得到特效播放的位置类型 0:包围盒比例播放。 1:攻击位置播放。2:被攻击位置播放。
DWORD osc_specEffMgr::get_effectPlayPosType( int _id )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );
			return m_arrSEffectIns[t_wId].get_effectPlayPosType();
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	return 0;

	unguard;
}

//! 得到特效播放的包围盒比例。
float osc_specEffMgr::get_effectPlayBoxScale( int _id )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );
			return m_arrSEffectIns[t_wId].get_effectPlayBoxScale();
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	return 0.0f;

	unguard;
}


//! 对某一个Effect位置的设定
BOOL osc_specEffMgr::set_effectPos( int _id,osVec3D& _pos,
				float _agl,float _speed/* = 1.0f*/,bool _forceUpdate/* = false*/ )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;
	
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
		{
			// River @ 2009-6-13:强制更新特效位置,用于绑定的特效，精确的位置更新
			m_arrSEffectIns[t_wId].set_effInsPos( _pos,_agl,_speed,_forceUpdate );
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	return TRUE;

	unguard;
}

//! 隐藏或显示当前id的effect.
void osc_specEffMgr::hide_effect(int _id,bool  _bHide/* = true*/ )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;

	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
			m_arrSEffectIns[t_wId].hide_effect( _id,_bHide );
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	unguard;
}


//! 设置特效的alpha值
void osc_specEffMgr::set_effectAlpha( int _id,float _alpha )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;

	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
			m_arrSEffectIns[t_wId].set_effectAlpha( _id,_alpha );
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	return;

	unguard;
}


//! 对特效进行fadeOut
void osc_specEffMgr::fade_effect( int _id,float _fadetime,bool _fadeOut/* = true*/ )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;

	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
			m_arrSEffectIns[t_wId].fade_effIns( _fadetime,_fadeOut );
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	unguard;
}

//! river @ 2010-8-24:用于控制effect的播放速度
void osc_specEffMgr::scale_effectPlaySpeed( int _id,float _speedRate )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;
	
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
			m_arrSEffectIns[t_wId].scale_effectPlaySpeed( _speedRate );
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	unguard;
}


//! 对特效进行缩放,用于对场景内的道具进行缩放动画，但此缩放仅限于osa动画和公告板，粒子场景不能动态缩放
void osc_specEffMgr::scale_effect( int _id,float _rate,float _time )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;
	
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
			m_arrSEffectIns[t_wId].scale_effIns( _rate,_time );
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	unguard;
}

//! 得到一个Effect的keyTime.
float osc_specEffMgr::get_effectKeyTime( int _id )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	WORD   t_wId = (WORD)_id;

	float  t_f = 0;
	
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
			t_f = m_arrSEffectIns[t_wId].get_keyTime();
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	return t_f;

	unguard;
}



//! 删除某一个Effect
void osc_specEffMgr::delete_effect( int _id,bool _finalRelease/* = false*/ )
{
	guard;

	WORD   t_wId = (WORD)_id;

	m_arrSEffectIns[t_wId].set_delete();

	// 
	// River @ 2011-2-15:减少CS操作，减少线程之间的瓶颈.
	/*
	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	//! 确认id和value.
	if( m_arrSEffectIns.validate_id( t_wId ) )
	{
		if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
		{
			m_arrSEffectIns[t_wId].delete_curEffect( _finalRelease );
			m_arrSEffectIns.release_node( t_wId );
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );
	*/

	return ;

	unguard;
}




