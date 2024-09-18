/////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdSpecialEffect.h
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
# pragma once

# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"

# include <vector>


//! 特效文件的文件头。
# define   EFFECTFILE_MAGIC     "qut"

//! 目前特效文件的版本号,只接受最新的版本s
# define   EFFECTFILE_VERSION   207

//! 特效文件路径的最大长度。
# define   MAX_EFFNAMELENGTH    32

//! 所有的特效使用同一个声音文件
#define EFFECT_SND_FILE "sound\\g_effect_fd.snd"

//! 所有特效文件的合集.
# define EFFECT_FILE_LIST "speff\\spe.list"


/** \brief
* 特效类型的标志。
*/
enum ose_effctType
{
	//! 动画。
	OSE_SEFFECT_OSA = 0,
	
	//! 公告板。
	OSE_SEFFECT_BB ,

	//! 粒子系统。
	OSE_SEFFECT_FDP,

	//! 其它类型的特效.

};

union osu_customPlayData
{
	//! 如果按时间播放，播放的时间。
	float            m_fPlayTime;

	//! 如果按次数播放，播放的次数。
	int              m_iPlayFrames;
};


//! 特效对应的音效结构
struct gcs_speSound
{
	//! 此声音数据对应的effect文件名
	char      m_szEffectFile[32];
	//! 特效对应的音效文件
	char      m_szSoundFile[32];
	//! 相对于特效的起始播放时间,是一个相对比例
	float     m_fStartTime;
	//! 播放类型: 0 是一次性播放;1是循环播放
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


//! 最多可以使用的effectObj数目
# define MAX_EFFOBJ   8

//! 一个Effect的实例id集合.
struct os_effObjCluser
{
private:
	VEC_bool   m_bCreated;

	//! 创建后直接
	VEC_float  m_fCreatedWithAlpha;

public:
	VEC_dword  m_dwEffObjId;

	//! 此特效播放对应的声音ID.
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

	//! River @ 2009-10-12:用于设置某一个特效元素创建出来后，直接有一个alpha透明。
	void   set_alpha( int _idx,float _alpha )
	{
		osassert( _idx < (int)m_fCreatedWithAlpha.size() );
		m_fCreatedWithAlpha[_idx] = _alpha;
	}

	//! River @ 2009-10-12:用于创建一个特效元素的时候，设置此元半透明
	void   alpha_initProcess( int _id,ose_effctType _etype );


	BOOL   used_id( int _idx ) 
	{ 
		if ( 0xffffffff == m_dwEffObjId[_idx] ) 
			return FALSE;
		else
			return TRUE;
	}

	bool   is_created( int _idx )  { return m_bCreated[_idx]; } 

	//! 此effect实例是否处于可以不再处理的状态
	bool   can_skipProcess( int _idx ) 
	{ 
		// 创建标志已经设置，但used_id是-1,表示创建播放过此特效，此时已经删除了。
		// 在这种情况下，返回真，可以略过处理。
		return (m_bCreated[_idx])&&(!used_id(_idx));
	}

};

/** \brief
 *  特效元素，有可能是*.osa,*.bb,*.fdp。
 *
 *  管理了整个特效中一个特效元素的播放。 
 *  这个特效管理器主要用于场景内的场景特效，使关卡编辑人员不用一个个的编辑
 *  场景内的特效数据,比如一个加入了各种特效组合的着点等等
 *  只认最新版本的特效数据，其它的版本可以一次性处理掉。
 */
class osc_effectObj
{
	friend class     osc_specEffect;
	friend class     osc_specEffectIns;

private:
	//! 当前Effect的类型。
	ose_effctType     m_eEffType;

	//! 播放的方式。
	ose_amPlayType    m_ePlayStyle;

	//! 如果按时间播放，播放的时间。
	//! 如果按次数播放，播放的次数。
	osu_customPlayData m_uPlayData;

	//! 在整个特效中，播放当前特效元素的时刻。
	float            m_fStartPlayTime;

	//! 当前元素在特效中的相对位置。
	osVec3D          m_vec3OffsetPos;

	//! 当前元素对应的特效文件的路径与名字。
	char             m_szEffectName[MAX_EFFNAMELENGTH];

	//! River added @ 2007-6-5:加入osa旋转的支持
	float            m_fObjRot;

	//! 当前特效的缩放效果
	float            m_fSizeScale;

public:
	osc_effectObj();


	/** \brief
	* 从内存中调入当前的effect元素数据。
	*	
	*\param BYTE*	_fstart 
	*/
	void             load_effectFromMem( BYTE*& _fstart,DWORD _fver );

	/** \brief
	* 停止播放当前的动画。
	*
	* \param Int _id 要删除的设备相关数据的id.
	*/
	void             delete_devdepEffect( int _id );

	/** \brief
	 *  根据调入的特效元素信息，创建对应的引擎设备相关id.
	 */
	int             create_devdepEffect( osVec3D& _woffset,
		                 float _agl = 0.0f, float _fSpeedScale = 1.0f,
						 bool _sceneEff = false,float _sizeScale = 1.0f );

	//! 预创建相关的设备相关数据
	void            pre_createDevdevEff( void );


	//! 验证当前的effectObj是否处于可用的更新状态
	BOOL            validate_effObj( int _id );


	float           get_startPlayTime( void ) { return m_fStartPlayTime; } 
	ose_amPlayType   get_effPlayStyle( void )  { return m_ePlayStyle; } 
	osu_customPlayData get_effCusPlayData( void ) { return m_uPlayData; }
	ose_effctType      get_effObjType( void )   { return m_eEffType; } 

};



//! 技能特效内的声音数据.
struct os_effectSndGroup
{
	//! 声音文件
	char   m_szSound[32];
	//! 开始播放时间，由于动作播放时间不确定，这个时间是一个相对比例
	float  m_fStartTime;
	//! 播放类型,0为一次性播放,1为循环播放.
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
 *  管理了一个完整特效的播放。
 */
class osc_specEffect : public os_refObj
{
	friend class             osc_specEffectIns;
protected:

	//! 当前effect的文件名。
	char                    m_szEffectName[64];
	//! hash id.
	DWORD                   m_dwHashId;

	//! 特效元素的个数。
	int                     m_iEffectItem;

	//! 特效元素的数组。
	osc_effectObj*           m_vecEffect;

	//! 特效元素是否使用全局光,即全局光是否影响特效元素。
	bool                    m_bUseGLight;

	//@{特效动态光相关变量
	//! 特效元素是否产生动态光。
	bool                    m_bGenDLight;
	//! 如果产生动态光，动态光的颜色,alpha分量随便（不影响）
	DWORD                   m_dwDLight;
	//! 动态光范围半径
	float                   m_fDLightRadius;
	//@}

	//! 当前特效正常播放一次需要的时间
	float                   m_fPlayTime;
protected:

	/** \brief
	 *  释放当前资源的纯虚函数，由release_ref函数调用。
	 *  
	 */
	virtual void release_obj( DWORD _ptr = NULL );

private:

	//@{
	//  204版本以后加入的特效，是否使用包围盒和包围盒的数据。
	//! 是不只使用包围盒。
	bool                    m_bUseBbox;
	osVec3D                 m_vec3BboxPos;
	osVec3D                 m_vec3BboxSize;
	os_bbox                 m_bbox;

	void                    recal_bbox( void );
	//@} 

	//@{ 
	//! River @ 2007-6-28:209的版本，加入了特效的播放位置，引擎不需要处理这些，返回数据给上层。
	//  m_dwPlayPos: 0:根据包围盒的比例播放。1: 攻击位置播放。 2:被攻击位置播放。
	DWORD                   m_dwPlayPos;
	float                   m_fPlayBoxScale;
	//@}

	//@{
	//! River @ 2009-5-23:加入decal相关的内容。
	char                    m_szDecalName[24];
	float                   m_fDecalStartTime;
	float                   m_fDecalEndTime;
	float                   m_fDecalSize;
	//! decal与背景相加还是混合
	BOOL                    m_bDecalAlphaAdd;
	//@} 

	//@{
	//! River @ 2010-1-14:加入keyTime.
	float                   m_fKeyTime;
	//@}


	//! 声音相关的数据
	os_effectSndGroup        m_sSndGroup;


	//! 删除一个effect的实例数据
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
	 *  调入当前的effect文件。
	 */
	BOOL              load_effectFromFile( const char* _fname,int _size = 0,BYTE*  _fileData = NULL );

	//! 得到当前mesh的hash_id和name.
	DWORD             get_effectHashId( void ) { return m_dwHashId; } 
	const char*        get_effectFName( void )  { return m_szEffectName; }


	//! 设置声音相关的信息，目前特效的声音信息是跟特效同名的文件，以后可能整合成一个大的文件
	void             set_effectSndInfo( os_effectSndGroup& _snd ){ m_sSndGroup = &_snd; }

	//! 预创建特效，使用特效存在于内存中
	void             pre_createEffectWithNoIns( void );


	/** \brief
	 *  得到当前Effect需要多长时间能正常播放一次.
	 *
	 *  第一次得到这个时间是不准确的时间，必须特效播放完成一次后，才能得到此特效的
	 *  稍准确的
	 */
	float             get_effPlayTime( void ){ 	return this->m_fPlayTime; }
	//! 设置一个Effect的正常播放一次的时间
	void              set_effPlayTime( float _time ) { this->m_fPlayTime = _time; }


	//! 得到一个effect的keyTime.
	float             get_keyTime( void ) { return m_fKeyTime; } 


};



/** \brief
 *  场景特效的实例数据，用于更优化快速的组织数据。
 *  在场景中多个地方都使用同一个特效的时候，可以节省更多的资源
 *  
 */
class osc_specEffectIns : public os_createVal
{
private:
	//! 当前EffectIns对应的核心实例数据
	osc_specEffect*          m_ptrEffect;

	//! 当前特效对应的动态光的id.
	int                     m_iDLightId;

	//! 特效加速后的，加速倍数 zmg_add 5.28
	float                   m_fSpeedScale;
	//! 当前特效的位置。
	osVec3D                 m_vec3EffPos;
	float                   m_fAgl;

	//! 当前effect实例内每一个effectObj对应的设备相关特效ID.
	os_effObjCluser          m_sDevdepEffId;

	BOOL                     m_bSoundPlayed;

	//! 从特效实例播放已经过去的时间
	float                   m_fEleTime;

	//! 是否是场景特效
	bool                    m_bSceneEffect;


	//! River added @ 2008-5-7:加入特效创建时的缩放功能
	float                   m_fEffInsScale;

	//! 加入创建时的基准缩放
	float                   m_fEffInsCreateBaseScale;

	//@{
	//! River mod 2008-7-16:加入缩放动画，多长时间内由当前的sizeScale基准缩放到一个新的基准
	float                  m_fNewScale;
	float                  m_fBaseScale;
	float                  m_fScaleTime;
	float                  m_fScaleEleTime;
	bool                   m_bInScaleState;
	//@}

	//! River added @ 2009-5-10: 去掉创建特效后，马上隐藏，不能隐藏的bug.
	bool                   m_bEffectHideState;

	//! 特效产生的地表Decal是否已经创建
	bool                   m_bEffectDecalInit;

	//! River @ 2011-2-15:设置删除标志，只在一个地方删除特效.
	bool                   m_bWillBeDelete;

private:
	//! 对此特效内声音的处理
	void                    framemove_sound( void );

	//! 重设三维声音的位置
	void                    reset_effSound( osVec3D& _newpos,float _speed = 1.0f );

	//! 缩放的move
	void                    scale_frameMove();

	//! 隐藏特效内部某一元素。
	void                    hide_effectObj( int _idx,BOOL _hide );

	//! framemove decal的创建
	void                    framemove_decal( void );


public:
	osc_specEffectIns();
	~osc_specEffectIns();

	//! 创建一个effectIns.
	WORD       create_specialEffect( osc_specEffect* _eff,
		                     float _timeScale,osVec3D& _pos,
							 float _agl,bool _insce = false,float _scale = 1.0f );

	
	//! 删除当前的effectIns
	void       delete_curEffect( bool _finalRelease = false );

	//! 设置删除effectIns,只在主线程删除操作，减少CS的数目.
	void       set_delete( void ) { m_bWillBeDelete = true; } 
	bool       get_delete( void ) { return m_bWillBeDelete; } 

	/** \brief
	 *  处理当前的Effect实例
	 *
	 *  \return bool 如果返回false,则当前的特效实例已经关闭，上层可以删除此实例.
	 */
	bool       frame_move( float _time );

	//! 设置当前Effect实例的位置
	void       set_effInsPos( osVec3D& _pos,float _agl,
		         float _speed = 1.0f,bool _forceUpdate = false );

	//! 缩放当前的effect.
	void       scale_effIns( float _rate,float _time );
	//! fade当前的effect.
	void       fade_effIns( float _fadeTime,bool _fadeOut = true );

	//! 控制特效的播放速度。
	void       scale_effectPlaySpeed( float _speedRate );

	//! 隐藏当前的effect.
	void       hide_effect( int _id,bool _hide );

	//! 设置effectIns的alpha值，只对osa特效有效
	void       set_effectAlpha( int _id,float _alpha );

	//! 得到当前effect的box.
	bool       get_effectBox( osVec3D& _pos,osVec3D& _size );

	//! 得到当前effect的box.
	const os_bbox* get_effectBox( void );

	//! 得到特效播放的位置类型 0:包围盒比例播放。 1:攻击位置播放。2:被攻击位置播放。
	DWORD             get_effectPlayPosType( void ){ return m_ptrEffect->m_dwPlayPos; }
	//! 得到特效播放的包围盒比例。
	float             get_effectPlayBoxScale( void ) { return m_ptrEffect->m_fPlayBoxScale; }


	//! 得到effect的keyTime.
	float             get_keyTime( void ) { return m_ptrEffect->get_keyTime(); }


	//! 播放特效的声音相关的接口
	static I_soundManager*  m_ptrSoundMgr;

};


//! 场景中初始化effect的数目
# define INIT_SEFFECTNUM  48

/** \brief
 *  整个场景内的特效管理器，管理了特效的调入，特效实例的播放.
 * 
 */
class osc_specEffMgr
{
private:
	//! 核心特效数据的管理器
	static CSpaceMgr<osc_specEffect>  m_arrSEffect;

	//! 特效实例数据。
	CSpaceMgr<osc_specEffectIns> m_arrSEffectIns;

	/** \brief
	 *  在场景中使用的特效实例数据
	 *
	 *  这些数据不需要FrameMove,可以永远播放下去，只要删除为止
	 */
	CSpaceMgr<osc_specEffectIns> m_arrSceneSEIns;

	
	//! 声音相关的数据
	static VEC_speSound       m_vecSpeSound;
	static VEC_dword          m_dwFileHashVal;

public:
	//! 场景指针
	static I_fdScene*  m_ptrScene;

private:
	//! 从现有的Effect查找是否有已经调入的Effect指针
	osc_specEffect*   search_effectFromName( const char* _name );


	//! 调入跟一个特效名同名的特效声音文件
	static BOOL       load_effectSndFile( const char* _fname,os_effectSndGroup& _snd );

	//! 一次性的调入所有的spe文件。
	static BOOL       load_allEffectFile( const char* _fname );


public:
	osc_specEffMgr();
	~osc_specEffMgr();

	//!
	static void onceinit_specEffMgr( void );

	/** \brief
	 *  从文件名创建一个场景的effect
	 *
	 *  \param _fname 要创建的特效文件名.
	 *  \param _pos   要创建的特效在场景中的位置。
	 *  \param _agl   要创建的特效绕Y轴的旋转角度
	 *  \return int   返回创建好的特效的id,如果是-1,则出错
	 *  \float* _effectPlayTime 要创建的特效，正常的播放时间。
	 */
	int      create_sEffect( const char* _fname,osVec3D& _pos,
		                     float _agl,bool _inscene = false,
				             BOOL _instance = TRUE,
							 float _scale = 1.0f,
							 float* _effectPlaytime = NULL );



	//! 得到一个effect的bounding box.
    bool     get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize );

	//! 得到effect的包围盒，有旋转方向的包围盒.
	const os_bbox*   get_effectBox( DWORD _id );


	//! 得到特效播放的位置类型 0:包围盒比例播放。 1:攻击位置播放。2:被攻击位置播放。
	DWORD    get_effectPlayPosType( int _id );
	//! 得到特效播放的包围盒比例。
	float    get_effectPlayBoxScale( int _id );

	//! 对EffectMgr进行FrameMove
	void     frame_move( void );

	//! 对某一个Effect位置的设定
	BOOL     set_effectPos( int _id,osVec3D& _pos,
		       float _agl,float _speed = 1.0f,bool _forceUpdate = false );
	//! 删除某一个Effect
	void     delete_effect( int _id,bool _finalRelease = false );

	//! 得到一个Effect的keyTime.
	float    get_effectKeyTime( int _id );


	//! 对特效进行缩放,用于对场景内的道具进行缩放动画，但此缩放仅限于osa动画和公告板，粒子场景不能动态缩放
	void     scale_effect( int _id,float _rate,float _time );
	//! 对特效进行fadeOut
	void     fade_effect( int _id,float _fadetime,bool _fadeOut = true );

	//! river @ 2010-8-24:用于控制effect的播放速度
    void     scale_effectPlaySpeed( int _id,float _speedRate );

	//! 设置特效的alpha值
	void     set_effectAlpha( int _id,float _alpha );

	//! 隐藏或显示当前id的effect.
	void     hide_effect(int _id,bool _bHide = true );


	//! 确认一个effect的id在内部是否删除
	BOOL     validate_effectId( DWORD _id )
	{
		WORD   t_wId = (WORD)_id;

		// River mod @ 2007-8-26:在某种情况下，会出现此种错误，
		// 暂使用此种方式来处理, 65535即-1
		if( t_wId == 65535 )
			return FALSE;

		//! 确认id和value.
		if( m_arrSEffectIns.validate_id( t_wId ) )
			if( m_arrSEffectIns[t_wId].validate_cval( _id ) )
				return TRUE;

		return FALSE;
	}

};
