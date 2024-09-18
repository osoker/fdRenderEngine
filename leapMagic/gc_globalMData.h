///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_globalMData.h
 *
 *  Desc:     调入怪物的外形id,id等全局信息
 * 
 *  His:      River created @ 2007-3-9
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once


# include "stdafx.h"
# include <vector>

// 这个结构在每一个tg内是128*128个,占用空间很大,所以使用1个字节对齐
# pragma pack(push,r1,1) 

struct LOOKSINFO
{
	int	  looksId;
	char  dir[64];
	//!怪物类型（特效还是模型）
	int	  monType;
	float scale;
	//!是否纪录equ文件
	bool		m_bSaveEquName;
	//!equ文件名
	char		m_chEquFileName[64];
};
# pragma pack(pop,r1)


//! 怪物外形id和怪物对应目录之间的对应管理器.
class  gc_monsterAppearance : public ost_Singleton<gc_monsterAppearance> 
{
private:
	gc_monsterAppearance();
	~gc_monsterAppearance();
	
	std::vector< LOOKSINFO >   m_vecMLInfo;


public:
	/** 得到一个gc_monsterAppearance的Instance指针.
	 */
	static gc_monsterAppearance*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void              DInstance( void );


	//! 读入怪物的外形id和怪物的目录之间的对应信息
	BOOL       load_globalMaInfo( const char* _fname );

	//! 从id得到怪物的外形信息
	const LOOKSINFO*  get_mLooksinfoFromId( int _id,float& _scale );

};



//! 可以作用到怪物身上的技能数据结构
struct SMonstMagicUse
{
	DWORD      m_iMagicId;
	int        m_iMagicLevel;
};

//! 旧版本怪物物用到的技术数据 sizeof() = 132
struct monstVersion1Data
{
	SMonstMagicUse  m_arrMagicUse[16];
	DWORD          m_dwMagicUseNum;
};

//! 对255个位进行操作的类
struct bitset_255
{
	//! 位数据
	DWORD     m_arrBit[8];

public:
	//! 设置某一个位的数据
	void      set_bit( int _idx,bool _set )
	{
		assert( _idx < 0xff );
		int    t_iBit = _idx / 32;

		if( _set )
		{
			DWORD  t_dwMask = 1 << (_idx&31); // (_idx&32 ) == (_idx%32)
			m_arrBit[t_iBit] |= t_dwMask;
		}
		else
		{
			DWORD  t_dwMask = 0xffffffff ^ (1<<_idx&31) ;// (_idx&32 ) == (_idx%32)
			m_arrBit[t_iBit] &= t_dwMask;
		}
	}

	//! 如果此位是1,返回真，否则返回false
	BOOL      test_bit( int _idx )
	{
		assert( _idx < 0xff );

		int    t_iBit = _idx / 32;
		DWORD  t_iMask = 1 << (_idx&31); // (_idx&32 ) == (_idx%32)

		return (m_arrBit[t_iBit] & t_iMask);
	}

};

//! 怪物的Ai选项   sizeof = 80 + 4*9
struct monstAIData   
{
	//! ai类型:0 主动攻击.1:被动攻击
	DWORD           m_dwAiType;

	//! 主攻技能选择：０：物理.1:技能.
	// River @ 2006-2-14:  2: 无攻击
	DWORD           m_dwMainAttackType;
	SMonstMagicUse   m_sMainMagic;

	//! 次攻技能选择: 0: 物理。1:技能. 2:无攻击
	DWORD           m_dwSecAttackType;
	SMonstMagicUse   m_sSecMagic;

	//! 当前怪物的异常属性.
	DWORD           m_dwExceptIdx;
	DWORD           m_dwCData[2];
	
	//! 所有状态的免疫选择
	bitset_255      m_sMianYi;

	//! 怪物的级别数据
	DWORD           m_dwMonstGrade;
	//! 怪物的归属索引。
	WORD           m_wGuishu;
	//! 怪物的势力索引。
	WORD          m_wShili;

	//@{ River @ 2006-3-1: 加入七项属性.
   //! 命中值
	DWORD           m_dwMinZhong;
   //! 抵扣物理抗性
	DWORD           m_dwDQWuLiKx;
   //! 抵扣魔法抗性
	DWORD           m_dwDQMoFaKx;
   //! 抵扣精神抗性
	DWORD           m_dwDQJingShenKx;
   //! 无视防御几率
	DWORD           m_dwWSFangYuRate;
   //! 反弹伤害几率
	DWORD           m_dwFTShangHaiRate;
   //! 反弹伤害百分比
	DWORD           m_dwFTShangHaiBFB;
	//@} 

	//@{ River @ 2006-3-2: 加入经验值
	DWORD           m_dwKillGetExp;
	//@} 

	//! River @ 2006-3-21: 加入智能攻击矩离,怪物自身半径
	//! River @ 2007-3-14: 把智能攻击矩离修改为触发半径
	DWORD           m_dwTriggerDis;
	DWORD           m_dwMonstSize;
	//@} 


	//@{ River @ 2006-3-28: 加入更新后的智能
    /* 为了不变动文件结构，此变量暂放在SMonstData结构内
	//! 符合条件后怪物是否攻击人物.false 符合条件不攻击。　true 符合条件攻击。
	BOOL            m_bAttactByCondition;
	*/
	//! 相应的三个条件，其中第二个条件固定，其它的两个使用配置文件
	DWORD           m_dwFCondition;
	DWORD           m_dwSCondition;
	DWORD           m_dwTCondition;
	
	//@} 


};
union union_AIData
{
	//! 版本1用到的数据,
	monstVersion1Data     m_sOldMagic;
	//! 版本2用到的数据
	monstAIData           m_sMonstAiData;
};

//! 怪物相关的数据
struct SMonstData
{
	//! 怪物ID 
	DWORD        m_dwMonstId;
	//! 怪物名称
	char         m_szMonstName[20];
	//! 怪物外形编号
	DWORD        m_dwMonstAppearCode;

	//! 怪物种类：
	DWORD        m_dwMonstType;

	//! 生命值
	DWORD        m_dwHpVal;

	//! 魔法值：
	DWORD        m_dwNewReserved1;;

	//! 此数据已经删除不用，在此处用于兼容以前的"抵抗值参数"
	DWORD        m_dwReserved1;

	//! 闪避值
	DWORD        m_dwShanBiVal;
	//! 此数据已经删除不用，在此处用于兼容以前的"招架值"
	//! 2007-1-13: 呼叫矩离　0－100
	DWORD        m_dwHuJiaoDistance;

	//! 双倍输出
	DWORD        m_dwDoubleOutput;

	//! 护甲值：整数，常量，决定了怪物能抵消多少物品伤害值   
	DWORD        m_dwHuJiaVal;

	//! 物理抗性：百分数，常量，决定了怪物能抵消多少受到的物理伤害比例  
	DWORD        m_dwWuLiKangXing;

	//! 精神抗性：百分数，常量，决定了怪物能抵消多少受到的精神伤害比例  
	DWORD        m_dwJingShenKangXing;

	//! 魔法抗性：百分数，常量，决定了怪物能抵消多少受到的魔法伤害比例 
	DWORD        m_dwMoFaKangXing;

	//! 最小物理攻击：整数，常量  
	DWORD        m_dwMinWuLiAttack;

	//! 最大物理攻击：整数，常量  
	DWORD        m_dwMaxWuLiAttack;

	//! 最小魔法攻击：整数，常量  
	DWORD        m_dwMinMoFaAttack;

	//! 最大魔法攻击：整数，常量  
	DWORD        m_dwMaxMoFaAttack;

	//! 最小精神攻击：整数，常量  
	DWORD        m_dwMinJingShengAttack;

	//! 最大精神攻击：整数，常量  
	DWORD        m_dwMaxJingShengAttack;

	//! 生命恢复间隔：整数，常量，指怪物两次回血之间的间隔时间  
	DWORD        m_dwShengMingHuiFuJiangGe;

	//! 魔法恢复间隔：整数，常量，指怪物两次回魔之间的间隔时间  
	DWORD        m_dwNewReserved2;

	//! 生命恢复值：整数，常量，指怪物每次恢复生命的点数    
	DWORD        m_dwShengMingHuiFuVal;

	//! 魔法恢复值：整数，常量，指怪物每次恢复魔法的点数  
	DWORD        m_dwNewReserved3;

	//! 攻击速度：整数，常量，指怪物两次发出物理攻击之间的时间 
	DWORD        m_dwAttackSpeed;

	//! 攻击距离：整数，常量，指怪物能够开始物理攻击的距离  
	DWORD        m_dwAttackDis;

	//! 此数据已经删除不用，在此处用于兼容以前的"打击回复时间"
	DWORD        m_dwReserved3;

	//@{ River @ 2006-3-29: 势力增加值与势力减少值
	//!  
	DWORD        m_dwShiliInc;
	DWORD        m_dwShiliDec;
	//@} 

	//! 当前怪物的天敌类型
	DWORD        m_dwEnemyType;

	//@{ River @ 2006-3-29: 怪物运行类型
	//! 0:0,无脚本运行  1,脚本初始化 2,完全脚本控制
	DWORD        m_dwMonstPRunType;


	/*
	//! 此数据已经删除不用，在此处用于兼容以前的"逃跑移动速度"
	DWORD        m_dwReserved7;
	使用这个数据来处理新的数据
	*/
	//! 符合条件后怪物是否攻击人物.false 符合条件不攻击。　true 符合条件攻击。
	BOOL            m_bAttactByCondition;

	//! 正常移动间隔    
	DWORD        m_dwZhengChangYiDongJianGe;

	//! 追击移动间隔   
	DWORD        m_dwZhuiJiYiDongJianGe;

	//! 逃跑移动间隔　　　编辑框?
	DWORD        m_dwTaoPaoYiDongJianGe;

	//! 追击半径：整数，常量，以怪物进入战斗状态的点为圆心。 
	DWORD        m_dwZhuiJiBanJing;

	//! 逃跑半径：整数，常量，以逃跑点为圆心的一个逃跑半径，当达到逃跑半径的时候就不再逃跑。
	DWORD        m_dwTaoPaoBanJing;

	//! 被动视野：整数，常量，决定了怪物在多少距离外能被人发现。
	DWORD        m_dwBeiDongShiYe;

	//! 主动视野：整数，常量，决定了怪物能看到多少距离外的目标。
	DWORD        m_dwZhuDongShiYe;

	//! 尸体是否能被剥取      CheckBox?    BYTE 
	BOOL         m_bBoDuo;

	//! 是否能被诱惑　　　　　CheckBox?    BYTE
	BOOL         m_bYouHuo;

	//! 怪物的智能相关数据。
	union_AIData    m_unAiData;

	//! 怪物天敌的列表，等怪物管理器所有的数据都调入后，根据天敌列表和天敌类型来计算
	DWORD*         m_vecEnemyMonstId;
	//! 怪物对应天敌ID的数目
	DWORD          m_dwEnemyIdNum;

	//! 怪物对应的步幅
	DWORD          m_dwBuFu;
};

//! 从怪物的id和名字得到怪物对应目录的管理器.
class gc_monsterIdNameMgr : public ost_Singleton<gc_monsterIdNameMgr> 
{
private:
	gc_monsterIdNameMgr();
	~gc_monsterIdNameMgr();


	std::vector< SMonstData >  m_vecMonstData;

	s_string          m_szFileName;

public:
	/** 得到一个gc_monsterIdNameMgr的Instance指针.
	 */
	static gc_monsterIdNameMgr*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void              DInstance( void );

	//! 读入全部的怪物数据
	BOOL         init_monstData( const char* _fname );

	//! 从怪物的id得到怪物对应的目录名
	const char*  get_monstDirFromId( int _id,DWORD& _visualField,
		            DWORD& _trigger,DWORD& _escape,DWORD& _callDis,float& _scale );


	//! 得到整个怪物管理器的数目
	int          get_monsterNum( void ) { return (int)m_vecMonstData.size(); }
	SMonstData*  get_monsterFromIdx( int _idx );

	//! 根据怪物的id,得到此怪物在整个管理器内的索引
	int          get_monsterIdxFromId( int _id );

	//! 设置怪物的移动步幅,按米计算
	void         set_monsterStepLength( int _idx,float _length );
	//! 设置怪物的正常移动间隔.按秒计算.
	void         set_monsterNormalMoveTime( int _idx,float _time );
	//! 设置怪物的追击移动间隔.按秒计算.
	void         set_monsterAttackMoveTime( int _idx,float _time );

	//! 存储修改后的怪物信息
	void         save_monsterData( void );

};
