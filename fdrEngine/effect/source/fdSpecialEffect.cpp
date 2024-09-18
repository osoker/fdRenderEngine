/////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdSpecialEffect.cpp
 *
 *  Desc:     �����ڵ���Ч�ļ���������������Ч�ļ��ϣ����Ը�������ڳ����༭��
 *            �ڷ��ø��ֶ�������ϣ���Щ����ڵ�������Ʒ�ڳ����б༭�ڱ��Զ��
 *            ��Ч�ڳ����ڱ༭Ҫ���׵Ķࡣ
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

//! ���е���Ч����ʹ����ͬ������������
# define EFFECT_MAXSNDDIS     50.0f
# define EFFECT_MINSNDDIS     8.0f


//! ������Ч��������صĽӿ�
I_soundManager* osc_specEffectIns::m_ptrSoundMgr = NULL;

//! ������Ч���ݵĹ�����
CSpaceMgr<osc_specEffect> osc_specEffMgr::m_arrSEffect;


//! River @ 2009-10-12:���ڴ���һ����ЧԪ�ص�ʱ�����ô�Ԫ��͸��
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
* ���ڴ��е��뵱ǰ��effectԪ�����ݡ�
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

	// �����ڵ���Чֻ��ѭ�����ŵ�����
	switch( m_ePlayStyle )
	{
	case OSE_PLAYFOREVER:
		//!ѭ�����Ų���������
		break;
	case OSE_PLAYWITHFRAME:
		//!����֡��
		READ_MEM_OFF( &m_uPlayData.m_iPlayFrames,_fstart,sizeof( int ) );
		break;
	case OSE_PLAYWITHTIME:
		//!����ʱ��
		READ_MEM_OFF( &m_uPlayData.m_fPlayTime, _fstart, sizeof( float ) );
		break;
	}

	// 
	// 208�İ汾
	if( _fver >= 208 )
	{
		float   t_fAgl;

		// ��ת��Ҫ���ϲ㴫�����ת���ʹ�ã��ݲ�֧��
		READ_MEM_OFF( &t_fAgl,_fstart,sizeof( float ) );

		// River added @ 2--7-6-5:������ת֧��
		m_fObjRot = t_fAgl;

		// ��ֻ֧������
		READ_MEM_OFF( &m_fSizeScale,_fstart,sizeof( float ) );
	}
	else
		m_fObjRot = 0.0f;

	unguard;
}


//! ��֤��ǰ��effectObj�Ƿ��ڿ��õĸ���״̬
BOOL osc_effectObj::validate_effObj( int _id )
{
	guard;

	if( _id == -1 )
		return FALSE;

	switch( m_eEffType )
	{
	case OSE_SEFFECT_OSA:
		// osa����ɾ�����ܣ��˴������Ѿ���osaɾ��
		return osc_specEffMgr::m_ptrScene->validate_aniMeshId( _id );
	case OSE_SEFFECT_BB:
		return osc_specEffMgr::m_ptrScene->validate_bbId( _id );
	case OSE_SEFFECT_FDP:
		// ����Ҳ����ɾ���Ĺ���...
		return osc_specEffMgr::m_ptrScene->validate_parId( _id );
	}

	return FALSE;

	unguard;
}


/** \brief
* ֹͣ���ŵ�ǰ�Ķ�����
*
* \param Int _id Ҫɾ�����豸������ݵ�id.
*/
void osc_effectObj::delete_devdepEffect( int _id )
{
	guard;

	osassert( _id != -1 );
	osassert( _id >= 0 );

	// ���ݲ�ͬ����ЧԪ�����͵����豸��ص���Ч���ݡ�
	switch( m_eEffType )
	{
	case OSE_SEFFECT_OSA:
		{	
			// osa����ɾ�����ܣ��˴������Ѿ���osaɾ��
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
			// ����Ҳ����ɾ���Ĺ���...
			if( osc_specEffMgr::m_ptrScene->validate_parId( _id ) )
				osc_specEffMgr::m_ptrScene->delete_particle( _id );

			break;
		}
	}

	unguard;
}


//! Ԥ������ص��豸�������
void osc_effectObj::pre_createDevdevEff( void )
{
	guard;

	// ���ݲ�ͬ����ЧԪ�����͵����豸��ص���Ч���ݡ�
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

			// ����ǳ����ڵ���Ч����osa�ܻ������Ӱ��
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

			// �����ڵ�������Ч�����ʹ�ù�������ʼ��Ϊ�����ʼ
			osc_specEffMgr::m_ptrScene->create_particle( t_pc,false );

			break;
		}
		
	}

	return ;

	unguard;
}


/** \brief
*  ���ݵ������ЧԪ����Ϣ��������Ӧ�������豸���id.
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

	// ���ݲ�ͬ����ЧԪ�����͵����豸��ص���Ч���ݡ�
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

			// ����ǳ����ڵ���Ч����osa�ܻ������Ӱ��
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

			// �����ڵ�������Ч�����ʹ�ù�������ʼ��Ϊ�����ʼ
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
*  �ͷŵ�ǰ��Դ�Ĵ��麯������release_ref�������á�
*  
*/
void osc_specEffect::release_obj( DWORD _ptr/* = NULL*/ )
{
	guard;

	SAFE_DELETE_ARRAY( m_vecEffect );

	unguard;
}


/** \brief
*  ���뵱ǰ��effect�ļ���
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

	// �ļ�ͷ�Ĵ���
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

	// ������Ϣ
	READ_MEM_OFF( &m_bUseGLight,t_fstart,sizeof( bool ) );
	if( !m_bUseGLight )
	{
		READ_MEM_OFF( &m_bGenDLight,t_fstart,sizeof( bool ) );
		READ_MEM_OFF( &m_dwDLight,t_fstart,sizeof( DWORD ) );
		READ_MEM_OFF( &m_fDLightRadius,t_fstart,sizeof( float ) );
	}

	// bbox��Ϣ.
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

	// ��Ч��Ԫ����Ϣ
	READ_MEM_OFF( &m_iEffectItem,t_fstart,sizeof( int ) );
	m_vecEffect = new osc_effectObj[m_iEffectItem];
	for( int t_i=0;t_i<m_iEffectItem;t_i ++ )
		m_vecEffect[t_i].load_effectFromMem( t_fstart,t_dwVersion );

	// ����������ص���Ϣ��208�汾�����Ժ��������ĵط����봦��
	if( t_dwVersion < 208 )
		READ_MEM_OFF( &m_sSndGroup,t_fstart,sizeof( os_effectSndGroup ) );
	// River added @ 2006-6-16: ��ʹ����ǰ�汾��������Ϣ
	m_sSndGroup.m_szSound[0] = NULL;

	// River @ 2007-6-28:������Ч�Ĳ���λ�á�
	if( t_dwVersion >= 209 )
	{
		READ_MEM_OFF( &m_dwPlayPos,t_fstart,sizeof( DWORD ) );
		READ_MEM_OFF( &m_fPlayBoxScale,t_fstart,sizeof( DWORD ) );

		//! ��ʱ��Ϊ�������ݡ�
		DWORD   t_dwReserved[8];
		READ_MEM_OFF( t_dwReserved,t_fstart,sizeof( DWORD )*8 );

		if( t_dwReserved[0] == 0 )
		{
			m_fPlayTime = -1.0f;
			osDebugOut( 
				"<%s>��Ч��û�д��������ʱ����Ϣ,������Ч�༭�������...\n",_fname );
		}
		else
			m_fPlayTime = *((float*)(&t_dwReserved[0]));		

		if( m_fPlayTime > 100.0f )
			m_fPlayTime = -1.0f;

	}

	//! River @ 2009-5-23:��������Ч�ڵر�decal�Ĳ���Ч��
	if( t_dwVersion >= 210 )
	{
		// 
		//! ��ȡdecal��ص�����
		READ_MEM_OFF( m_szDecalName,t_fstart,sizeof( char )*24 );
		READ_MEM_OFF( &m_fDecalStartTime,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &m_fDecalEndTime,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &m_fDecalSize,t_fstart,sizeof( float ) );
		READ_MEM_OFF( &m_bDecalAlphaAdd,t_fstart,sizeof( BOOL ) );
	}

	//! River added @ 2010-1-14:����keyTime.
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

//! Ԥ������Ч��ʹ����Ч�������ڴ���
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
// ��Чʵ��.
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

	//! �����������ɾ������
	if( m_sDevdepEffId.m_iSoundId != -1 )
	{
		m_ptrSoundMgr->StopSound( m_sDevdepEffId.m_iSoundId );
		m_sDevdepEffId.m_iSoundId = -1;
	}

	m_ptrEffect->delete_devdepEffIns( m_sDevdepEffId );
	m_ptrEffect->release_ref();

	// River @ 2010-3-20:���������ͷŴ���Ч��Ч��
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


//! ����һ��effectIns.
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

	// River added @ 2009-5-31;��ʼ��Ϊ��û�д����ر�decal.
	m_bEffectDecalInit = false;

	// 
	// ������̬��,����ǳ����ڴ�������Ч,�򲻴�����̬��
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

	// River @ 2008-5-7:������Ч������
	osassertex( _scale > 0.0f,"���ܳ���С�������Ч����....\n" );
	m_fEffInsScale = _scale;
	m_fEffInsCreateBaseScale = _scale;
	
	m_bInScaleState = false;

	// ���ڳ����ڵ���Ч����������ı�ע
	m_bSceneEffect = _insce;

	return this->get_val();

	unguard;
}

//! �Դ���Ч�������Ĵ���
void osc_specEffectIns::framemove_sound( void )
{
	guard;

	// �������ڲ�����
	if( m_sDevdepEffId.m_iSoundId != -1 ) 
		return;

	// �������Ҫ���ŵ�����
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
			// �����ѭ�����ţ��������������ֵ�ID.���򣬲���Ҫ����
			if( m_ptrEffect->m_sSndGroup.m_dwType )
				m_sDevdepEffId.m_iSoundId = t_wId;
		}
	}

	return;

	unguard;
}

//! framemove��ǰ������״̬
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

//! framemove decal�Ĵ���
void osc_specEffectIns::framemove_decal( void )
{
	guard;

	// ʱ�䵽������decal,decalmanager���Զ���ɾ������Ч��Ӧ��decal.
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

	// River mod @ 2008-7-16:����effect�ڲ���scale.
	scale_frameMove();


	// 
	// TEST CODE:
	// RIVER MOD 2006-5-24: ��������ο�������������
	// 1: ���̵߳�ͼ����������ڵ���һ����Ч�����ҵ�һ������ʹ�õ���Чʵ����
	//    ���Ұ������Чʵ������Ϊʹ��,��ʱ��Чʵ����û�г�ʼ����
	// 2: ���ǳ��������߳�������Ч���´��룬�������ˣ���effectΪ��...
	// 3: 
	osassertex( m_ptrEffect,
		"����������Ա,�鿴��ͼ�����߳��Ƿ����ڴ��������Ч��ʵ��,����֤����ĸ�Դ.....\n" );	

	m_fEleTime += _time;

	//�����������ֵĴ���
	if( !m_bSoundPlayed )
		framemove_sound();

	//! River @ 2009-5-23:������Ч��Ӧ��decal����
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

		// �������ʱ�䵽�ɾ��������С��Ч
		if( m_sDevdepEffId.used_id( t_i )&&
			(OSE_PLAYWITHTIME == t_ptrEffObj->get_effPlayStyle()) )
		{
			if( (m_fEleTime - t_ptrEffObj->get_startPlayTime()) > 
				t_ptrEffObj->get_effCusPlayData().m_fPlayTime )
			{
				// ��ʹ�ö�������ɾ�����ܵ�ʵ��������osa��particle,�˴�ҲҪɾ��ȷ��һ��
				t_ptrEffObj->delete_devdepEffect(
					m_sDevdepEffId.m_dwEffObjId[t_i] );

				m_sDevdepEffId.m_dwEffObjId[t_i] = -1;
				continue;
			}
		}

		//
		// River added 2006-6-15: ����ǰ�֡������,�򴴽���ÿһ֡ȷ�ϴ�Id�Ƿ�ɾ��
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
		// River added @ 2006-8-29:���ڷ�ѭ�����ŵ���Ч������ڲ�ɾ���������´���
		if( m_sDevdepEffId.used_id( t_i )&&
			(OSE_PLAYFOREVER == t_ptrEffObj->get_effPlayStyle() ) )
		{
			if( !t_ptrEffObj->validate_effObj( m_sDevdepEffId.m_dwEffObjId[t_i] ) )
			{
				int   t_id;
				t_id = m_ptrEffect->m_vecEffect[t_i].create_devdepEffect( 
					m_vec3EffPos,m_fAgl,m_fSpeedScale,m_bSceneEffect,m_fEffInsScale );
				m_sDevdepEffId.set_id( t_i,t_id ); 
				// River added @ 2009-10-12:��ʼ������������osa��͸��
				m_sDevdepEffId.alpha_initProcess( t_i,
					m_ptrEffect->m_vecEffect[t_i].get_effObjType() );

				//��River added @ 2009-5-10:����Ǵ���ǰ��������Ч����˴����ء�
				if( m_bEffectHideState )
					hide_effectObj( t_i,true );

				continue;
			}
		}


		// ����Ѿ����������ǲ�������ʱ�䣬�򲻴������ЧԪ��
		if( (m_fEleTime < m_ptrEffect->m_vecEffect[t_i].get_startPlayTime()) ||
			m_sDevdepEffId.used_id( t_i ) || 
			m_sDevdepEffId.is_created( t_i )  )
			continue;

		// ����Ҫ���ŵĵ�t_i����ЧԪ��
		int   t_id;
		t_id = m_ptrEffect->m_vecEffect[t_i].create_devdepEffect( 
			m_vec3EffPos,m_fAgl,m_fSpeedScale,m_bSceneEffect,m_fEffInsScale );
		m_sDevdepEffId.set_id( t_i,t_id ); 
		// River added @ 2009-10-12:��ʼ������������osa��͸��
		m_sDevdepEffId.alpha_initProcess( t_i,
			m_ptrEffect->m_vecEffect[t_i].get_effObjType() );


		//��River added @ 2009-5-10:����Ǵ���ǰ��������Ч����˴����ء�
		if( m_bEffectHideState )
			hide_effectObj( t_i,true );


	}

	return t_bKeepMove;

	unguard;
}

//! ������ά������λ��
void osc_specEffectIns::reset_effSound( osVec3D& _newpos,float _speed/* = 1.0f*/ )
{	
	guard;

	osVec3D     t_vec3Velocity = _newpos - m_vec3EffPos;

	osVec3Normalize( &t_vec3Velocity,&t_vec3Velocity );
	t_vec3Velocity *= _speed;

	memset( &t_vec3Velocity,0,sizeof( osVec3D ) );
	/**/
	// ��������Ѿ����ڲ���ɾ������Ҳ����Ҫ����
	if( !m_ptrSoundMgr->FrameMoveSndPos( 
		m_sDevdepEffId.m_iSoundId,_newpos,&t_vec3Velocity ) )
		m_sDevdepEffId.m_iSoundId = -1;



	unguard;
}


//! �õ���ǰeffect��box.
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

		//! ����bbox������
		t_sBBox.scale_bb( osVec3D( m_fEffInsScale,m_fEffInsScale,m_fEffInsScale) );

		return &t_sBBox;
	}

	return NULL;

	unguard;
}

//! �õ���ǰeffect��box.
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

//! ������Ч�ڲ�ĳһԪ�ء�
void osc_specEffectIns::hide_effectObj( int _idx,BOOL _hide )
{
	guard;

	if( !m_sDevdepEffId.is_created( _idx ) )
		return;

	// �Ѿ�����������������������ɾ��.
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


//! ���ص�ǰ��effect.
void osc_specEffectIns::hide_effect( int _id,bool _hide )
{
	guard;

	m_bEffectHideState = _hide;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// �Ѿ�����������������������ɾ��.
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


//! ����effectIns��alphaֵ��ֻ��osa��Ч��Ч
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

		// �Ѿ�����������������������ɾ��.
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


//! fade��ǰ��effect.
void osc_specEffectIns::fade_effIns( float _fadeTime,bool _fadeOut/* = true*/ )
{
	guard;

	osassert( _fadeTime >= 0.0f );

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// �Ѿ�����������������������ɾ��.
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


//! ������Ч�Ĳ����ٶȡ�
void osc_specEffectIns::scale_effectPlaySpeed( float _speedRate )
{
	guard;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
		{
			// ���û�д��������ȴ�����ǰ��osaObject.
			frame_move( 0.0001f );
			if( !m_sDevdepEffId.is_created(t_i) )
				continue;
		}

		// �Ѿ�����������������������ɾ��.
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



//! ���ŵ�ǰ��effect.
void osc_specEffectIns::scale_effIns( float _rate,float _time )
{
	guard;

	osassert( _time >= 0.0f );
	osassert( _rate > 0.0f );

	//! ������effect�Լ���������� 
	//  River @ 2010-1-7:�Դ���ʱ������Ϊ��׼��С
	m_fBaseScale = m_fEffInsCreateBaseScale;//m_fEffInsScale;
	m_fNewScale = _rate;
	m_fScaleTime = _time;
	m_fScaleEleTime = 0.0f;
	m_bInScaleState = true;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// �Ѿ�����������������������ɾ��.
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


//! ���õ�ǰEffectʵ����λ��
void osc_specEffectIns::set_effInsPos( osVec3D& _pos,float _agl,
				float _speed/* = 1.0f*/,bool _forceUpdate/* = false*/ )
{
	guard;

	osassert( m_ptrEffect );

	osVec3D    t_vec3FinalPos;
	osMatrix   t_sMat;
	osVec3D    t_vec3Transform;


	// ���������ֵ�����
	if( -1 != m_sDevdepEffId.m_iSoundId )
		reset_effSound( _pos,_speed );

	// ��̬��Ĳ���.
	if( -1 != m_iDLightId )
		osc_specEffMgr::m_ptrScene->set_dLightPos( m_iDLightId,_pos );

	// 
	// ���������Ч��ת�Ļ�����Ӧ���������Ч����Ҫ��Ӧλ�õ���ת
	m_vec3EffPos = _pos;
	m_fAgl = _agl;

	//! ���ڼ�ʱ�ĸ��³����ڵ���Чλ��
	osc_TGManager*    t_ptrScene = (osc_TGManager*)osc_specEffMgr::m_ptrScene;

	for( int t_i=0;t_i<m_ptrEffect->m_iEffectItem;t_i ++ )
	{
		if( !m_sDevdepEffId.is_created(t_i) )
			continue;

		// �Ѿ�����������������������ɾ��.
		if( m_sDevdepEffId.m_dwEffObjId[t_i] == 0xffffffff )
			continue;

		// River mod @ 2007-7-12:��������λ�ú���ת��ʱ��Ҳ��ʵʱ��ʾ��ת��
		t_vec3Transform = m_ptrEffect->m_vecEffect[t_i].m_vec3OffsetPos;	
		
		// River mod @ 2008-7-22:ʹ��Ч�ڲ�Ԫ�ص�λ�������ŷ���ʱ��������Ч��ʾ��ȷ
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
				// zrw�޸ĵ�bug,�˴�Ӧ�ü�����ת��ƫ��.
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
			osassertex( false,"�������Ч����..." );
		}
	}


	unguard;
}


/////////////////////////////////////////////////////////////////////////////////
//
// ��Ч������
//
/////////////////////////////////////////////////////////////////////////////////
I_fdScene* osc_specEffMgr::m_ptrScene = NULL;

//! ������ص�����
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
	// ���ͷ�EffectIns,���ͷ�Effectʵ��
	m_arrSEffectIns.destroy_mgr();
	m_arrSEffect.destroy_mgr();
	m_vecSpeSound.clear();
	m_dwFileHashVal.clear();
}


# define  EFFECT_DIRNAME   "speff\\"

//! һ���Եĵ������е�spe�ļ���
BOOL osc_specEffMgr::load_allEffectFile( const char* _fname )
{
	guard;

	// 
	// ���볡�������е���Чʹ�õ���������
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
		// ������Ч�ĳ�����أ������ô���Ч��������Ϣ
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

	// ������ļ������ڣ����أ�������Чʱ��û������
	if( !file_exist( EFFECT_SND_FILE ) )
		return;


	// 
	// ���볡�������е���Чʹ�õ���������
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
		        va("<%s>�ļ���ʽ����...\n",EFFECT_SND_FILE )  );
	osassert( t_dwVersion == 0x100 );

	READ_MEM_OFF( &t_dwVersion,t_fstart,sizeof( DWORD ) );
	m_vecSpeSound.resize( t_dwVersion );

	// 
	READ_MEM_OFF( &m_vecSpeSound[0],t_fstart,sizeof(gcs_speSound)*t_dwVersion );

	END_USEGBUF( t_iGBufIdx );

	// ��ϣÿһ����Ч���ļ���
	m_dwFileHashVal.resize( t_dwVersion );
	for( DWORD t_i=0;t_i<t_dwVersion;t_i ++ )
		m_dwFileHashVal[t_i] = ::string_hash( m_vecSpeSound[t_i].m_szEffectFile );

# if __PACKFILE_LOAD__
	// River @ 2011-2-15:һ���Եĵ�����Ϸ���õ������е�spe�ļ���ʹ����ʱ������
	load_allEffectFile( EFFECT_FILE_LIST );
# endif 

	unguard;
}


//! �����һ����Ч��ͬ������Ч�����ļ�
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

		// ����ļ�����ͬ���򷵻أ���������Ƚ�
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


//! �����е�Effect�����Ƿ����Ѿ������Effectָ��
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
	// �ٴβ��ң������ֶ����Сд����������ɹ��������û�����effect�ļ���.
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
	// River test code @ 2011-3-24:���������ڲ⣬�˴�Ϊ������Ϣ.
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



//! �õ�effect�İ�Χ�У�����ת����İ�Χ��.
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


//! �õ�һ��effect��bounding box.
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
*  ���ļ�������һ��������effect
*
*  \param _fname Ҫ��������Ч�ļ���.
*  \param _pos   Ҫ��������Ч�ڳ����е�λ�á�
*  \param _agl   Ҫ��������Ч��Y�����ת�Ƕ�
*/
int osc_specEffMgr::create_sEffect( const char* _fname,osVec3D& _pos,
		                  float _agl,bool _inscene/* = false*/,
						  BOOL _instance/* = TRUE*/,float _scale,/* = 1.0f*/
						  float* _effectPlaytime/* = NULL*/  )
{
	guard;

	osc_specEffect*    t_ptrEffect = NULL;
	osc_specEffectIns*  t_ptrEffectIns = NULL;

	// �õ����õ�Effectָ��
	t_ptrEffect = search_effectFromName( _fname );

	
# if __PACKFILE_LOAD__
	osassertex( t_ptrEffect,va( "�Ҳ����ļ�<%s>...\n",_fname ) );
# else
	//! River @ 2011-3-10:����޷��ҵ��ļ����򴴽�
	if( !t_ptrEffect )
	{
		m_arrSEffect.get_freeNodeAUse( &t_ptrEffect );
		t_ptrEffect->load_effectFromFile( _fname );

		// 
		// ������Ч�ĳ�����أ������ô���Ч��������Ϣ
		os_effectSndGroup   t_snd;
		if( load_effectSndFile( _fname,t_snd ) )
			t_ptrEffect->set_effectSndInfo( t_snd );
	}
# endif 

	// Ԥ�������ݵĴ���
	if( !_instance )
	{
		t_ptrEffect->pre_createEffectWithNoIns();
		//! River added @ 2009-6-22:�����ϲ�õ���Ч�Ĳ���ʱ��
		if( _effectPlaytime )
			*_effectPlaytime = t_ptrEffect->get_effPlayTime();
		return -1;
	}
	t_ptrEffect->add_ref();

	// �õ����õ�EffectInsָ��
	int   t_idx = -1;
	WORD  t_wCV;

	// River @ 2011-2-15: ��С��CS���룬ȷ��Ч��
	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );
	t_idx = m_arrSEffectIns.get_freeNodeAUse( &t_ptrEffectIns );
	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	t_wCV = t_ptrEffectIns->create_specialEffect( t_ptrEffect,1.0f,_pos,_agl,_inscene,_scale );
	//! River added @ 2009-6-22:�����ϲ�õ���Ч�Ĳ���ʱ��
	if( _effectPlaytime )
		*_effectPlaytime = t_ptrEffect->get_effPlayTime();

	// 
	return osn_mathFunc::syn_dword( t_wCV,t_idx );

	unguard;
}

//! ��EffectMgr����FrameMove
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
		// ȷ�ϳ�ʼ��֮���effect����frameMove.
		if( t_ptrNode.p->get_insInit() )
		{
			if( !t_ptrNode.p->frame_move( t_fTime ) )
				m_arrSEffectIns.release_node( t_ptrNode.idx );

			//  River @ 2011-2-15:
			//! ����������ĵط�������ɾ�����effect,���ڴ˴�ɾ��.
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

//! �õ���Ч���ŵ�λ������ 0:��Χ�б������š� 1:����λ�ò��š�2:������λ�ò��š�
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

//! �õ���Ч���ŵİ�Χ�б�����
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


//! ��ĳһ��Effectλ�õ��趨
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
			// River @ 2009-6-13:ǿ�Ƹ�����Чλ��,���ڰ󶨵���Ч����ȷ��λ�ø���
			m_arrSEffectIns[t_wId].set_effInsPos( _pos,_agl,_speed,_forceUpdate );
		}
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	return TRUE;

	unguard;
}

//! ���ػ���ʾ��ǰid��effect.
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


//! ������Ч��alphaֵ
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


//! ����Ч����fadeOut
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

//! river @ 2010-8-24:���ڿ���effect�Ĳ����ٶ�
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


//! ����Ч��������,���ڶԳ����ڵĵ��߽������Ŷ������������Ž�����osa�����͹���壬���ӳ������ܶ�̬����
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

//! �õ�һ��Effect��keyTime.
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



//! ɾ��ĳһ��Effect
void osc_specEffMgr::delete_effect( int _id,bool _finalRelease/* = false*/ )
{
	guard;

	WORD   t_wId = (WORD)_id;

	m_arrSEffectIns[t_wId].set_delete();

	// 
	// River @ 2011-2-15:����CS�����������߳�֮���ƿ��.
	/*
	::EnterCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	//! ȷ��id��value.
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




