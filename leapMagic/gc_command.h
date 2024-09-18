///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_command.h
 *
 *  Desc:     魔法demo客户端的消息处理基础，程序内的每一个元素都处理自己能够处理的命令或是消息。
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
	//@{ 场景相关的命令处理
	//! 场景调入.
	LM_SCENESTART = 1,

	LM_SCENELOAD,
	//! 场景释放
	LM_SCENERELEASE,

	//! 场景内鼠标点下
	LM_SCENE_LBUTTONDOWN,

	//! 场景内鼠标右键点下
	LM_SCENE_RBUTTONDOWN,

	LM_SCENE_CONTROLDOWN,

	//! 场景自由移动
	LM_SPACEMOVE,

	//! 场景内播放一个技能魔法,场景收到这个消息后，再把这个消息进行处理，发送给技能管理器播放。
	LM_SKILLPLAY,

	//! 场景内插入播放一个特效
	LM_EFFECTPLAY,

	//! 场景内播放一个人物的状态魔法，这个消息转发给技能管理器处理
	LM_MAGICSTATE,

	//! 场景播放一个背景音乐
	LM_SCENEMUSIC,

	//! 处理使用名字的装备切换
	LM_NAMEEQUIPCHANGE,

	//! 场景内鼠标移动
	LM_SCENE_MOUSEMOVE,

	LM_SCENEEND,
	//@}


	//@{ 相机相样的命令处理
	//! 相机的旋转和Pitch
	LM_CAMERASTART,

	LM_CAMERAPITCH,
	LM_CAMERAROT,
	LM_MOVECLOSE,
	LM_MOVECLOSEF, // 移动绝对的矩离
	//! 相机的焦点和位置一起移动
	LM_CAMMOVE,

	//! 往前移动
	LM_MOVEFORWORD,
	//! 往右移动
	LM_MOVERIGHT,
	//! 焦点往上移动
	LM_MOVEUP,

	//! 存储相机
	LM_SAVECAMERA,

	//! 调入相机
	LM_LOADCAMERA,

	LM_CAMERAEND,
	//@} 


	//@{ 主角人物相关的命令处理
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



	//@{ 场景内怪物相关的命令处理
	LM_MONSTERSTART,

	LM_MONSTERINSERT,
	LM_MONSTERWALK,
	LM_MONSTERATTACK,
	LM_MONSTERDEAD,
	LM_MONSTEREQUIP,

	// 怪物的动作变化:
	LM_MONSTERACT,

	// 设置怪物的步伐时间,一个完整步动作的时间
	LM_MONSTERRUNACTTIME,

	// 设置怪物的完整步幅长度
	LM_MONSTERRUNSPEED,

	// 设置怪物的头顶显示说话功能
	LM_MONSTERSAY,
	LM_MONSTERHIDE,

	// 怪物的骑乘命令
	LM_MONSTERRIDE,
	LM_MONSTERUNRIDE,

	// 删除场景一个怪物的命令
	LM_MONSTERDEL,

	LM_MONSTEREND,
	//@} 



	//@{
	//! 场景内天气变化的信息
	LM_WEATHERCHANGE,
	//@}

	//@{ 特效管理器接收到的命令处理
	//
	LM_MEMGRSTART,

	//! 播放一个技能
	LM_MESKILLPLAY,

	//! 管理播放一个特效
	LM_MEEFFECTPLAY,

	//! 播放一个状态特效
	LM_MESTATEEFFECT,

	LM_MEMGREND,


};


/** \brief
 *  魔法Demo的基础命令
 *
 *  此处使用command,主要是想把用户的输入和来自网络的消息或是来自人工智能线程的消息，
 *  都统称为指令，所有的entity都对自己可执行的指令负责。
 */
struct lm_command
{
	//! 指令id.
	lm_commandType     m_eCommandId;

	//! 指令内容的size.
	DWORD              m_dwCmdContentSize;
	//! 命令内容，由执行指令的entity来解释
	const void*        m_ptrCommandContent;
	//! 指令用到的开始handle
	int                m_iSHandle;
	//! 指令到的目标handle
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



//! 场景内怪或是npc的插入数据结构
struct lm_creatureInsert
{
	//! 在场景内的插入位置
	int   m_iXPos;
	int   m_iZPos;

	//! 旋转角度
	float m_fRotAgl;

	//! 人物的目录名,由人物管理器来得到创建人物的初始化数据结构
	char  m_szChrDir[32];

	//! 待插入场景的怪物的名字，如果为空，则没有名字
	char  m_szCreatureName[16];

	//! 是否是旗帜类型怪物
	BOOL  m_bFlagType;

	//! 如果是旗帜类的怪物，此为旗帜的外形数据
	BYTE  m_btFlagEquip[4];

	//! 怪物的缩放
	float m_fScale;

public:
	lm_creatureInsert() {  m_bFlagType = FALSE;m_fRotAgl = 0.0f;m_fScale = 1.0f; } 

};


struct lm_spaceMoveCmd
{
	//! 在新场景内的位置。
	int   m_iXPos;
	int   m_iZPos;

	//! 新场景的地图名
	char  m_szMapName[32];

};

//! 主角人物插入场景应该填充的数据结构
struct lm_heroInsert : public lm_creatureInsert
{
	//! 插入场景内人物的初始化装备
	os_equipment   m_sInitEquip;
};


// 
//! 动作变化的命令,在控制台内改变主角人物的动作
struct lm_creatureActChange
{
	//! 新动作的名字，只能使用名字来播放动作
	char      m_szNewActName[32];
	//! 插入新动作的方式 0: 替换原来的动作。 1: 播放新动作一次   2:播放死亡类的动作
	int       m_iPlayActType;

	//! 切换新动作的速度,用于改变场景人物的动作速度
	float     m_fPlayActScale;

	//! 播放新动作时，人物的旋转方向
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

//! 技能的名字
struct lm_cmdSkillName
{
	char  m_szSkillName[64];
	//! 技能离地表的高度
	float m_fSkillHeight;
};

//! 所有文件名相关的消息,最长使用64个字符
struct lm_cmdFileName
{
	char    m_szFileName[64];
	
	const char* operator=( const char* _fname );
};


//! 要播放的特效的命令
struct lm_skillPlay
{
	//! 技能的索引，从索引内得到要播放的相关技能
	int        m_iSkillIndex;

	//
	/** \briefr
	 *  技能播放的开始位置和结束(或是攻击)位置 根据不同的特效，这两个数据有不同的用途。
	 *
	 *  因为攻击位置和开始位置会随着时间变化，所以使用指针来得到最新的位置信息。
	 */
	const osVec3D*    m_ptrStartPos;
	const osVec3D*    m_ptrAttackPos;

	//! 技能播放的位置调整高度值。
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

//! 在某个位置播放一个特效
struct lm_effectPlay
{
	//! 要播放的特效名
	char    m_szEffName[64];

	//! 要播放的特效位置
	osVec3D m_vec3Pos;
};

//! 存储相机到文件或是从文件中调入相机
struct lm_cameraSaveLoad
{
	//! 要存储或是调入的相机文件名
	char    m_szCameraFile[64];
	//! 相机动画的时间
	float   m_fCamAniTime;
};

//! 人物或是怪物说话的功能，顶在头顶的字
struct lm_talkContent
{
	//! 说话的内容
	char    m_szTalkContent[256];

	//! 内容存在人物头顶的时间
	float   m_fTcTime;

	lm_talkContent(){ m_szTalkContent[0] = NULL;m_fTcTime = 2.0f; }

	lm_talkContent& operator=( const lm_talkContent& _tc );

};


//! 游戏内可以执行指令的实体
class  lm_gameEntity
{
protected:
	//! 当前entity所在的位置
	osVec3D            m_vec3EntPos;

	//! 当前entity的旋转角度
	float              m_fRotAgl;

public:
	//! 整个场景内的entity数目
	static int         m_iEntityNum;

public:
	lm_gameEntity();
	~lm_gameEntity(); 

	//! 执行指令的接口
	virtual BOOL           process_command( lm_command& _command ) = 0;

	//! 得到ent的位置。
	virtual void           get_entityPos( osVec3D& _pos ) { _pos = m_vec3EntPos; } 

	//! 得到entity的位置引用，其它地方使用指针可以得到最新的位置信息。
	virtual const osVec3D* get_entityPosPtr( void ) { return &m_vec3EntPos; }

	virtual const float*   get_entityRotAglPtr( void ) { return &m_fRotAgl; }

};



//! Demo内实现的可以自动执行的命令
struct lm_autoCmd
{
	//! 要执行的命令的内容.
	char         m_szCmdStr[64];

	//! 相对于上条命令执行需要的偏移时间.
	float        m_fOffsetExecTime;

};
typedef std::queue<lm_autoCmd>   QUE_autoCmd;



/////////////////////////////////////////////////////////////////////////////////////////////
//
// 场景内的用于控制人物的变量功能
//
/////////////////////////////////////////////////////////////////////////////////////////////
//! 场景内的对象控制相关变量
struct lm_cmdVariableObj
{
	//! 对象的名字
	char     m_szObjName[16];
	//! 对象名字的hash_value.
	DWORD    m_dwObjNameHash;

	//! 对象内部的句柄
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
//! 场景内可用的变量的管理器
class lmc_cmdVarObjMgr
{
private:
	//! 场景内变量的列表
	VEC_variable   m_vecVariable;

	//! 

public:
	lmc_cmdVarObjMgr();
	~lmc_cmdVarObjMgr();

	//! 往场景内增加一个变量
	void    add_sceneVariable( const char* _vname,int _vvalue );

	//! 在场景内删除一个变量
	void    delete_sceneVariable( const char* _vname );

	//! 在场景内删除一个变量
	void    delete_variable( int _handle );

	//! 根据名字，得到一个变量
	int     get_sceneVariable( const char* _vname );

	//! 变量是否存在于场景内
	bool    is_variableInScene( const char* _vname );


};



//! 全局的命令队列的初始化大小
# define INIT_APP_COMMAND   4096
# define INIT_APP_CMDBUF    1024*1024*4

/** \brief
 *  全局的命令队列和缓冲区
 *
 *  每一个命令的内容都会复制到命令管理器的内部内存一份
 */
class lmc_commandMgr : public ost_Singleton<lmc_commandMgr> 
{
private:
	//! 全局的命令队列
	VEC_lmCommand   m_vecCmdQueue;
	int             m_iCmdPtr;

	//! 命令对应的肉容
	BYTE*           m_ptrCmdContentBuf;
	int             m_ptrCmdCntPtr;

	//@{ 
	//! 测试相关,用于解析命令的数据
	VEC_string      m_vecCmdList;	
	//@}


	//@{
	//! 自动化执行命令的列表
	QUE_autoCmd     m_sAutoCmdQueue;
	//! 累积上一个命令用去的时间,可自动命令可以每一个命令相对于上一个命令执行
	float           m_fAccuTime;
	//! 如果自动命令队列不为空,则此变量开始计时.
	float           m_fAutoCmdExeTime;
	//! 是否正处于自动命令执行状态
	BOOL            m_bAutoCmdExeStatus;


	//! 当自动命令队列为空的时候,重设相关的数据.
	void            reset_autoCmdQueue( void );
	//@}

	//! 控制变量功能
	lmc_cmdVarObjMgr m_sVariableMgr;

private:
	lmc_commandMgr();
	~lmc_commandMgr();

	//! 分析命令字串.
	const char*     get_cmdStr( const char* _cmd );
	const char*     get_cmdContent( const char* _cmd );

	//! 读入字串的第n个数据，直到碰到空格
	const char*     get_keyval( const char* _str,int _idx );
	//! 查找第n个命令数据块的开始位置
	int             get_keyStartPos( const char* _str,int _idx );

	//! 分析自由移动命令
	bool            interpret_spaceMoveCmd( lm_spaceMoveCmd& _cmd,const char* _cmdstr );
	//! 分析怪物插入的命令
	bool            interpret_monsterInsert( 
		               lm_creatureInsert& _cmd,const char* _cmdstr,bool _flag = false );
	//! 分析人物的动作变化
	bool            interpret_actChange( lm_creatureActChange& _cmd,const char* _cmdstr );
	//! 分析出技能测试的命令
	bool            interpret_skillPlay( int& _idx,const char* _cmdstr );
	//! 分析出特效测试的命令
	bool            interpret_effPlay( lm_effectPlay& _cmd,const char* _cmdstr );
	//! 分析出全局光的改变命令
	bool            interpret_ambient( const char* _cmdstr );
	//! 分析出全局方向光的命令.
	bool            interpret_dirlight( const char* _cmdstr );
	//! 分析出全局的雾的颜色的命令
	bool            interpret_fogColor( const char* _cmdstr );
	//! 分析出全局雾的开始和结束矩离
	bool            interpret_fogDis( const char* _cmdstr );
	//! 分析出武器装备相关的数据
	int             interpret_weapon( const char* _cmdstr );
	//! 分析出身体部位装备相关的数据。
	bool            interpret_equip( const char* _cmdstr,os_equipment& _equip );
	//! 分析出有两个整型值后跟的命令类型
	bool            interpret_int2( const char* _cmdstr,int& _v1,int& _v2 );
	//! 要析出字串，后跟一个浮点型的数据类型
	bool            interpret_str1_float1( const char* _cmdstr,lm_effectPlay& _cmd );
	//! 解析出字串,后跟一浮点值
	bool            interpret_str1_float( const char* _cmdstr,lm_cameraSaveLoad& _cmd );



	//! 字串的开头是否是变量,并处理相应的内容
	bool            interpret_varCmd( const char* _str,int& _handle,int& _target );


	//! 调入地图，并插入主角人物
	void            load_map( lm_spaceMoveCmd& _cmd );


public:
	/** 得到一个sg_timer的Instance指针.
	 */
	static lmc_commandMgr*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void              DInstance( void );


	//! 插入一个命令
	void      push_command( lm_command& _cmd );
	
	//! 立即执行一个命令: 
	void      do_commandImm( lm_command& _cmd );

	//! 全部清空命令
	void      clear_command( void )    { m_iCmdPtr = 0;m_ptrCmdCntPtr = 0; } 
	//! 得到管理器内命令的个数
	int       get_cmdNum( void )       { return m_iCmdPtr; }
	//! 得到第n个命令的指针
	lm_command* get_cmdIdx( int _idx );

	//! 把字串解析为内部的命令,并推入到命令队列内
	bool      interpret_strToCmd( const char* _str );

	//@{
	//!  用于批量执行命令的一些功能
	void      frame_moveAutoCmd( float _eleTime );
	//!  推入需要自动执行的cmd
	bool      push_autoCmd( lm_autoCmd& _autocmd );
	//!  调入和执行一个命令文件列表
	bool      load_execCmdFile( const char* _filename );
	//@} 
	

};


//! 用于调整主角人物脚印的数据结构
struct lm_footMarkInfo
{
	char     m_szFootMarkTex[32];
	//! 脚印的旋转调整
	float    m_fFootMarkRotAdj;
	
	float    m_fMarkVanishTime;
	float    m_fMarkSize;

	float    m_fMark1Percent;
	float    m_fMark2Percent;

	//! 人物的位置在中心点，两个脚印应该偏离中心点，往外侧移动一点
	float    m_fMarkOffset;
};
struct lm_footWaterInfo
{

	float	 m_playTime;	//!< 两次水面特效播放的时间间隔
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
//! 全局用到的变量.

//! 
extern BOOL       g_bDrawCreatureBBox;

//@}



//! 测试Demo内常用的一些全局变量
struct lm_gvar
{
	//! 主角人物的碰撞位置
	static int         g_iHeroPosX;
	static int         g_iHeroPosZ;

	//! 主角人物的详细位置信息.
	static float       g_fHeroPosX;
	static float       g_fHeroPosZ;

	//! 上次播放正确的技能id.
	static s_string    g_szLastSkill;

	//! 当前是否截屏
	static BOOL        g_bCatchScreen;
	
	//! 全局的handle,每创建完一个人物，设置此handle
	static int         g_iCreatureHandle;

	//! 是否显示怪物的名字。
	static BOOL        g_bDispMonsterName;

	//! 是否相机跟随人物
	static BOOL        g_bCamFollowHero;

	//! 相机的移动速度
	static float       g_fCamMoveSpeed;

	//! 是否打开怪物的自动发魔法功能
	static bool        g_bRandSkillPlay;

	//! 用于控制人物施法动作的速度
	static float       g_fSkillActSpeed;

	//! 是否显示调试信息
	static BOOL        g_bRenderDebugInfo;

	//! 是否在调入地图后，接下来遍历整个地图，用于检测地图数据的完整性。
	static BOOL        g_bSpaceMoveWholeMap;

	//! 是否需要调整地图的碰撞信息
	static BOOL        g_bAdjMapCollInfo;

	//! 是否鼠标移动时,在鼠标的位置显示周围格子的碰撞信息
	static BOOL        g_bDisplayColInfo;

	//! 画出场景内所有怪物的视野信息
	static BOOL        g_bDisplayAllMonInfo;

	//! 按鼠标键时, 是否按下ctrl键.
	static BOOL        g_bCtrlDown;

	//! 是否显示触发矩离
	static BOOL        g_bDisTrigger;
	//! 是否显示主动视野
	static BOOL        g_bDisVision;
	//! 是否显示逃跑矩离.
	static BOOL        g_bDisRun;
	//! 是否显示呼叫矩离.
	static BOOL        g_bDisCall;


	//! 是否查看怪物的正常步时间.
	static BOOL        g_bNormalMoveTime;

	//! 插入怪物后,是否选中当前插入的怪物
	static BOOL        g_bSelLastInsertMonst;

	//! 整个场景时间停止
	static BOOL        g_bSuspendTime;

	//! 是否显示二维的人物
	static BOOL        g_bDisplayCharInRect;

	//! 全局的特效id,用于测试包围盒.
	static int         g_iEffectId;


public:
	lm_gvar();
	~lm_gvar();

};

extern s_string g_szHeroDir[6];