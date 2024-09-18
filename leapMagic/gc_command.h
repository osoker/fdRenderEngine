///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_command.h
 *
 *  Desc:     ħ��demo�ͻ��˵���Ϣ��������������ڵ�ÿһ��Ԫ�ض������Լ��ܹ���������������Ϣ��
 * 
 *  His:      River created @ 2006-4-13
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "stdafx.h"
# include <queue>


//
enum lm_commandType
{
	//@{ ������ص������
	//! ��������.
	LM_SCENESTART = 1,

	LM_SCENELOAD,
	//! �����ͷ�
	LM_SCENERELEASE,

	//! ������������
	LM_SCENE_LBUTTONDOWN,

	//! ����������Ҽ�����
	LM_SCENE_RBUTTONDOWN,

	LM_SCENE_CONTROLDOWN,

	//! ���������ƶ�
	LM_SPACEMOVE,

	//! �����ڲ���һ������ħ��,�����յ������Ϣ���ٰ������Ϣ���д������͸����ܹ��������š�
	LM_SKILLPLAY,

	//! �����ڲ��벥��һ����Ч
	LM_EFFECTPLAY,

	//! �����ڲ���һ�������״̬ħ���������Ϣת�������ܹ���������
	LM_MAGICSTATE,

	//! ��������һ����������
	LM_SCENEMUSIC,

	//! ����ʹ�����ֵ�װ���л�
	LM_NAMEEQUIPCHANGE,

	//! ����������ƶ�
	LM_SCENE_MOUSEMOVE,

	LM_SCENEEND,
	//@}


	//@{ ��������������
	//! �������ת��Pitch
	LM_CAMERASTART,

	LM_CAMERAPITCH,
	LM_CAMERAROT,
	LM_MOVECLOSE,
	LM_MOVECLOSEF, // �ƶ����Եľ���
	//! ����Ľ����λ��һ���ƶ�
	LM_CAMMOVE,

	//! ��ǰ�ƶ�
	LM_MOVEFORWORD,
	//! �����ƶ�
	LM_MOVERIGHT,
	//! ���������ƶ�
	LM_MOVEUP,

	//! �洢���
	LM_SAVECAMERA,

	//! �������
	LM_LOADCAMERA,

	LM_CAMERAEND,
	//@} 


	//@{ ����������ص������
	//
	LM_HEROSTART,

	LM_HEROINSERT,
	LM_HEROWALK,	
	LM_HEROATTACK,
	LM_HERONEWACTION,
	LM_HEROEQUIPCHANGE,
	LM_HEROHIDE,
	LM_HEROSAY,

	LM_HERORIDE,
	LM_HEROUNRIDE,

	LM_HEROEND,
	//@}



	//@{ �����ڹ�����ص������
	LM_MONSTERSTART,

	LM_MONSTERINSERT,
	LM_MONSTERWALK,
	LM_MONSTERATTACK,
	LM_MONSTERDEAD,
	LM_MONSTEREQUIP,

	// ����Ķ����仯:
	LM_MONSTERACT,

	// ���ù���Ĳ���ʱ��,һ��������������ʱ��
	LM_MONSTERRUNACTTIME,

	// ���ù����������������
	LM_MONSTERRUNSPEED,

	// ���ù����ͷ����ʾ˵������
	LM_MONSTERSAY,
	LM_MONSTERHIDE,

	// ������������
	LM_MONSTERRIDE,
	LM_MONSTERUNRIDE,

	// ɾ������һ�����������
	LM_MONSTERDEL,

	LM_MONSTEREND,
	//@} 



	//@{
	//! �����������仯����Ϣ
	LM_WEATHERCHANGE,
	//@}

	//@{ ��Ч���������յ��������
	//
	LM_MEMGRSTART,

	//! ����һ������
	LM_MESKILLPLAY,

	//! ������һ����Ч
	LM_MEEFFECTPLAY,

	//! ����һ��״̬��Ч
	LM_MESTATEEFFECT,

	LM_MEMGREND,


};


/** \brief
 *  ħ��Demo�Ļ�������
 *
 *  �˴�ʹ��command,��Ҫ������û�������������������Ϣ���������˹������̵߳���Ϣ��
 *  ��ͳ��Ϊָ����е�entity�����Լ���ִ�е�ָ���
 */
struct lm_command
{
	//! ָ��id.
	lm_commandType     m_eCommandId;

	//! ָ�����ݵ�size.
	DWORD              m_dwCmdContentSize;
	//! �������ݣ���ִ��ָ���entity������
	const void*        m_ptrCommandContent;
	//! ָ���õ��Ŀ�ʼhandle
	int                m_iSHandle;
	//! ָ���Ŀ��handle
	int                m_iEHandle;

public:
	lm_command()
	{
		m_ptrCommandContent = NULL;
		m_iSHandle = -1;
		m_iEHandle = -1;
	}
};

typedef std::vector<lm_command>  VEC_lmCommand;



//! �����ڹֻ���npc�Ĳ������ݽṹ
struct lm_creatureInsert
{
	//! �ڳ����ڵĲ���λ��
	int   m_iXPos;
	int   m_iZPos;

	//! ��ת�Ƕ�
	float m_fRotAgl;

	//! �����Ŀ¼��,��������������õ���������ĳ�ʼ�����ݽṹ
	char  m_szChrDir[32];

	//! �����볡���Ĺ�������֣����Ϊ�գ���û������
	char  m_szCreatureName[16];

	//! �Ƿ����������͹���
	BOOL  m_bFlagType;

	//! �����������Ĺ����Ϊ���ĵ���������
	BYTE  m_btFlagEquip[4];

	//! ���������
	float m_fScale;

public:
	lm_creatureInsert() {  m_bFlagType = FALSE;m_fRotAgl = 0.0f;m_fScale = 1.0f; } 

};


struct lm_spaceMoveCmd
{
	//! ���³����ڵ�λ�á�
	int   m_iXPos;
	int   m_iZPos;

	//! �³����ĵ�ͼ��
	char  m_szMapName[32];

};

//! ����������볡��Ӧ���������ݽṹ
struct lm_heroInsert : public lm_creatureInsert
{
	//! ���볡��������ĳ�ʼ��װ��
	os_equipment   m_sInitEquip;
};


// 
//! �����仯������,�ڿ���̨�ڸı���������Ķ���
struct lm_creatureActChange
{
	//! �¶��������֣�ֻ��ʹ�����������Ŷ���
	char      m_szNewActName[32];
	//! �����¶����ķ�ʽ 0: �滻ԭ���Ķ����� 1: �����¶���һ��   2:����������Ķ���
	int       m_iPlayActType;

	//! �л��¶������ٶ�,���ڸı䳡������Ķ����ٶ�
	float     m_fPlayActScale;

	//! �����¶���ʱ���������ת����
	float     m_fRotAgl;
	BOOL      m_bUseAgl;

public:
	lm_creatureActChange()
	{
		m_szNewActName[0] = NULL;
		m_bUseAgl = FALSE;
		m_fPlayActScale = 1.0f;
	}
};

//! ���ܵ�����
struct lm_cmdSkillName
{
	char  m_szSkillName[64];
	//! ������ر�ĸ߶�
	float m_fSkillHeight;
};

//! �����ļ�����ص���Ϣ,�ʹ��64���ַ�
struct lm_cmdFileName
{
	char    m_szFileName[64];
	
	const char* operator=( const char* _fname );
};


//! Ҫ���ŵ���Ч������
struct lm_skillPlay
{
	//! ���ܵ��������������ڵõ�Ҫ���ŵ���ؼ���
	int        m_iSkillIndex;

	//
	/** \briefr
	 *  ���ܲ��ŵĿ�ʼλ�úͽ���(���ǹ���)λ�� ���ݲ�ͬ����Ч�������������в�ͬ����;��
	 *
	 *  ��Ϊ����λ�úͿ�ʼλ�û�����ʱ��仯������ʹ��ָ�����õ����µ�λ����Ϣ��
	 */
	const osVec3D*    m_ptrStartPos;
	const osVec3D*    m_ptrAttackPos;

	//! ���ܲ��ŵ�λ�õ����߶�ֵ��
	float             m_fHeiAdj;

	//! 
	lm_skillPlay& operator=(lm_skillPlay& _p )
	{
		m_iSkillIndex = _p.m_iSkillIndex;
		m_ptrStartPos = _p.m_ptrStartPos;
		m_ptrAttackPos = _p.m_ptrAttackPos;
		m_fHeiAdj = _p.m_fHeiAdj;

		return *this;
	}
};

//! ��ĳ��λ�ò���һ����Ч
struct lm_effectPlay
{
	//! Ҫ���ŵ���Ч��
	char    m_szEffName[64];

	//! Ҫ���ŵ���Чλ��
	osVec3D m_vec3Pos;
};

//! �洢������ļ����Ǵ��ļ��е������
struct lm_cameraSaveLoad
{
	//! Ҫ�洢���ǵ��������ļ���
	char    m_szCameraFile[64];
	//! ���������ʱ��
	float   m_fCamAniTime;
};

//! ������ǹ���˵���Ĺ��ܣ�����ͷ������
struct lm_talkContent
{
	//! ˵��������
	char    m_szTalkContent[256];

	//! ���ݴ�������ͷ����ʱ��
	float   m_fTcTime;

	lm_talkContent(){ m_szTalkContent[0] = NULL;m_fTcTime = 2.0f; }

	lm_talkContent& operator=( const lm_talkContent& _tc );

};


//! ��Ϸ�ڿ���ִ��ָ���ʵ��
class  lm_gameEntity
{
protected:
	//! ��ǰentity���ڵ�λ��
	osVec3D            m_vec3EntPos;

	//! ��ǰentity����ת�Ƕ�
	float              m_fRotAgl;

public:
	//! ���������ڵ�entity��Ŀ
	static int         m_iEntityNum;

public:
	lm_gameEntity();
	~lm_gameEntity(); 

	//! ִ��ָ��Ľӿ�
	virtual BOOL           process_command( lm_command& _command ) = 0;

	//! �õ�ent��λ�á�
	virtual void           get_entityPos( osVec3D& _pos ) { _pos = m_vec3EntPos; } 

	//! �õ�entity��λ�����ã������ط�ʹ��ָ����Եõ����µ�λ����Ϣ��
	virtual const osVec3D* get_entityPosPtr( void ) { return &m_vec3EntPos; }

	virtual const float*   get_entityRotAglPtr( void ) { return &m_fRotAgl; }

};



//! Demo��ʵ�ֵĿ����Զ�ִ�е�����
struct lm_autoCmd
{
	//! Ҫִ�е����������.
	char         m_szCmdStr[64];

	//! �������������ִ����Ҫ��ƫ��ʱ��.
	float        m_fOffsetExecTime;

};
typedef std::queue<lm_autoCmd>   QUE_autoCmd;



/////////////////////////////////////////////////////////////////////////////////////////////
//
// �����ڵ����ڿ�������ı�������
//
/////////////////////////////////////////////////////////////////////////////////////////////
//! �����ڵĶ��������ر���
struct lm_cmdVariableObj
{
	//! ���������
	char     m_szObjName[16];
	//! �������ֵ�hash_value.
	DWORD    m_dwObjNameHash;

	//! �����ڲ��ľ��
	int      m_iObjHandle;
	
public:
	lm_cmdVariableObj();
	void     reset( void )
	{
		m_szObjName[0] = NULL;
		m_dwObjNameHash = 0;
		m_iObjHandle = 0;
	}
};

typedef std::vector<lm_cmdVariableObj>  VEC_variable;
//! �����ڿ��õı����Ĺ�����
class lmc_cmdVarObjMgr
{
private:
	//! �����ڱ������б�
	VEC_variable   m_vecVariable;

	//! 

public:
	lmc_cmdVarObjMgr();
	~lmc_cmdVarObjMgr();

	//! ������������һ������
	void    add_sceneVariable( const char* _vname,int _vvalue );

	//! �ڳ�����ɾ��һ������
	void    delete_sceneVariable( const char* _vname );

	//! �ڳ�����ɾ��һ������
	void    delete_variable( int _handle );

	//! �������֣��õ�һ������
	int     get_sceneVariable( const char* _vname );

	//! �����Ƿ�����ڳ�����
	bool    is_variableInScene( const char* _vname );


};



//! ȫ�ֵ�������еĳ�ʼ����С
# define INIT_APP_COMMAND   4096
# define INIT_APP_CMDBUF    1024*1024*4

/** \brief
 *  ȫ�ֵ�������кͻ�����
 *
 *  ÿһ����������ݶ��Ḵ�Ƶ�������������ڲ��ڴ�һ��
 */
class lmc_commandMgr : public ost_Singleton<lmc_commandMgr> 
{
private:
	//! ȫ�ֵ��������
	VEC_lmCommand   m_vecCmdQueue;
	int             m_iCmdPtr;

	//! �����Ӧ������
	BYTE*           m_ptrCmdContentBuf;
	int             m_ptrCmdCntPtr;

	//@{ 
	//! �������,���ڽ������������
	VEC_string      m_vecCmdList;	
	//@}


	//@{
	//! �Զ���ִ��������б�
	QUE_autoCmd     m_sAutoCmdQueue;
	//! �ۻ���һ��������ȥ��ʱ��,���Զ��������ÿһ�������������һ������ִ��
	float           m_fAccuTime;
	//! ����Զ�������в�Ϊ��,��˱�����ʼ��ʱ.
	float           m_fAutoCmdExeTime;
	//! �Ƿ��������Զ�����ִ��״̬
	BOOL            m_bAutoCmdExeStatus;


	//! ���Զ��������Ϊ�յ�ʱ��,������ص�����.
	void            reset_autoCmdQueue( void );
	//@}

	//! ���Ʊ�������
	lmc_cmdVarObjMgr m_sVariableMgr;

private:
	lmc_commandMgr();
	~lmc_commandMgr();

	//! ���������ִ�.
	const char*     get_cmdStr( const char* _cmd );
	const char*     get_cmdContent( const char* _cmd );

	//! �����ִ��ĵ�n�����ݣ�ֱ�������ո�
	const char*     get_keyval( const char* _str,int _idx );
	//! ���ҵ�n���������ݿ�Ŀ�ʼλ��
	int             get_keyStartPos( const char* _str,int _idx );

	//! ���������ƶ�����
	bool            interpret_spaceMoveCmd( lm_spaceMoveCmd& _cmd,const char* _cmdstr );
	//! ����������������
	bool            interpret_monsterInsert( 
		               lm_creatureInsert& _cmd,const char* _cmdstr,bool _flag = false );
	//! ��������Ķ����仯
	bool            interpret_actChange( lm_creatureActChange& _cmd,const char* _cmdstr );
	//! ���������ܲ��Ե�����
	bool            interpret_skillPlay( int& _idx,const char* _cmdstr );
	//! ��������Ч���Ե�����
	bool            interpret_effPlay( lm_effectPlay& _cmd,const char* _cmdstr );
	//! ������ȫ�ֹ�ĸı�����
	bool            interpret_ambient( const char* _cmdstr );
	//! ������ȫ�ַ���������.
	bool            interpret_dirlight( const char* _cmdstr );
	//! ������ȫ�ֵ������ɫ������
	bool            interpret_fogColor( const char* _cmdstr );
	//! ������ȫ����Ŀ�ʼ�ͽ�������
	bool            interpret_fogDis( const char* _cmdstr );
	//! ����������װ����ص�����
	int             interpret_weapon( const char* _cmdstr );
	//! ���������岿λװ����ص����ݡ�
	bool            interpret_equip( const char* _cmdstr,os_equipment& _equip );
	//! ����������������ֵ�������������
	bool            interpret_int2( const char* _cmdstr,int& _v1,int& _v2 );
	//! Ҫ�����ִ������һ�������͵���������
	bool            interpret_str1_float1( const char* _cmdstr,lm_effectPlay& _cmd );
	//! �������ִ�,���һ����ֵ
	bool            interpret_str1_float( const char* _cmdstr,lm_cameraSaveLoad& _cmd );



	//! �ִ��Ŀ�ͷ�Ƿ��Ǳ���,��������Ӧ������
	bool            interpret_varCmd( const char* _str,int& _handle,int& _target );


	//! �����ͼ����������������
	void            load_map( lm_spaceMoveCmd& _cmd );


public:
	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static lmc_commandMgr*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void              DInstance( void );


	//! ����һ������
	void      push_command( lm_command& _cmd );
	
	//! ����ִ��һ������: 
	void      do_commandImm( lm_command& _cmd );

	//! ȫ���������
	void      clear_command( void )    { m_iCmdPtr = 0;m_ptrCmdCntPtr = 0; } 
	//! �õ�������������ĸ���
	int       get_cmdNum( void )       { return m_iCmdPtr; }
	//! �õ���n�������ָ��
	lm_command* get_cmdIdx( int _idx );

	//! ���ִ�����Ϊ�ڲ�������,�����뵽���������
	bool      interpret_strToCmd( const char* _str );

	//@{
	//!  ��������ִ�������һЩ����
	void      frame_moveAutoCmd( float _eleTime );
	//!  ������Ҫ�Զ�ִ�е�cmd
	bool      push_autoCmd( lm_autoCmd& _autocmd );
	//!  �����ִ��һ�������ļ��б�
	bool      load_execCmdFile( const char* _filename );
	//@} 
	

};


//! ���ڵ������������ӡ�����ݽṹ
struct lm_footMarkInfo
{
	char     m_szFootMarkTex[32];
	//! ��ӡ����ת����
	float    m_fFootMarkRotAdj;
	
	float    m_fMarkVanishTime;
	float    m_fMarkSize;

	float    m_fMark1Percent;
	float    m_fMark2Percent;

	//! �����λ�������ĵ㣬������ӡӦ��ƫ�����ĵ㣬������ƶ�һ��
	float    m_fMarkOffset;
};
struct lm_footWaterInfo
{

	float	 m_playTime;	//!< ����ˮ����Ч���ŵ�ʱ����
	char     m_szFootWaterIdleSpe[32];
	char     m_szFootWaterWalkSpe[32];
	float	 m_fFootWaterRotAdj;

	osVec3D	 m_FootWaterOffset;
	lm_footWaterInfo()
	{
		m_playTime = 1.0f;
		m_szFootWaterIdleSpe[0] = 0;
		m_szFootWaterWalkSpe[0] = 0;
		m_fFootWaterRotAdj = 0;
	}
};

//@{
//! ȫ���õ��ı���.

//! 
extern BOOL       g_bDrawCreatureBBox;

//@}



//! ����Demo�ڳ��õ�һЩȫ�ֱ���
struct lm_gvar
{
	//! �����������ײλ��
	static int         g_iHeroPosX;
	static int         g_iHeroPosZ;

	//! �����������ϸλ����Ϣ.
	static float       g_fHeroPosX;
	static float       g_fHeroPosZ;

	//! �ϴβ�����ȷ�ļ���id.
	static s_string    g_szLastSkill;

	//! ��ǰ�Ƿ����
	static BOOL        g_bCatchScreen;
	
	//! ȫ�ֵ�handle,ÿ������һ��������ô�handle
	static int         g_iCreatureHandle;

	//! �Ƿ���ʾ��������֡�
	static BOOL        g_bDispMonsterName;

	//! �Ƿ������������
	static BOOL        g_bCamFollowHero;

	//! ������ƶ��ٶ�
	static float       g_fCamMoveSpeed;

	//! �Ƿ�򿪹�����Զ���ħ������
	static bool        g_bRandSkillPlay;

	//! ���ڿ�������ʩ���������ٶ�
	static float       g_fSkillActSpeed;

	//! �Ƿ���ʾ������Ϣ
	static BOOL        g_bRenderDebugInfo;

	//! �Ƿ��ڵ����ͼ�󣬽���������������ͼ�����ڼ���ͼ���ݵ������ԡ�
	static BOOL        g_bSpaceMoveWholeMap;

	//! �Ƿ���Ҫ������ͼ����ײ��Ϣ
	static BOOL        g_bAdjMapCollInfo;

	//! �Ƿ�����ƶ�ʱ,������λ����ʾ��Χ���ӵ���ײ��Ϣ
	static BOOL        g_bDisplayColInfo;

	//! �������������й������Ұ��Ϣ
	static BOOL        g_bDisplayAllMonInfo;

	//! ������ʱ, �Ƿ���ctrl��.
	static BOOL        g_bCtrlDown;

	//! �Ƿ���ʾ��������
	static BOOL        g_bDisTrigger;
	//! �Ƿ���ʾ������Ұ
	static BOOL        g_bDisVision;
	//! �Ƿ���ʾ���ܾ���.
	static BOOL        g_bDisRun;
	//! �Ƿ���ʾ���о���.
	static BOOL        g_bDisCall;


	//! �Ƿ�鿴�����������ʱ��.
	static BOOL        g_bNormalMoveTime;

	//! ��������,�Ƿ�ѡ�е�ǰ����Ĺ���
	static BOOL        g_bSelLastInsertMonst;

	//! ��������ʱ��ֹͣ
	static BOOL        g_bSuspendTime;

	//! �Ƿ���ʾ��ά������
	static BOOL        g_bDisplayCharInRect;

	//! ȫ�ֵ���Чid,���ڲ��԰�Χ��.
	static int         g_iEffectId;


public:
	lm_gvar();
	~lm_gvar();

};

extern s_string g_szHeroDir[6];