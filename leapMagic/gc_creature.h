///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_creature.h
 *
 *  Desc:     ħ��demo�ͻ����õ���������ʾ�����࣬ħ��demo�ڵĹ���������ﶼ�������̳�
 * 
 *  His:      River created @ 2006-4-14
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"
# include "gc_charSound.h"
//# include "gc_chaEffect.h"

// ��������͹���ļ����ؼ�������
# define   MONSTER_WALK   "�ܲ�"
# define   MONSTER_WALK1  "��·"
# define   HERO_RUN       "�����ܲ�"
# define   MONSTER_FREE   "����"
# define   RIDE_ACT       "���"
# define   HERO_FREE      "���ִ���" 

# define   MONSTER_FLY     "����"
# define   MONSTER_RUN     "�ܲ�"


//! ���������ľ��룬���������֣�һ�����������һ���߶ȵ�(��)��һ�����ڵ������ܵ�(��)
# define  FLY_OFFSET      10.0f


class gc_creatureMgr;

struct ChaHeader
{
	ChaHeader():mbUseDirLAsAmbient(FALSE),mAttackedPos(0.0f,0.0f,0.0f)
	{}
	DWORD	mID;		//!< �ļ�ID
	DWORD	mVersion;	//!< �汾��Ϣ

	DWORD	mHeadPicFileNameOfs;	//!< ����ͷ���ļ���ƫ��,Char����
	DWORD	mHeadPicFileNameNr;		

	int		mTotalActTypeNum;
	int		mPartNum;

	osVec3D	mBoxMax;
	osVec3D	mBoxMin;

	osVec3D	mPos;
	float		mRot;

	osVec3D	mDeathBoxMax;
	osVec3D	mDeathBoxMin;

	BOOL		mbUseDirLAsAmbient;

	osVec3D	mAttackedPos;
	//! River added @ 2007-6-4:������﹥���Ŀ�ʼλ��
	osVec3D	mStartPos;


	BOOL		mbIsRide;	//!< �Ƿ�������
	osVec3D	mRidePos;	//!< ����΢����

	DWORD	mEffectOfs;
	DWORD	mEffectNr;
	float	mScale;
	DWORD	mReserved4;
};


// ���������Ч 
struct SChaEffectItem
{
	DWORD mNameOfs;	
	DWORD mNameNr;
	osVec3D	mEffectOffset;
};


class CMyChaFile
{
public:
	CMyChaFile();
	~CMyChaFile();
	bool load_chaFile( const char* _filename );

	//! �洢cha�ļ�����Ҫ�༭������Ϣ
	bool save_rideChaFile( const char* _filename,osVec3D& _offset );

	os_skinMeshInit& get_skinMeshInit()	{return m_osSkinMeshInit;}
	
	const char* get_headTexName();
	int get_totalActTypeNum();
	int get_partNum();
	osVec3D& get_boxMax();
	osVec3D& get_boxMin();
	osVec3D& get_deathBoxMax();
	osVec3D& get_deathBoxMin();
	osVec3D& get_attackedPos();
	osVec3D& get_startPos( void ) { return m_header.mStartPos; } 


private:
	ChaHeader       m_header;
	std::string		mHeadPicFileNamStr;
	os_skinMeshInit m_osSkinMeshInit;

};

//! ����˵�����ݵĹ�����
class gc_talkStringMgr : public ost_Singleton<gc_talkStringMgr>
{
private:
	//! ȫ�ֵ�˵�����ݷ���һ�����ڽ�ʡ�ռ�
	CSpaceMgr<lm_talkContent>     m_vecTalkContent;

	gc_talkStringMgr();

public:
	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static gc_talkStringMgr*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void         DInstance( void );


	//! ����һ��talkString. 
	int            create_talkString( const lm_talkContent* _tc );  
	//! ɾ��һ��talkString
	void           delete_talkString( int _id );

	//! �õ�ĳһ��talkString
	lm_talkContent*    get_talkString( int _id );

};


/** \brief
 *  �����������࣬�����������Ӵ˼̳С�
 *  
 */
class gc_creature : public lm_gameEntity 
{
	friend class gc_creatureMgr;
protected:
	//! ָ�������ָ��
	I_skinMeshObj*        m_ptrSkinMesh;

	//! ��ǰ��Creature�Ƿ��ڼ���״̬
	BOOL                  m_bActive;

	//@{
	//  �ƶ���صı���
	//! ������ƶ��ٶ�,���ܸ��ݲ�ͬ�Ķ����в�ͬ���ƶ��ٶ�
	float                 m_fMoveSpeed;
	//! Ŀ���ƶ�λ��
	osVec3D               m_vec3DestMovePos;
	//! �Ƿ����ƶ�״̬
	BOOL                  m_bInMoveStatus;
	//! �ƶ��ķ���
	float                 m_fMoveDir;
	//@} 

	// ������صı���
	VEC_actSound          m_vecActSound;


	//! ��ǰ���ڲ��ŵĶ���������.�����ܲ��⣬�����Ķ���ͨ��ֻ��һ������
	char                  m_szCurActName[32];
	BOOL                  m_bActSndHavePlayed;

	//! �������ﴴ����ص���Դ
	static CMyChaFile     m_sChaFileLoader;

	//! ��ǰ���������
	char                  m_szCreatureName[16];

	//! ��ǰ������ʹ�õ���ԴĿ¼��
	char                  m_szCreDir[16];

	//@{
	//! ����˵����ص�����
	BOOL                  m_bInTalkState;
	int                   m_iTalkContentId;
	//@}

	//! �����Ƿ�������״̬
	BOOL                  m_bInHideState;

	//@{
	//! �����Ƿ������״̬
	BOOL                  m_bInRideState;
	//! ���������ľ���
	float                 m_fEarthOffset;
	char                  m_szRidesDir[32];
	I_skinMeshObj*        m_ptrRides;
	//@}

	//! creature����Ӱ��id
	int                   m_iFakeShadowId;

	//! ������creatureʱ,��ͼ��û�е���,λ�þͻ�������
	BOOL                  m_bRightCrePos;


	// River @ 2007-6-6:���빥������ܴ�����������
	osVec3D               m_vec3AttackPos;
	osVec3D               m_vec3SufferPos;

	//! ��ǰcreature��Ŀ¼��
	s_string              m_szCreatureDir;

private:
	const char*           process_creatureDataDir( const char* _dir );

	//! ���ݴ���Ķ������ֺ����Ͳ����µĶ���
	void                  start_newActFromName( const char* _actName,int _type,float _scale = 1.0f );

protected:
	//! ��bfs�ļ��ڶ���creature����ش�����Ϣ
	bool                  read_bfsInfo( os_skinMeshInit& _smInit  );

	//! ������ǰ������ָ��
	virtual BOOL          create_creature( lm_command& _command );

	/** \brief
	 *  �õ��������ݵ���ͼ����
	 *
	 *  \param s_string _str ���������stirng��������
	 */
	void                  get_flagTexName( lm_command& _command,s_string _str[3] );

	//! �������������ص�����
	void                  read_sndInfo( os_skinMeshInit& _smInit );

	//! ����ĳ�������Ƿ���������Ϣ
	int                   find_actSndInfo( const char* _actName );

	//! ��ʼ���������µĶ���
	virtual void          start_newAction( lm_command& _command );

	//! ��ʼִ�й��������״̬
	virtual void          start_freeStatus( void );

	//! ���������װ����Ϣ
	virtual void          change_euqipment( lm_command& _command );

	//! ��ʼ�����˵���ִ���ʾ
	virtual void          start_talkString( lm_command& _command );
	void                  framemove_talkString( float _eletime );

	//! �Ƿ���������
	void                  creatureHideProcess( lm_command& _command );

	//! ������˿�ʼ
	BOOL                  start_ride( lm_command& _command );
	//! �������
	BOOL                  end_ride( void );

public:
	gc_creature(void);
	~gc_creature(void);


	//! �õ�����Ӧ����ת�ķ���
	static float          get_charNewDir( osVec3D& _oldpos,osVec3D& _newpos );

	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );

	//! �Ƿ��ڼ���״̬
	virtual BOOL          is_activeChar( void ) { return m_bActive; } 

	//! �õ�creatrue������
	const char*           get_creatureName( void );

	//! �õ�����ĸ߶�
	float                 get_creatureHeight( void ) ;

	//! �õ��������ԴĿ¼
	const char*           get_creatureDir( void ) { return m_szCreDir; } 
	//! �õ�����Ķ�����������ʱ��
	float                 get_creatureActTime( const char* _actName ) ;

	//! ���赱ǰ��Creature.
	void                  reset_creature( void );

	I_skinMeshObj*        get_skinPtr( void ){ return m_ptrSkinMesh; } 
};

//! ��creature������������������
class gc_monster : public gc_creature
{
protected:
	//@{
	//  ���ﲥ�������Ч�Ĳ���
	float                 m_fLastSkillTime;
	//@}

	//! ���ù�����һ����������ʱ��
	float                 m_fRunActTime;

	//! ���ڹ���������ʱ��ʹ��ȫ���Ŀ�ʼ�ƶ�����
	void                  start_monsterRideWalk( lm_command& _command );

	//! ��ʼִ�й������·����
	void                  start_monsterWalk( lm_command& _command );

	//! ���������Ч�Ĳ���
	void                  frame_moveRandEffect( float _etime );

	//! �л������װ��

	//! ���ù����������ʱ��
	void                  set_monsterWalkActTime( lm_command& _command );
	//! ���ù���Ĳ��������ƶ��ٶ�
	void                  set_monsterMoveSpeed( lm_command& _command );

	//! �ڳ����ڴ���һ��monster
	bool                  create_monster( lm_command& _command );

	//! �����ҵ�һ�����õĹ�����·����.
	bool                  try_monsterWalkAction( os_newAction& _newAct );

	//! ��·�õ��Ķ�������,����ʹ�����������,���û������,��ʹ��ԭ���Ķ�����
	s_string              m_szWalkActName;

public:
	gc_monster();
	~gc_monster();

	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );

	//! �Ե�ǰ��monster����FrameMove.
	void                  frame_move( float _etime );

	//! �ͷŲ����赱ǰ��monster�ṹ
	void                  reset_monster( void );

	//! ���ô˹�����·����������. 
	void                  set_walkActionName( const char* _actName );
	//! ���ô˹���Ĳ����Ͳ�ʱ��
	void                  set_monstWalkInfo( float _stepLength,float _stepTime );

};

//! ��creature���������������������
class gc_hero : public gc_creature
{
private:
	//! ��������Ҫ�ƶ�����λ��
	osVec3D              m_vec3RealTarget;

	//! ���¼�����һ������Ӧ�õ����ֱ��λ��
	bool                 recal_nextPos( void );

	//! 
	bool                 m_bFade;
	float                m_fFadeTime;
protected:
	//! ������������
	virtual BOOL          create_creature( lm_command& _command );

	//! �������
	void                  update_camera( void );

	//! ���ڹ���������ʱ��ʹ��ȫ���Ŀ�ʼ�ƶ�����
	void                  start_monsterRideWalk( lm_command& _command );

	//! ��ʼ����������·״̬
	void                  start_heroWalk( lm_command& _command );

	//! ��ʼ����Ĺ�������
	void                  start_attack( lm_command& _command );
	//! ��ʼ���������µĶ���
	virtual void          start_newAction( lm_command& _command );

	//@{
	//!����ǰ�����������Ƿ�������ӡ����ʾ����
	bool                  m_bMark1;
	bool                  m_bMark2;
	float                 m_fLastPosePercent;

	//! ��ӡ�������Ϣ
	lm_footMarkInfo       m_sFootMarkInfo;	
	lm_footWaterInfo	  m_sFootWaterInfo;
	//! ��ӡ��decal��ʼ������
	os_decalInit          m_sFMInit;
	//! �������������Ϣ���õ�����Ĳ��������Ϣ
	int                   m_iSndInfoIdx;
	void                  get_fmInfoFromSndInfo( const char* _chrDir );
	//! frame move footMark.
	void                  footmark_frameMove( float _percent );	
	//! windy mod
	void				  footWater_frameMove( float TimeMs );			
	//@} 

	//! �Զ���������FrameMove.
	void                  frame_moveActSnd( int _sndIdx );


public:
	gc_hero();
	~gc_hero();

	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );

	//! �������������״̬
	virtual BOOL          frame_move( float _time,bool _nextUpdate = false );

	//! �������������λ��
	void                  reset_heroPos( int _x,int _z );

	//! River mod @ 2008-11-9: �����������ʵĿ��㣬����Ѱ·���
	void                  set_realTarget( osVec3D& _pos ){ m_vec3RealTarget = _pos ;}


	//! �����������������λ��.
	void                  add_ridePosOffset( osVec3D& _vecOffset );

	//! �洢�����������Ϣ����.
	void                  save_rideOffset( void );

	//! �������������λ��
	void                  reset_heroPos( osVec3D& _pos );

	//! ����fade״̬
	bool                  set_faceState( bool _fade ){ m_bFade = _fade; }


	//! ���������·���ٶȣ����ڵ��Է���
	static float          m_fRunSpeed;
	//! ���Եر�̬����
	int m_dynamiclight;
	osVec3D		mOldPos;
};


/** \brief
 *  ���������������������.
 */
class gc_creatureMgr : public lm_gameEntity
{
private:
	//! �����������
	gc_hero               m_sHero;

	//! �����ڹ�������ݼ���
	CSpaceMgr<gc_monster>  m_arrMonster;

	//! ���������ѡ�еĹ���
	gc_creature*             m_ptrSelCreature;
	std::list<gc_creature*>  m_listMultiSel;

	//! ���������Ĺ����id
	int                    m_iFlagId;

	//! ��ǰ������Ƿ�㵽����
	BOOL                   m_bPickMonster;

private:
	//! �����ڲ���һ������
	gc_monster*           get_freeMonstIns( int& _idx );

	//! ������������Ĵ���
	void                  process_lbuttonDown( lm_command& _command );


public:
	gc_creatureMgr();
	~gc_creatureMgr();

	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );

	//! �Գ�����ÿһ�����creature����FrameMove
	virtual BOOL          frame_move( void );


	//! �õ����������λ��.
	virtual void          get_heroFocusPos( osVec3D& _pos );

	//! �������������λ��
	void                  reset_heroPos( int _x,int _z );

	//! �������������λ��
	void                  reset_heroPos( osVec3D& _pos );

	//! �Ƿ��м����ѡ�й���
	gc_creature*          get_activeSelMonster( void ) { return m_ptrSelCreature; }

	std::list< gc_creature*>& get_multiSel( void ) { return m_listMultiSel; }
	
	//! ���ݴ����handle�����õ�������Ҫ�Ĺ���
	gc_creature*          get_creaturePtrByHandle( int _handle );
	//! ���ݴ����id��ɾ������
	bool                  del_creatureByHandle( int _handle );

	//! ɾ��������������еĹ���
	void                  del_allCreature( void );


	//! ������������������ڣ�����creature������
	void                  draw_creatureName( I_fdScene* _scene );

	/** \brief
	 *  ������;�������������������bounding box,
	 *
	 *  ��������ĵ��ñ�����beginScene��endScene֮��
	 */
	void                  draw_creatureBBox( void );

	//! �õ���ǰ���������ѡ�еĹ����ָ��
	lm_gameEntity*        get_heroEntity( void );
	lm_gameEntity*        get_selCreatureEntity( void );

	//! ������
	I_skinMeshObj*        get_heroPtr( void );

	//! ���ù�������ѡ�еĹ���Ϊ��
	void                  set_nullSelCreature( void );

	//! ��һ��handle���õ��˹������ڵ���ԴĿ¼
	const char*           get_creatureStr( int _handle );


	//! ��һ��handle��һ�����������õ�һ����������������Ҫ��ʱ��,���ʱ��Ϊ-1,��û�д˶���
	float                 get_creatureActTime( int _handle,const char* _acttime );

	//! �õ��ϴε�������Ƿ�ѡ���˹���
	BOOL                  get_monsterPicked( void ) { return m_bPickMonster; }

};


