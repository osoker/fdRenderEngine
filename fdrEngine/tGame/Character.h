# pragma once
//===================================================================
/** \file  
*  Filename: Character.h
*  Desc:     场景中角色人物的定义
*  His:      王凯 @ 1/12 2004 16:18
*/
//===================================================================

# include "newAstar.h"

//============================================================================

/**
*	单元格大小
*/
#define	 BASESIZE					TILE_WIDTH/2.0f
/**
*	！人物状态的定义
*/
#define  OBJ_STATE_WAIT				0
#define  OBJ_STATE_WALK				1
#define  OBJ_STATE_RUN				2
#define  OBJ_STATE_ACTION			3
#define  OBJ_STATE_ACT				4
#define  OBJ_STATE_BEACT			5
#define  OBJ_STATE_DEATH			6

#define  OBJ_STATE_LEAF				7

//等待加入其它状态

/**
*	!ini文件key-value的定义
*/
#define  HERO						"HERO"
#define  NPC						"NPC"
#define  NAME						"名字"
#define  DIRTCTION					"目录名"
#define  FIELDX						"FIELDX"
#define  FIELDY						"FIELDY"
#define  POSX						"POSX"
#define  POSY						"POSY"
#define  HEIGHT						"HEIGHT"
//!下面四行只对目前的npc有效
#define  PATHPOSX1					"路径一X1"
#define  PATHPOSY1					"路径一Y1"
#define  PATHPOSX2					"路径二X2"
#define  PATHPOSY2					"路径二Y2"

#define  ANGLE						"角度"

#define  SHADEDIR					"Character\\"

class I_Hero;
class I_Npc;
class CNpcManager;


//extern 	os_skinMeshInit   g_smInitOne;
//! 由4个8位字符生成无符号32位类型数值宏
#define OSMAKEFOURCC(ch0, ch1, ch2, ch3)			\
	((DWORD)(unsigned char)(ch0) | ((DWORD)(unsigned char)(ch1) << 8) |		\
	((DWORD)(unsigned char)(ch2) << 16) | ((DWORD)(unsigned char)(ch3) << 24 ))

#define ChaFileID OSMAKEFOURCC('C','H','A','\0')

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
	DWORD	mReserved3;
	DWORD	mReserved4;
};

// 人物伴随特效 
struct SChaEffectItem
{
	DWORD mNameOfs;	
	DWORD mNameNr;
	osVec3D	mEffectOffset;
};



/**brief
*!人物基类接口
*/
struct I_Character
{
	//!名字
	std::string					m_strName;
	//!由I_fdScene创建的对象
	I_skinMeshObj				*m_pObj;
	//！身体部分的数量
	int							m_nPartNum;
	//！当前状态
	UINT						m_nState;
	//!生命值
	int							m_nLife;
	//!起始位置
	osVec3D						m_verPos;
	//!角色离地面的高度
	float						m_fHeight;
	//!旋转的角度
	float						m_fAngle;
	//!当前的方向
	osVec3D						m_vCurDir;
	//!角色的速度
	float						m_fSpeed;
	//!寻路找到的路径。
	/*std::list<BaseMesh>*/PATH			m_VECPath;
	//!人物当前所在的格子
	BaseMesh					m_sBaseMesh;
	//!当前人物所在格子的高度值
	float						m_fHei;
	//!人物所在的当前地图
	int							m_nFieldX;
	int							m_nFieldY;
	//!装备列表
	std::map<s_string, os_equipment>   m_MAPequipment;
	//!可用的全部装备的数量
	int							m_nTotalEquipCount;
	//!可用的全部装备的名称
	std::vector<s_string>		m_VECEquipName;
	//！当前装备的名称
	s_string					m_strCurEquipName;
	//!当前装备
	os_equipment			    *m_pCurEquip;
	//!全部的动作数量
	int							m_nTotalActCount;
	//!当前动作
	int						    m_nCurAct;
	//!当前动作执行一次所需要的时间
	float						m_fFrameTime;
	//!执行动作已经用过的时间
	float						m_fActTime;
	//!变换矩阵
	osMatrix				    m_matWorld;
};
/**brief
*!主角接口定义
*/
class I_Hero : public I_Character
{
public:
	I_Hero(void);
	~I_Hero(void);

	int							m_LightID;
public:
	//!查找路径
	bool FindPath(int _nX, int _nY, int _X, int _Y);

	//!移动人物
	void Move(float _fTime);

	//!变换装备
	void ChangeEquipment();

	//!变换动作
	void ChangeAction();
	//!变换动作2
	void ChangeAction(int _nCurAct);

	//!查找周围的八个格子
	void FindMesh(CNpcManager *_pNpcManager, float _fTime);


	//! 调入英雄的时候调入英雄的所有的动作,这个函数用于切换英雄的动作到下一个动作.
	void change_nextAction( void );


	//! 处理当前Character的小量移动，用于平滑地运动相机的位置。
	void move_heroX( float _xdis );
	void move_heroZ( float _zdis );
	void move_heroY( float _ydis );

private:
	//!得到当前位置在整个地图中的索引,并返回下一个格子的中心点
	void  GetCurMeshIndex();


public:
    //！当前正在攻击的对象
	I_Npc*     m_pcNpc;

	//! 只有走路的动作播放声音.
	bool       m_bSoundPlay;

	//! 一个动作的时间长度.
	float      m_fActTime;
	//! 动作时间的比例开始播放声音.
	float      m_fS1,m_fS2;
	bool       m_bS1,m_bS2;

	//! 播放人物动作声音的frameMovel
	void       frame_moveSnd( void );


};
/**brief
*!NPC接口定义
*/
class I_Npc : public I_Character
{
public:
	I_Npc(void);
	~I_Npc(void);

public:
	//!预先设定好的三个节点
	std::vector<BaseMesh>		m_VECNode;
	
public:
	//!查找路径
	bool FindPath();
	//!移动人物
	void Move(float _fTime);
	//!变换动作
	void ChangeAction();
	//!变换动作2
	void ChangeAction(int _nCurAct);

	BOOL operator == ( CONST I_Npc& t_Npc )
	{
		return (m_strName == t_Npc.m_strName);
	}
	BOOL operator == ( CONST s_string& _strName )
	{
		return (m_strName == _strName);
	}
private:
	//!得到当前位置在整个地图中的索引,并返回下一个格子的中心点
	void  GetCurMeshIndex();
};

/**brief
* !NPC管理类定义
*/
class CNpcManager
{
public:
	CNpcManager(void);
	~CNpcManager(void);

public:
	//!从一个目录下读取全部的Npc的ini文件
	bool Create(char *_pchDirName);

	void Reset();

	//!更新全部npc的位置
	void UpdatePos(float _fTime);

	//!根据名字得到npc
	I_Npc*	GetNpcFromName(s_string& _strName);

	//!根据名字删除一个npc
	void	RemoveNpcFromName(s_string& _strName);

	//!得到全部的npc列表
	std::vector<I_Npc*>* GetTotalNpcList(){
		return &m_VECNpcs;
	}
	std::vector<I_Npc*>		m_VECNpcs;
};
/**brief
* !从一个ini文件装入一个hero
*/
bool LoadHeroFromIni(char *_chPath, I_Hero *_pHero);




/**brief
*!计算从地图格子的索引到世界坐标的位置
*/
osVec2D FromIndexToVertex(int _nX, int _nY, int _X, int _Y);

/**brief
*!计算从世界地图格子的索引到世界坐标的位置
*/
osVec2D FromWorldIndexToVertex(int _X, int _Y);

/**brief
*!计算从本地地图格子的索引到世界格子的索引
*/
void FromLocalToWorld(int _nX, int _nY, int _X, int _Y, BaseMesh& _BaseMesh);

/**brief
*!计算从世界地图格子的索引到本地格子的索引
*/
void FromWorldToLocal(int _X, int _Y, int& _nX, int& _nY, BaseMesh& _BaseMesh);

bool   gfEqual(  float _1, float _2  );