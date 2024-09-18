///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_magicEffect.h
 *
 *  Desc:     ħ��demo�ͻ����õ���ħ����Ч��ع�����
 * 
 *  His:      River created @ 2006-5-23
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
# include "StdAfx.h"
# include "gc_command.h"


//! ������Ч����ʱ�䷽ʽ
enum SKILL_EFFECT_TIMESTYLE
{
	SET_ORDER_SECTION3,   //��ʱ��˳�򲥷�,3�ζ��� 
	SET_SYNCHRO,          //������Чͬ������,һ������������3����Ч
};

//! ����һ�������е�һ����Ч(spe)��ν��в���
enum SKILL_EFFECT_PLAYSTYLE
{
	SEP_FOLLOW_LAUNCHER_CENTER_FOREVER,				//! ���淢����һֱ���ţ��Է����ߵ���������Ϊԭ�� ___

	SEP_FOLLOW_TARGET,                      //! ������ҵĹ����㲥��

	SEP_POSITION_ONETIMES,							//! ��ָ����λ���ϲ���һ����Ч               ___��ʯ
	
	SEP_NONE,
};

//! һ�������������е���Ч��Ŀ
# define MAX_EFFECTPERSKILL    3

struct SKILLDATA
{
	int id;
	char name[40];
	bool bAction;
	char actionName[40];
	char actionName2[40]; //ʩ������
	ose_amPlayType actionType;
	SKILL_EFFECT_TIMESTYLE 	effectTimeStyle;
	char effectFilename[MAX_EFFECTPERSKILL][40];
	SKILL_EFFECT_PLAYSTYLE effectType[MAX_EFFECTPERSKILL];
	float motionSpeed;
	float motionAdjRadian;

	//char stateEffectFilename[40];         //! ʹ�ô˼��ܺ���Ӱ�춼�����е�״̬��Ч��(ѭ��)
	float effectStartTime;                     //! ��Ч��ʼ����ʱ��
	char	reserve[27];				// �����ֶ�
	bool	bIsPlayEffectBeHit;			//�Ƿ��ڱ������㲥����Ч
	float followEffectStartTime;   // ������Ч��ʼ����ʱ��
	float followEffectEndTime;     // ������Ч��ֹ����ʱ��
	char followEffectFilename[40];        //! ʹ�ô˼���ʱ�ڷ�������ߵİ�����Ч(ѭ��)

	int iconid;
	char desc[256];

	// ��װ�˺���������ȫ�ֵ�ʩ�������ٶȸı�
	float  get_effStartTime( void ) const ;
	//
	float  get_followEffEndTime( void ) const;


};
typedef std::vector<SKILLDATA>   VEC_skillData;


//! ���湥���㲥�ŵ���Чʹ��ͳһ�Ĳ���ʱ��
# define   FOLLOWATTACK_EFFECTTIME   10.0f


//
// ������Ч�Ĳ�����
// ���¿�ݼ���ʱ�򣬷�������Ч�����������
// ��Ч�������յ�����󣬳�ʼ����Ч��Ȼ�󷢸���������ı��Ӧ���������
// ����Ч��ʼFrameMove,�ڲ���ʱ������Ӧ��ʱ��󲥷���Ч��
// ���������Ч���ź�ɾ����Ӧ����Чins.

/** \brief
 *  ħ�����ŵ��ࡣ
 *
 *  ��һ�����ܵĲ��Ž��й���ʹ�������ڲ�����Ч���Žӿڣ�
 *  ��ÿһ����Ч�Ĵ��������ź�ɾ�����й���
 */
class lm_magicEffectIns : public os_createVal
{
private:
	//! �������Ч��ID.
	const SKILLDATA*     m_ptrSkillData;

	//! ��Ч������ص�����
	lm_skillPlay         m_sSkillPlay;

	//! ��ǰ��Чƫ��ʱ��,���ʱ��������ڶ�����ʼ���ź��ʱ��ƫ��ֵ
	float                m_fEleTime;

	//@{ ������Ч�����������
	//! �Ƿ��Ѿ����ﵽ��Ч���ſ�ʼ��
	BOOL                 m_bArrivedEffStartPt;
	//! �м�Ĺ���������Ч�Ƿ񵽴﹥��Ŀ�꣬����������Ļ���
	BOOL                 m_bArrivedAttackTarget;
	float                m_fDisFromStart;
	float                m_fMoveEffAgl;
	//! �Ƿ��������̽���
	BOOL                 m_bAmassPowerEnd;

	//! ÿһ����Чʵ����Ӧ��id
	DWORD                m_dwEffInsId[MAX_EFFECTPERSKILL];
	BOOL                 m_bSkillEffPlayed[MAX_EFFECTPERSKILL];
	//@}


	//@{
	//! ������Ч�Ĳ����������
	BOOL                 m_bArrivedFolEffStartPt;
	DWORD                m_dwFolEffId;
	BOOL                 m_bFollowEffEnd;
	//@}

	//@{
	//! ���湥���������Ч������
	BOOL                 m_bPlayingFollowAttack;
	float                m_fFollowAttackTime;
	osVec3D              m_vec3LastFrameAttackPos;
	const osVec3D*       m_ptrAttackPos;
	//@}

	//! ���ﲥ��������Ч��ʱ��
	float                m_fFollowEffectEndTime;


	//! ��Ч�ĵ����߶�
	float                m_fSkillAdjHeight;

private:

	//! �ж�һ��������Ч�Ƿ�ȫ���������,�ں���frameMove_effectInsBePlaying������
	BOOL                 is_skillEffectPlaying( void );

	/** \brief
	 *  ��ǰ��EffectIns�Ƿ���ʹ����,���ȫ�����Ų����꣬�򷵻�false.
	 *
	 *  ������Ҫ����λ�õ���Ч���ڴ˺���������λ�á�
	 */
	BOOL                 frameMove_effectInsBePlaying( float _eletime );

	//! �԰�����Ч�Ĵ���
	BOOL                 frame_moveFollowEff( void );

	//! ��������Ч���д���
	void                 framemove_amassPower( int _idx );

	//! �Թ�����Ч���д�����������У�����ҵ�����Ļ���
	void                 framemove_moveEffect( int _idx );

	//! �Ը��湥���㲥�ŵ���Ч����FrameMove
	void                 framemove_followAttackEffect( int _idx );

	float                get_amassPowerTime( void );

public:
	lm_magicEffectIns(void);
	~lm_magicEffectIns(void);

	//! ����effect������
	BOOL                 init_magicEffectIns( lm_command& _cmd,const SKILLDATA* _skill );

	//! FrameMove
	BOOL                 frame_moveMagicEffect( float _eleTime );

	//! 

};


//! �����״̬��Чʹ�õ����ݽṹ��
class lm_monsterStateIns
{
public:
	
	//! ��Ч��id.
	int        m_iSpeffId;
	//! ��Ӧ�Ĺ���handleID,����ɾ����״̬��Ч
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
 *  Demo��ħ��������
 *
 *  ħ����������ʼ����ʱ��Ҫ����ȫ�ֵ�ͼ��༭���༭���ļ����ļ���
 *  ʹ����������ļ�������ʾ���еļ��ܡ�
 *  
 *  ħ���Ĳ���Ҫ��������
 *  ����ħ���������г���ʱ���ħ�������ɹ������Լ�����������
 *  
 */
class lm_meManager : public lm_gameEntity
{
private:
	//! ���ļ��ڵ���ģ������������õ��ļ���������ݡ�
	VEC_skillData          m_vecSkillData;

	//! ��Ч���Ŷ�Ӧ��ʵ������
	CSpaceMgr<lm_magicEffectIns>    m_vecMeIns;

	//! ���Բ�����Ч��id
	DWORD                  m_dwTestEffId;

	//! �����״̬���С�
	CSpaceMgr<lm_monsterStateIns>  m_vecStateEffect;

private:
	//! ������Ч�����������һ����Ч
	BOOL                  play_magicEffect( lm_command& _cmd );

	/** \brief
	 *  �ڳ�����һ��λ���ϲ���һ����Ч,
	 *
	 *  �����ڲ����ã������´β�����Чʱ��ɾ����ǰ�ڲ��ŵ���Ч
	 */
	BOOL                  play_effectInSce( lm_command& _cmd );

	//! �ڳ����ڲ���һ��״̬��Ч
	BOOL                  play_stateMagic( lm_command& _cmd );

public:
	lm_meManager();
	~lm_meManager();

	//! �����ʼ�����������õ��ļ�����ص���Ϣ
	BOOL                  init_skillList( const char* _fname );

	//! �Ӽ����������˼��ܵ�id
	int                   get_skillIdFromName( const char* _name );

	//! ������Ч��������֡�䴦��
	void                  frame_moveMeMgr( void );

	//! ִ��ָ��Ľӿ�
	virtual BOOL          process_command( lm_command& _command );

};
