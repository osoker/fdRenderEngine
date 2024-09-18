/////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdSpecialEffect.h
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
# pragma once

# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"

# include <vector>


//! ��Ч�ļ����ļ�ͷ��
# define   EFFECTFILE_MAGIC     "qut"

//! Ŀǰ��Ч�ļ��İ汾��,ֻ�������µİ汾s
# define   EFFECTFILE_VERSION   207

//! ��Ч�ļ�·������󳤶ȡ�
# define   MAX_EFFNAMELENGTH    32

//! ���е���Чʹ��ͬһ�������ļ�
#define EFFECT_SND_FILE "sound\\g_effect_fd.snd"

//! ������Ч�ļ��ĺϼ�.
# define EFFECT_FILE_LIST "speff\\spe.list"


/** \brief
* ��Ч���͵ı�־��
*/
enum ose_effctType
{
	//! ������
	OSE_SEFFECT_OSA = 0,
	
	//! ����塣
	OSE_SEFFECT_BB ,

	//! ����ϵͳ��
	OSE_SEFFECT_FDP,

	//! �������͵���Ч.

};

union osu_customPlayData
{
	//! �����ʱ�䲥�ţ����ŵ�ʱ�䡣
	float            m_fPlayTime;

	//! ������������ţ����ŵĴ�����
	int              m_iPlayFrames;
};


//! ��Ч��Ӧ����Ч�ṹ
struct gcs_speSound
{
	//! ���������ݶ�Ӧ��effect�ļ���
	char      m_szEffectFile[32];
	//! ��Ч��Ӧ����Ч�ļ�
	char      m_szSoundFile[32];
	//! �������Ч����ʼ����ʱ��,��һ����Ա���
	float     m_fStartTime;
	//! ��������: 0 ��һ���Բ���;1��ѭ������
	DWORD     m_dwPlayType;
	gcs_speSound()
	{
		m_szSoundFile[0] = 0;
		m_fStartTime     = 0.0f;
		m_dwPlayType     = 0;
	}
	gcs_speSound& operator=( const gcs_speSound& _es )
	{
		strcpy( m_szSoundFile, _es.m_szSoundFile );
		m_fStartTime = _es.m_fStartTime;
		m_dwPlayType = _es.m_dwPlayType;
		return *this;
	}
};

typedef std::vector<gcs_speSound>   VEC_speSound;


//! ������ʹ�õ�effectObj��Ŀ
# define MAX_EFFOBJ   8

//! һ��Effect��ʵ��id����.
struct os_effObjCluser
{
private:
	VEC_bool   m_bCreated;

	//! ������ֱ��
	VEC_float  m_fCreatedWithAlpha;

public:
	VEC_dword  m_dwEffObjId;

	//! ����Ч���Ŷ�Ӧ������ID.
	int    m_iSoundId;

public:
	os_effObjCluser()
	{
		m_bCreated.resize( MAX_EFFOBJ );
		m_dwEffObjId.resize( MAX_EFFOBJ );
		m_fCreatedWithAlpha.resize( MAX_EFFOBJ );
		for( int t_i=0;t_i<(int)m_dwEffObjId.size();t_i ++ )
		{
			m_dwEffObjId[t_i] = 0xffffffff;
			m_bCreated[t_i] = false;
			m_fCreatedWithAlpha[t_i] = 1.0f;
		}
		m_iSoundId = -1;
	}
	void   reset( int _effObjNum )
	{ 
		if( _effObjNum > (int)m_dwEffObjId.size() )
		{
			m_bCreated.resize( _effObjNum );
			m_fCreatedWithAlpha.resize( _effObjNum );
			m_dwEffObjId.resize( _effObjNum );
		}

		for( int t_i=0;t_i<(int)m_dwEffObjId.size();t_i ++ )
		{
			m_dwEffObjId[t_i] = 0xffffffff;
			m_bCreated[t_i] = false;
			m_fCreatedWithAlpha[t_i] = 1.0f;
		}
		m_iSoundId = -1;

	}

	void   set_id( int _idx,int _id )
	{
		m_dwEffObjId[_idx] = _id;
		m_bCreated[_idx] = true;
	}

	//! River @ 2009-10-12:��������ĳһ����ЧԪ�ش���������ֱ����һ��alpha͸����
	void   set_alpha( int _idx,float _alpha )
	{
		osassert( _idx < (int)m_fCreatedWithAlpha.size() );
		m_fCreatedWithAlpha[_idx] = _alpha;
	}

	//! River @ 2009-10-12:���ڴ���һ����ЧԪ�ص�ʱ�����ô�Ԫ��͸��
	void   alpha_initProcess( int _id,ose_effctType _etype );


	BOOL   used_id( int _idx ) 
	{ 
		if ( 0xffffffff == m_dwEffObjId[_idx] ) 
			return FALSE;
		else
			return TRUE;
	}

	bool   is_created( int _idx )  { return m_bCreated[_idx]; } 

	//! ��effectʵ���Ƿ��ڿ��Բ��ٴ����״̬
	bool   can_skipProcess( int _idx ) 
	{ 
		// ������־�Ѿ����ã���used_id��-1,��ʾ�������Ź�����Ч����ʱ�Ѿ�ɾ���ˡ�
		// ����������£������棬�����Թ�����
		return (m_bCreated[_idx])&&(!used_id(_idx));
	}

};

/** \brief
 *  ��ЧԪ�أ��п�����*.osa,*.bb,*.fdp��
 *
 *  ������������Ч��һ����ЧԪ�صĲ��š� 
 *  �����Ч��������Ҫ���ڳ����ڵĳ�����Ч��ʹ�ؿ��༭��Ա����һ�����ı༭
 *  �����ڵ���Ч����,����һ�������˸�����Ч��ϵ��ŵ�ȵ�
 *  ֻ�����°汾����Ч���ݣ������İ汾����һ���Դ������
 */
class osc_effectObj
{
	friend class     osc_specEffect;
	friend class     osc_specEffectIns;

private:
	//! ��ǰEffect�����͡�
	ose_effctType     m_eEffType;

	//! ���ŵķ�ʽ��
	ose_amPlayType    m_ePlayStyle;

	//! �����ʱ�䲥�ţ����ŵ�ʱ�䡣
	//! ������������ţ����ŵĴ�����
	osu_customPlayData m_uPlayData;

	//! ��������Ч�У����ŵ�ǰ��ЧԪ�ص�ʱ�̡�
	float            m_fStartPlayTime;

	//! ��ǰԪ������Ч�е����λ�á�
	osVec3D          m_vec3OffsetPos;

	//! ��ǰԪ�ض�Ӧ����Ч�ļ���·�������֡�
	char             m_szEffectName[MAX_EFFNAMELENGTH];

	//! River added @ 2007-6-5:����osa��ת��֧��
	float            m_fObjRot;

	//! ��ǰ��Ч������Ч��
	float            m_fSizeScale;

public:
	osc_effectObj();


	/** \brief
	* ���ڴ��е��뵱ǰ��effectԪ�����ݡ�
	*	
	*\param BYTE*	_fstart 
	*/
	void             load_effectFromMem( BYTE*& _fstart,DWORD _fver );

	/** \brief
	* ֹͣ���ŵ�ǰ�Ķ�����
	*
	* \param Int _id Ҫɾ�����豸������ݵ�id.
	*/
	void             delete_devdepEffect( int _id );

	/** \brief
	 *  ���ݵ������ЧԪ����Ϣ��������Ӧ�������豸���id.
	 */
	int             create_devdepEffect( osVec3D& _woffset,
		                 float _agl = 0.0f, float _fSpeedScale = 1.0f,
						 bool _sceneEff = false,float _sizeScale = 1.0f );

	//! Ԥ������ص��豸�������
	void            pre_createDevdevEff( void );


	//! ��֤��ǰ��effectObj�Ƿ��ڿ��õĸ���״̬
	BOOL            validate_effObj( int _id );


	float           get_startPlayTime( void ) { return m_fStartPlayTime; } 
	ose_amPlayType   get_effPlayStyle( void )  { return m_ePlayStyle; } 
	osu_customPlayData get_effCusPlayData( void ) { return m_uPlayData; }
	ose_effctType      get_effObjType( void )   { return m_eEffType; } 

};



//! ������Ч�ڵ���������.
struct os_effectSndGroup
{
	//! �����ļ�
	char   m_szSound[32];
	//! ��ʼ����ʱ�䣬���ڶ�������ʱ�䲻ȷ�������ʱ����һ����Ա���
	float  m_fStartTime;
	//! ��������,0Ϊһ���Բ���,1Ϊѭ������.
	DWORD  m_dwType;

	os_effectSndGroup& operator=(os_effectSndGroup* _snd )
	{
		strcpy( m_szSound,_snd->m_szSound );
		m_fStartTime = _snd->m_fStartTime;
		m_dwType = _snd->m_dwType;

		return *this;
	}
};

/** \brief
 *  ������һ��������Ч�Ĳ��š�
 */
class osc_specEffect : public os_refObj
{
	friend class             osc_specEffectIns;
protected:

	//! ��ǰeffect���ļ�����
	char                    m_szEffectName[64];
	//! hash id.
	DWORD                   m_dwHashId;

	//! ��ЧԪ�صĸ�����
	int                     m_iEffectItem;

	//! ��ЧԪ�ص����顣
	osc_effectObj*           m_vecEffect;

	//! ��ЧԪ���Ƿ�ʹ��ȫ�ֹ�,��ȫ�ֹ��Ƿ�Ӱ����ЧԪ�ء�
	bool                    m_bUseGLight;

	//@{��Ч��̬����ر���
	//! ��ЧԪ���Ƿ������̬�⡣
	bool                    m_bGenDLight;
	//! ���������̬�⣬��̬�����ɫ,alpha������㣨��Ӱ�죩
	DWORD                   m_dwDLight;
	//! ��̬�ⷶΧ�뾶
	float                   m_fDLightRadius;
	//@}

	//! ��ǰ��Ч��������һ����Ҫ��ʱ��
	float                   m_fPlayTime;
protected:

	/** \brief
	 *  �ͷŵ�ǰ��Դ�Ĵ��麯������release_ref�������á�
	 *  
	 */
	virtual void release_obj( DWORD _ptr = NULL );

private:

	//@{
	//  204�汾�Ժ�������Ч���Ƿ�ʹ�ð�Χ�кͰ�Χ�е����ݡ�
	//! �ǲ�ֻʹ�ð�Χ�С�
	bool                    m_bUseBbox;
	osVec3D                 m_vec3BboxPos;
	osVec3D                 m_vec3BboxSize;
	os_bbox                 m_bbox;

	void                    recal_bbox( void );
	//@} 

	//@{ 
	//! River @ 2007-6-28:209�İ汾����������Ч�Ĳ���λ�ã����治��Ҫ������Щ���������ݸ��ϲ㡣
	//  m_dwPlayPos: 0:���ݰ�Χ�еı������š�1: ����λ�ò��š� 2:������λ�ò��š�
	DWORD                   m_dwPlayPos;
	float                   m_fPlayBoxScale;
	//@}

	//@{
	//! River @ 2009-5-23:����decal��ص����ݡ�
	char                    m_szDecalName[24];
	float                   m_fDecalStartTime;
	float                   m_fDecalEndTime;
	float                   m_fDecalSize;
	//! decal�뱳����ӻ��ǻ��
	BOOL                    m_bDecalAlphaAdd;
	//@} 

	//@{
	//! River @ 2010-1-14:����keyTime.
	float                   m_fKeyTime;
	//@}


	//! ������ص�����
	os_effectSndGroup        m_sSndGroup;


	//! ɾ��һ��effect��ʵ������
	void              delete_devdepEffIns( os_effObjCluser& _id )
	{
		for( int t_i=0;t_i<m_iEffectItem;t_i ++ )
		{
			if( _id.m_dwEffObjId[t_i] == DWORD( -1 ) )
				continue;
			m_vecEffect[t_i].delete_devdepEffect( _id.m_dwEffObjId[t_i] );
			_id.m_dwEffObjId[t_i] = -1;
		}
	}

public:
	osc_specEffect();
	virtual ~osc_specEffect();

	/** \brief
	 *  ���뵱ǰ��effect�ļ���
	 */
	BOOL              load_effectFromFile( const char* _fname,int _size = 0,BYTE*  _fileData = NULL );

	//! �õ���ǰmesh��hash_id��name.
	DWORD             get_effectHashId( void ) { return m_dwHashId; } 
	const char*        get_effectFName( void )  { return m_szEffectName; }


	//! ����������ص���Ϣ��Ŀǰ��Ч��������Ϣ�Ǹ���Чͬ�����ļ����Ժ�������ϳ�һ������ļ�
	void             set_effectSndInfo( os_effectSndGroup& _snd ){ m_sSndGroup = &_snd; }

	//! Ԥ������Ч��ʹ����Ч�������ڴ���
	void             pre_createEffectWithNoIns( void );


	/** \brief
	 *  �õ���ǰEffect��Ҫ�೤ʱ������������һ��.
	 *
	 *  ��һ�εõ����ʱ���ǲ�׼ȷ��ʱ�䣬������Ч�������һ�κ󣬲��ܵõ�����Ч��
	 *  ��׼ȷ��
	 */
	float             get_effPlayTime( void ){ 	return this->m_fPlayTime; }
	//! ����һ��Effect����������һ�ε�ʱ��
	void              set_effPlayTime( float _time ) { this->m_fPlayTime = _time; }


	//! �õ�һ��effect��keyTime.
	float             get_keyTime( void ) { return m_fKeyTime; } 


};



/** \brief
 *  ������Ч��ʵ�����ݣ����ڸ��Ż����ٵ���֯���ݡ�
 *  �ڳ����ж���ط���ʹ��ͬһ����Ч��ʱ�򣬿��Խ�ʡ�������Դ
 *  
 */
class osc_specEffectIns : public os_createVal
{
private:
	//! ��ǰEffectIns��Ӧ�ĺ���ʵ������
	osc_specEffect*          m_ptrEffect;

	//! ��ǰ��Ч��Ӧ�Ķ�̬���id.
	int                     m_iDLightId;

	//! ��Ч���ٺ�ģ����ٱ��� zmg_add 5.28
	float                   m_fSpeedScale;
	//! ��ǰ��Ч��λ�á�
	osVec3D                 m_vec3EffPos;
	float                   m_fAgl;

	//! ��ǰeffectʵ����ÿһ��effectObj��Ӧ���豸�����ЧID.
	os_effObjCluser          m_sDevdepEffId;

	BOOL                     m_bSoundPlayed;

	//! ����Чʵ�������Ѿ���ȥ��ʱ��
	float                   m_fEleTime;

	//! �Ƿ��ǳ�����Ч
	bool                    m_bSceneEffect;


	//! River added @ 2008-5-7:������Ч����ʱ�����Ź���
	float                   m_fEffInsScale;

	//! ���봴��ʱ�Ļ�׼����
	float                   m_fEffInsCreateBaseScale;

	//@{
	//! River mod 2008-7-16:�������Ŷ������೤ʱ�����ɵ�ǰ��sizeScale��׼���ŵ�һ���µĻ�׼
	float                  m_fNewScale;
	float                  m_fBaseScale;
	float                  m_fScaleTime;
	float                  m_fScaleEleTime;
	bool                   m_bInScaleState;
	//@}

	//! River added @ 2009-5-10: ȥ��������Ч���������أ��������ص�bug.
	bool                   m_bEffectHideState;

	//! ��Ч�����ĵر�Decal�Ƿ��Ѿ�����
	bool                   m_bEffectDecalInit;

	//! River @ 2011-2-15:����ɾ����־��ֻ��һ���ط�ɾ����Ч.
	bool                   m_bWillBeDelete;

private:
	//! �Դ���Ч�������Ĵ���
	void                    framemove_sound( void );

	//! ������ά������λ��
	void                    reset_effSound( osVec3D& _newpos,float _speed = 1.0f );

	//! ���ŵ�move
	void                    scale_frameMove();

	//! ������Ч�ڲ�ĳһԪ�ء�
	void                    hide_effectObj( int _idx,BOOL _hide );

	//! framemove decal�Ĵ���
	void                    framemove_decal( void );


public:
	osc_specEffectIns();
	~osc_specEffectIns();

	//! ����һ��effectIns.
	WORD       create_specialEffect( osc_specEffect* _eff,
		                     float _timeScale,osVec3D& _pos,
							 float _agl,bool _insce = false,float _scale = 1.0f );

	
	//! ɾ����ǰ��effectIns
	void       delete_curEffect( bool _finalRelease = false );

	//! ����ɾ��effectIns,ֻ�����߳�ɾ������������CS����Ŀ.
	void       set_delete( void ) { m_bWillBeDelete = true; } 
	bool       get_delete( void ) { return m_bWillBeDelete; } 

	/** \brief
	 *  ����ǰ��Effectʵ��
	 *
	 *  \return bool �������false,��ǰ����Чʵ���Ѿ��رգ��ϲ����ɾ����ʵ��.
	 */
	bool       frame_move( float _time );

	//! ���õ�ǰEffectʵ����λ��
	void       set_effInsPos( osVec3D& _pos,float _agl,
		         float _speed = 1.0f,bool _forceUpdate = false );

	//! ���ŵ�ǰ��effect.
	void       scale_effIns( float _rate,float _time );
	//! fade��ǰ��effect.
	void       fade_effIns( float _fadeTime,bool _fadeOut = true );

	//! ������Ч�Ĳ����ٶȡ�
	void       scale_effectPlaySpeed( float _speedRate );

	//! ���ص�ǰ��effect.
	void       hide_effect( int _id,bool _hide );

	//! ����effectIns��alphaֵ��ֻ��osa��Ч��Ч
	void       set_effectAlpha( int _id,float _alpha );

	//! �õ���ǰeffect��box.
	bool       get_effectBox( osVec3D& _pos,osVec3D& _size );

	//! �õ���ǰeffect��box.
	const os_bbox* get_effectBox( void );

	//! �õ���Ч���ŵ�λ������ 0:��Χ�б������š� 1:����λ�ò��š�2:������λ�ò��š�
	DWORD             get_effectPlayPosType( void ){ return m_ptrEffect->m_dwPlayPos; }
	//! �õ���Ч���ŵİ�Χ�б�����
	float             get_effectPlayBoxScale( void ) { return m_ptrEffect->m_fPlayBoxScale; }


	//! �õ�effect��keyTime.
	float             get_keyTime( void ) { return m_ptrEffect->get_keyTime(); }


	//! ������Ч��������صĽӿ�
	static I_soundManager*  m_ptrSoundMgr;

};


//! �����г�ʼ��effect����Ŀ
# define INIT_SEFFECTNUM  48

/** \brief
 *  ���������ڵ���Ч����������������Ч�ĵ��룬��Чʵ���Ĳ���.
 * 
 */
class osc_specEffMgr
{
private:
	//! ������Ч���ݵĹ�����
	static CSpaceMgr<osc_specEffect>  m_arrSEffect;

	//! ��Чʵ�����ݡ�
	CSpaceMgr<osc_specEffectIns> m_arrSEffectIns;

	/** \brief
	 *  �ڳ�����ʹ�õ���Чʵ������
	 *
	 *  ��Щ���ݲ���ҪFrameMove,������Զ������ȥ��ֻҪɾ��Ϊֹ
	 */
	CSpaceMgr<osc_specEffectIns> m_arrSceneSEIns;

	
	//! ������ص�����
	static VEC_speSound       m_vecSpeSound;
	static VEC_dword          m_dwFileHashVal;

public:
	//! ����ָ��
	static I_fdScene*  m_ptrScene;

private:
	//! �����е�Effect�����Ƿ����Ѿ������Effectָ��
	osc_specEffect*   search_effectFromName( const char* _name );


	//! �����һ����Ч��ͬ������Ч�����ļ�
	static BOOL       load_effectSndFile( const char* _fname,os_effectSndGroup& _snd );

	//! һ���Եĵ������е�spe�ļ���
	static BOOL       load_allEffectFile( const char* _fname );


public:
	osc_specEffMgr();
	~osc_specEffMgr();

	//!
	static void onceinit_specEffMgr( void );

	/** \brief
	 *  ���ļ�������һ��������effect
	 *
	 *  \param _fname Ҫ��������Ч�ļ���.
	 *  \param _pos   Ҫ��������Ч�ڳ����е�λ�á�
	 *  \param _agl   Ҫ��������Ч��Y�����ת�Ƕ�
	 *  \return int   ���ش����õ���Ч��id,�����-1,�����
	 *  \float* _effectPlayTime Ҫ��������Ч�������Ĳ���ʱ�䡣
	 */
	int      create_sEffect( const char* _fname,osVec3D& _pos,
		                     float _agl,bool _inscene = false,
				             BOOL _instance = TRUE,
							 float _scale = 1.0f,
							 float* _effectPlaytime = NULL );



	//! �õ�һ��effect��bounding box.
    bool     get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize );

	//! �õ�effect�İ�Χ�У�����ת����İ�Χ��.
	const os_bbox*   get_effectBox( DWORD _id );


	//! �õ���Ч���ŵ�λ������ 0:��Χ�б������š� 1:����λ�ò��š�2:������λ�ò��š�
	DWORD    get_effectPlayPosType( int _id );
	//! �õ���Ч���ŵİ�Χ�б�����
	float    get_effectPlayBoxScale( int _id );

	//! ��EffectMgr����FrameMove
	void     frame_move( void );

	//! ��ĳһ��Effectλ�õ��趨
	BOOL     set_effectPos( int _id,osVec3D& _pos,
		       float _agl,float _speed = 1.0f,bool _forceUpdate = false );
	//! ɾ��ĳһ��Effect
	void     delete_effect( int _id,bool _finalRelease = false );

	//! �õ�һ��Effect��keyTime.
	float    get_effectKeyTime( int _id );


	//! ����Ч��������,���ڶԳ����ڵĵ��߽������Ŷ������������Ž�����osa�����͹���壬���ӳ������ܶ�̬����
	void     scale_effect( int _id,float _rate,float _time );
	//! ����Ч����fadeOut
	void     fade_effect( int _id,float _fadetime,bool _fadeOut = true );

	//! river @ 2010-8-24:���ڿ���effect�Ĳ����ٶ�
    void     scale_effectPlaySpeed( int _id,float _speedRate );

	//! ������Ч��alphaֵ
	void     set_effectAlpha( int _id,float _alpha );

	//! ���ػ���ʾ��ǰid��effect.
	void     hide_effect(int _id,bool _bHide = true );


	//! ȷ��һ��effect��id���ڲ��Ƿ�ɾ��
	BOOL     validate_effectId( DWORD _id )
	{
		WORD   t_wId = (WORD)_id;

		// River mod @ 2007-8-26:��ĳ������£�����ִ��ִ���
		// ��ʹ�ô��ַ�ʽ������, 65535��-1
		if( t_wId == 65535 )
			return FALSE;

		//! ȷ��id��value.
		if( m_arrSEffectIns.validate_id( t_wId ) )
			if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
				return TRUE;

		return FALSE;
	}

};
