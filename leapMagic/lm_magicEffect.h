///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_magicEffect.h
 *
 *  Desc:     魔法demo客户端用到的魔法特效相关功能类
 * 
 *  His:      River created @ 2006-5-23
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"


//! 技能特效播放时间方式
enum SKILL_EFFECT_TIMESTYLE
{
	SET_ORDER_SECTION3,   //按时间顺序播放,3段动画 
	SET_SYNCHRO,          //所有特效同步播放,一个技能最多存有3个特效
};

//! 描述一个技能中的一个特效(spe)如何进行播放
enum SKILL_EFFECT_PLAYSTYLE
{
	SEP_FOLLOW_LAUNCHER_CENTER_FOREVER,				//! 跟随发起者一直播放，以发起者的身体中心为原点 ___

	SEP_FOLLOW_TARGET,                      //! 跟随玩家的攻击点播放

	SEP_POSITION_ONETIMES,							//! 在指定的位置上播放一次特效               ___陨石
	
	SEP_NONE,
};

//! 一个技能最多可以有的特效数目
# define MAX_EFFECTPERSKILL    3

struct SKILLDATA
{
	int id;
	char name[40];
	bool bAction;
	char actionName[40];
	char actionName2[40]; //施法动作
	ose_amPlayType actionType;
	SKILL_EFFECT_TIMESTYLE 	effectTimeStyle;
	char effectFilename[MAX_EFFECTPERSKILL][40];
	SKILL_EFFECT_PLAYSTYLE effectType[MAX_EFFECTPERSKILL];
	float motionSpeed;
	float motionAdjRadian;

	//char stateEffectFilename[40];         //! 使用此技能后受影响都所带有的状态特效名(循环)
	float effectStartTime;                     //! 特效起始播放时间
	char	reserve[27];				// 备用字段
	bool	bIsPlayEffectBeHit;			//是否在被攻击点播放特效
	float followEffectStartTime;   // 伴随特效起始播放时间
	float followEffectEndTime;     // 伴随特效终止播放时间
	char followEffectFilename[40];        //! 使用此技能时在发动者身边的伴随特效(循环)

	int iconid;
	char desc[256];

	// 包装此函数，加入全局的施法动作速度改变
	float  get_effStartTime( void ) const ;
	//
	float  get_followEffEndTime( void ) const;


};
typedef std::vector<SKILLDATA>   VEC_skillData;


//! 跟随攻击点播放的特效使用统一的播放时间
# define   FOLLOWATTACK_EFFECTTIME   10.0f


//
// 播放特效的操作：
// 按下快捷键的时候，发出对特效管理器的命令。
// 特效管理器收到命令后，初始化特效，然后发给主角人物改变对应动作的命令。
// 此特效开始FrameMove,内部计时，到相应的时间后播放特效。
// 完成整个特效播放后，删除相应的特效ins.

/** \brief
 *  魔法播放的类。
 *
 *  对一个技能的播放进行管理，使用引擎内部的特效播放接口，
 *  对每一个特效的创建，播放和删除进行管理。
 */
class lm_magicEffectIns : public os_createVal
{
private:
	//! 引擎此特效的ID.
	const SKILLDATA*     m_ptrSkillData;

	//! 特效播放相关的数据
	lm_skillPlay         m_sSkillPlay;

	//! 当前特效偏移时间,这个时间是相对于动作开始播放后的时间偏移值
	float                m_fEleTime;

	//@{ 技能特效播放相关数据
	//! 是否已经到达到特效播放开始点
	BOOL                 m_bArrivedEffStartPt;
	//! 中间的攻击过程特效是否到达攻击目标，比如火球术的火球
	BOOL                 m_bArrivedAttackTarget;
	float                m_fDisFromStart;
	float                m_fMoveEffAgl;
	//! 是否蓄力过程结束
	BOOL                 m_bAmassPowerEnd;

	//! 每一个特效实例对应的id
	DWORD                m_dwEffInsId[MAX_EFFECTPERSKILL];
	BOOL                 m_bSkillEffPlayed[MAX_EFFECTPERSKILL];
	//@}


	//@{
	//! 伴随特效的播放相关数据
	BOOL                 m_bArrivedFolEffStartPt;
	DWORD                m_dwFolEffId;
	BOOL                 m_bFollowEffEnd;
	//@}

	//@{
	//! 跟随攻击点相关特效的数据
	BOOL                 m_bPlayingFollowAttack;
	float                m_fFollowAttackTime;
	osVec3D              m_vec3LastFrameAttackPos;
	const osVec3D*       m_ptrAttackPos;
	//@}

	//! 人物播放畜力特效的时间
	float                m_fFollowEffectEndTime;


	//! 特效的调整高度
	float                m_fSkillAdjHeight;

private:

	//! 判断一个技能特效是否全部播放完成,在函数frameMove_effectInsBePlaying最后调用
	BOOL                 is_skillEffectPlaying( void );

	/** \brief
	 *  当前的EffectIns是否在使用中,如果全部播放播放完，则返回false.
	 *
	 *  对于需要设置位置的特效，在此函数内设置位置。
	 */
	BOOL                 frameMove_effectInsBePlaying( float _eletime );

	//! 对伴随特效的处理
	BOOL                 frame_moveFollowEff( void );

	//! 对蓄力特效进行处理。
	void                 framemove_amassPower( int _idx );

	//! 对过程特效进行处理，如火球术中，从玩家到怪物的火球
	void                 framemove_moveEffect( int _idx );

	//! 对跟随攻击点播放的特效进行FrameMove
	void                 framemove_followAttackEffect( int _idx );

	float                get_amassPowerTime( void );

public:
	lm_magicEffectIns(void);
	~lm_magicEffectIns(void);

	//! 创建effect并播放
	BOOL                 init_magicEffectIns( lm_command& _cmd,const SKILLDATA* _skill );

	//! FrameMove
	BOOL                 frame_moveMagicEffect( float _eleTime );

	//! 

};


//! 人物的状态特效使用的数据结构。
class lm_monsterStateIns
{
public:
	
	//! 特效的id.
	int        m_iSpeffId;
	//! 对应的怪物handleID,用于删除此状态特效
	int        m_iHandleId;

	const osVec3D*   m_vec3PtrPos;
	osVec3D          m_vec3LastPos;

	lm_monsterStateIns()
	{
		m_iSpeffId = -1;
		m_vec3PtrPos = NULL;
	}
	void    reset( void )
	{
		m_iSpeffId = -1;
		m_vec3PtrPos = NULL;
	}
	void    frame_move( void );

};

/** \brief
 *  Demo的魔法管理类
 *
 *  魔法管理器初始化的时候，要调入全局的图标编辑器编辑出的技能文件。
 *  使用这个技能文件，来显示所有的技能。
 *  
 *  魔法的插入要处理的命令：
 *  播放魔法，对于有持续时间的魔法，则由管理器自己管理消除。
 *  
 */
class lm_meManager : public lm_gameEntity
{
private:
	//! 从文件内调入的，整个场景内用到的技能相关数据。
	VEC_skillData          m_vecSkillData;

	//! 特效播放对应的实例数据
	CSpaceMgr<lm_magicEffectIns>    m_vecMeIns;

	//! 测试播放特效的id
	DWORD                  m_dwTestEffId;

	//! 怪物的状态队列。
	CSpaceMgr<lm_monsterStateIns>  m_vecStateEffect;

private:
	//! 根据特效的命令，来播放一个特效
	BOOL                  play_magicEffect( lm_command& _cmd );

	/** \brief
	 *  在场景内一个位置上播放一个特效,
	 *
	 *  仅用于测试用，所以下次播放特效时，删除当前在播放的特效
	 */
	BOOL                  play_effectInSce( lm_command& _cmd );

	//! 在场景内播放一个状态特效
	BOOL                  play_stateMagic( lm_command& _cmd );

public:
	lm_meManager();
	~lm_meManager();

	//! 必须初始化整个场景用到的技能相关的信息
	BOOL                  init_skillList( const char* _fname );

	//! 从技能名得至此技能的id
	int                   get_skillIdFromName( const char* _name );

	//! 整理特效管理器的帧间处理
	void                  frame_moveMeMgr( void );

	//! 执行指令的接口
	virtual BOOL          process_command( lm_command& _command );

};
