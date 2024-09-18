///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_creature.h
 *
 *  Desc:     魔法demo客户端用到的人物显示基础类，魔法demo内的怪物，主角人物都从这个类继承
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

// 主角人物和怪物的几个关键动作名
# define   MONSTER_WALK   "跑步"
# define   MONSTER_WALK1  "走路"
# define   HERO_RUN       "单手跑步"
# define   MONSTER_FREE   "待机"
# define   RIDE_ACT       "骑乘"
# define   HERO_FREE      "单手待机" 

# define   MONSTER_FLY     "飞行"
# define   MONSTER_RUN     "跑步"


//! 坐骑跟地面的矩离，坐骑有两种，一种是离地面有一定高度的(龙)，一种是在地面上跑的(马)
# define  FLY_OFFSET      10.0f


class gc_creatureMgr;

struct ChaHeader
{
	ChaHeader():mbUseDirLAsAmbient(FALSE),mAttackedPos(0.0f,0.0f,0.0f)
	{}
	DWORD	mID;		//!< 文件ID
	DWORD	mVersion;	//!< 版本信息

	DWORD	mHeadPicFileNameOfs;	//!< 人物头像文件名偏移,Char类型
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
	//! River added @ 2007-6-4:加入怪物攻击的开始位置
	osVec3D	mStartPos;


	BOOL		mbIsRide;	//!< 是否是坐骑
	osVec3D	mRidePos;	//!< 坐骑微调节

	DWORD	mEffectOfs;
	DWORD	mEffectNr;
	float	mScale;
	DWORD	mReserved4;
};


// 人物伴随特效 
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

	//! 存储cha文件，主要编辑坐骑信息
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

//! 人物说话内容的管理器
class gc_talkStringMgr : public ost_Singleton<gc_talkStringMgr>
{
private:
	//! 全局的说话内容放在一起，用于节省空间
	CSpaceMgr<lm_talkContent>     m_vecTalkContent;

	gc_talkStringMgr();

public:
	/** 得到一个sg_timer的Instance指针.
	 */
	static gc_talkStringMgr*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void         DInstance( void );


	//! 创建一个talkString. 
	int            create_talkString( const lm_talkContent* _tc );  
	//! 删除一个talkString
	void           delete_talkString( int _id );

	//! 得到某一个talkString
	lm_talkContent*    get_talkString( int _id );

};


/** \brief
 *  基础的人物类，怪物，主角人物从此继承。
 *  
 */
class gc_creature : public lm_gameEntity 
{
	friend class gc_creatureMgr;
protected:
	//! 指向的人物指针
	I_skinMeshObj*        m_ptrSkinMesh;

	//! 当前的Creature是否处于激活状态
	BOOL                  m_bActive;

	//@{
	//  移动相关的变量
	//! 怪物的移动速度,可能根据不同的动作有不同的移动速度
	float                 m_fMoveSpeed;
	//! 目标移动位置
	osVec3D               m_vec3DestMovePos;
	//! 是否处于移动状态
	BOOL                  m_bInMoveStatus;
	//! 移动的方向
	float                 m_fMoveDir;
	//@} 

	// 声音相关的变量
	VEC_actSound          m_vecActSound;


	//! 当前正在播放的动作的名字.除了跑步外，其它的动作通常只有一个声音
	char                  m_szCurActName[32];
	BOOL                  m_bActSndHavePlayed;

	//! 调入人物创建相关的资源
	static CMyChaFile     m_sChaFileLoader;

	//! 当前怪物的名字
	char                  m_szCreatureName[16];

	//! 当前怪物所使用的资源目录名
	char                  m_szCreDir[16];

	//@{
	//! 人物说话相关的数据
	BOOL                  m_bInTalkState;
	int                   m_iTalkContentId;
	//@}

	//! 人物是否处于隐藏状态
	BOOL                  m_bInHideState;

	//@{
	//! 怪物是否处于骑乘状态
	BOOL                  m_bInRideState;
	//! 坐骑离地面的矩离
	float                 m_fEarthOffset;
	char                  m_szRidesDir[32];
	I_skinMeshObj*        m_ptrRides;
	//@}

	//! creature似阴影的id
	int                   m_iFakeShadowId;

	//! 创建此creature时,地图还没有调入,位置就会有问题
	BOOL                  m_bRightCrePos;


	// River @ 2007-6-6:加入攻击点和受创点的相关数据
	osVec3D               m_vec3AttackPos;
	osVec3D               m_vec3SufferPos;

	//! 当前creature的目录名
	s_string              m_szCreatureDir;

private:
	const char*           process_creatureDataDir( const char* _dir );

	//! 根据传入的动作名字和类型播放新的动作
	void                  start_newActFromName( const char* _actName,int _type,float _scale = 1.0f );

protected:
	//! 从bfs文件内读入creature的相关创建信息
	bool                  read_bfsInfo( os_skinMeshInit& _smInit  );

	//! 创建当前的人物指针
	virtual BOOL          create_creature( lm_command& _command );

	/** \brief
	 *  得到旗帜数据的贴图数据
	 *
	 *  \param s_string _str 传入的三个stirng数据数组
	 */
	void                  get_flagTexName( lm_command& _command,s_string _str[3] );

	//! 读入怪物声音相关的数据
	void                  read_sndInfo( os_skinMeshInit& _smInit );

	//! 查找某个动作是否有声音信息
	int                   find_actSndInfo( const char* _actName );

	//! 开始主角人物新的动作
	virtual void          start_newAction( lm_command& _command );

	//! 开始执行怪物的休闲状态
	virtual void          start_freeStatus( void );

	//! 处理人物的装备信息
	virtual void          change_euqipment( lm_command& _command );

	//! 开始人物的说话字串显示
	virtual void          start_talkString( lm_command& _command );
	void                  framemove_talkString( float _eletime );

	//! 是否隐藏人物
	void                  creatureHideProcess( lm_command& _command );

	//! 怪物骑乘开始
	BOOL                  start_ride( lm_command& _command );
	//! 结束骑乘
	BOOL                  end_ride( void );

public:
	gc_creature(void);
	~gc_creature(void);


	//! 得到人物应该旋转的方向
	static float          get_charNewDir( osVec3D& _oldpos,osVec3D& _newpos );

	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );

	//! 是否处于激活状态
	virtual BOOL          is_activeChar( void ) { return m_bActive; } 

	//! 得到creatrue的名字
	const char*           get_creatureName( void );

	//! 得到怪物的高度
	float                 get_creatureHeight( void ) ;

	//! 得到怪物的资源目录
	const char*           get_creatureDir( void ) { return m_szCreDir; } 
	//! 得到怪物的动作正常播放时间
	float                 get_creatureActTime( const char* _actName ) ;

	//! 重设当前的Creature.
	void                  reset_creature( void );

	I_skinMeshObj*        get_skinPtr( void ){ return m_ptrSkinMesh; } 
};

//! 从creature类派生出怪物的相关类
class gc_monster : public gc_creature
{
protected:
	//@{
	//  怪物播放随机特效的测试
	float                 m_fLastSkillTime;
	//@}

	//! 设置怪物走一个完整步的时间
	float                 m_fRunActTime;

	//! 对于怪物有坐骑时，使用全样的开始移动函数
	void                  start_monsterRideWalk( lm_command& _command );

	//! 开始执行怪物的走路命令
	void                  start_monsterWalk( lm_command& _command );

	//! 怪物随机特效的测试
	void                  frame_moveRandEffect( float _etime );

	//! 切换怪物的装备

	//! 设置怪物的完整步时间
	void                  set_monsterWalkActTime( lm_command& _command );
	//! 设置怪物的步幅，即移动速度
	void                  set_monsterMoveSpeed( lm_command& _command );

	//! 在场景内创建一个monster
	bool                  create_monster( lm_command& _command );

	//! 尝试找到一个可用的怪物走路动作.
	bool                  try_monsterWalkAction( os_newAction& _newAct );

	//! 走路用到的动作名字,必须使用这个动作名,如果没有设置,则使用原来的动作名
	s_string              m_szWalkActName;

public:
	gc_monster();
	~gc_monster();

	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );

	//! 对当前的monster进行FrameMove.
	void                  frame_move( float _etime );

	//! 释放并重设当前的monster结构
	void                  reset_monster( void );

	//! 设置此怪物走路动作的名字. 
	void                  set_walkActionName( const char* _actName );
	//! 设置此怪物的步幅和步时间
	void                  set_monstWalkInfo( float _stepLength,float _stepTime );

};

//! 从creature基类派生出主角人物的类
class gc_hero : public gc_creature
{
private:
	//! 人物最终要移动到的位置
	osVec3D              m_vec3RealTarget;

	//! 重新计算下一步人物应该到达的直线位置
	bool                 recal_nextPos( void );

	//! 
	bool                 m_bFade;
	float                m_fFadeTime;
protected:
	//! 创建主角人物
	virtual BOOL          create_creature( lm_command& _command );

	//! 更新相机
	void                  update_camera( void );

	//! 对于怪物有坐骑时，使用全样的开始移动函数
	void                  start_monsterRideWalk( lm_command& _command );

	//! 开始主角人物走路状态
	void                  start_heroWalk( lm_command& _command );

	//! 开始人物的攻击动作
	void                  start_attack( lm_command& _command );
	//! 开始主角人物新的动作
	virtual void          start_newAction( lm_command& _command );

	//@{
	//!　当前动作周期内是否两个脚印都显示过了
	bool                  m_bMark1;
	bool                  m_bMark2;
	float                 m_fLastPosePercent;

	//! 脚印的相关信息
	lm_footMarkInfo       m_sFootMarkInfo;	
	lm_footWaterInfo	  m_sFootWaterInfo;
	//! 脚印的decal初始化数据
	os_decalInit          m_sFMInit;
	//! 从人物的声音信息，得到人物的步子相关信息
	int                   m_iSndInfoIdx;
	void                  get_fmInfoFromSndInfo( const char* _chrDir );
	//! frame move footMark.
	void                  footmark_frameMove( float _percent );	
	//! windy mod
	void				  footWater_frameMove( float TimeMs );			
	//@} 

	//! 对动作声音的FrameMove.
	void                  frame_moveActSnd( int _sndIdx );


public:
	gc_hero();
	~gc_hero();

	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );

	//! 更新主角人物的状态
	virtual BOOL          frame_move( float _time,bool _nextUpdate = false );

	//! 重设主角人物的位置
	void                  reset_heroPos( int _x,int _z );

	//! River mod @ 2008-11-9: 设置人物的真实目标点，用于寻路相关
	void                  set_realTarget( osVec3D& _pos ){ m_vec3RealTarget = _pos ;}


	//! 设置人物的马上人物位移.
	void                  add_ridePosOffset( osVec3D& _vecOffset );

	//! 存储人物的坐骑信息调整.
	void                  save_rideOffset( void );

	//! 重设主角人物的位置
	void                  reset_heroPos( osVec3D& _pos );

	//! 设置fade状态
	bool                  set_faceState( bool _fade ){ m_bFade = _fade; }


	//! 主角人物的路步速度，用于调试方便
	static float          m_fRunSpeed;
	//! 测试地表动态光用
	int m_dynamiclight;
	osVec3D		mOldPos;
};


/** \brief
 *  管理整个场景内生物的类.
 */
class gc_creatureMgr : public lm_gameEntity
{
private:
	//! 主角人物的类
	gc_hero               m_sHero;

	//! 场景内怪物的数据集合
	CSpaceMgr<gc_monster>  m_arrMonster;

	//! 鼠标左键点击选中的怪物
	gc_creature*             m_ptrSelCreature;
	std::list<gc_creature*>  m_listMultiSel;

	//! 场景内旗帜怪物的id
	int                    m_iFlagId;

	//! 当前鼠标点击是否点到怪物
	BOOL                   m_bPickMonster;

private:
	//! 场景内插入一个怪物
	gc_monster*           get_freeMonstIns( int& _idx );

	//! 对鼠标左键点击的处理
	void                  process_lbuttonDown( lm_command& _command );


public:
	gc_creatureMgr();
	~gc_creatureMgr();

	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );

	//! 对场景内每一个活动的creature进行FrameMove
	virtual BOOL          frame_move( void );


	//! 得到主角人物的位置.
	virtual void          get_heroFocusPos( osVec3D& _pos );

	//! 重设主角人物的位置
	void                  reset_heroPos( int _x,int _z );

	//! 重设主角人物的位置
	void                  reset_heroPos( osVec3D& _pos );

	//! 是否有激活的选中怪物
	gc_creature*          get_activeSelMonster( void ) { return m_ptrSelCreature; }

	std::list< gc_creature*>& get_multiSel( void ) { return m_listMultiSel; }
	
	//! 根据传入的handle，来得到我们需要的怪物
	gc_creature*          get_creaturePtrByHandle( int _handle );
	//! 根据传入的id来删除怪物
	bool                  del_creatureByHandle( int _handle );

	//! 删除怪物管理器所有的怪物
	void                  del_allCreature( void );


	//! 画出整个怪物管理器内，所有creature的名字
	void                  draw_creatureName( I_fdScene* _scene );

	/** \brief
	 *  调试用途函数，画出所有人物的bounding box,
	 *
	 *  这个函数的调用必须在beginScene和endScene之间
	 */
	void                  draw_creatureBBox( void );

	//! 得到当前主角人物和选中的怪物的指针
	lm_gameEntity*        get_heroEntity( void );
	lm_gameEntity*        get_selCreatureEntity( void );

	//! 测试用
	I_skinMeshObj*        get_heroPtr( void );

	//! 设置管理器内选中的怪物为空
	void                  set_nullSelCreature( void );

	//! 从一个handle，得到此怪物所在的资源目录
	const char*           get_creatureStr( int _handle );


	//! 从一个handle和一个动作名，得到一个动作正常播放需要的时间,如果时间为-1,则没有此动作
	float                 get_creatureActTime( int _handle,const char* _acttime );

	//! 得到上次的鼠标点击是否选中了怪物
	BOOL                  get_monsterPicked( void ) { return m_bPickMonster; }

};


