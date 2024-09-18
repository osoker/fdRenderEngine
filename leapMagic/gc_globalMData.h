///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_globalMData.h
 *
 *  Desc:     ������������id,id��ȫ����Ϣ
 * 
 *  His:      River created @ 2007-3-9
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once


# include "stdafx.h"
# include <vector>

// ����ṹ��ÿһ��tg����128*128��,ռ�ÿռ�ܴ�,����ʹ��1���ֽڶ���
# pragma pack(push,r1,1) 

struct LOOKSINFO
{
	int	  looksId;
	char  dir[64];
	//!�������ͣ���Ч����ģ�ͣ�
	int	  monType;
	float scale;
	//!�Ƿ��¼equ�ļ�
	bool		m_bSaveEquName;
	//!equ�ļ���
	char		m_chEquFileName[64];
};
# pragma pack(pop,r1)


//! ��������id�͹����ӦĿ¼֮��Ķ�Ӧ������.
class  gc_monsterAppearance : public ost_Singleton<gc_monsterAppearance> 
{
private:
	gc_monsterAppearance();
	~gc_monsterAppearance();
	
	std::vector< LOOKSINFO >   m_vecMLInfo;


public:
	/** �õ�һ��gc_monsterAppearance��Instanceָ��.
	 */
	static gc_monsterAppearance*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void              DInstance( void );


	//! ������������id�͹����Ŀ¼֮��Ķ�Ӧ��Ϣ
	BOOL       load_globalMaInfo( const char* _fname );

	//! ��id�õ������������Ϣ
	const LOOKSINFO*  get_mLooksinfoFromId( int _id,float& _scale );

};



//! �������õ��������ϵļ������ݽṹ
struct SMonstMagicUse
{
	DWORD      m_iMagicId;
	int        m_iMagicLevel;
};

//! �ɰ汾�������õ��ļ������� sizeof() = 132
struct monstVersion1Data
{
	SMonstMagicUse  m_arrMagicUse[16];
	DWORD          m_dwMagicUseNum;
};

//! ��255��λ���в�������
struct bitset_255
{
	//! λ����
	DWORD     m_arrBit[8];

public:
	//! ����ĳһ��λ������
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

	//! �����λ��1,�����棬���򷵻�false
	BOOL      test_bit( int _idx )
	{
		assert( _idx < 0xff );

		int    t_iBit = _idx / 32;
		DWORD  t_iMask = 1 << (_idx&31); // (_idx&32 ) == (_idx%32)

		return (m_arrBit[t_iBit] & t_iMask);
	}

};

//! �����Aiѡ��   sizeof = 80 + 4*9
struct monstAIData   
{
	//! ai����:0 ��������.1:��������
	DWORD           m_dwAiType;

	//! ��������ѡ�񣺣�������.1:����.
	// River @ 2006-2-14:  2: �޹���
	DWORD           m_dwMainAttackType;
	SMonstMagicUse   m_sMainMagic;

	//! �ι�����ѡ��: 0: ����1:����. 2:�޹���
	DWORD           m_dwSecAttackType;
	SMonstMagicUse   m_sSecMagic;

	//! ��ǰ������쳣����.
	DWORD           m_dwExceptIdx;
	DWORD           m_dwCData[2];
	
	//! ����״̬������ѡ��
	bitset_255      m_sMianYi;

	//! ����ļ�������
	DWORD           m_dwMonstGrade;
	//! ����Ĺ���������
	WORD           m_wGuishu;
	//! ���������������
	WORD          m_wShili;

	//@{ River @ 2006-3-1: ������������.
   //! ����ֵ
	DWORD           m_dwMinZhong;
   //! �ֿ�������
	DWORD           m_dwDQWuLiKx;
   //! �ֿ�ħ������
	DWORD           m_dwDQMoFaKx;
   //! �ֿ۾�����
	DWORD           m_dwDQJingShenKx;
   //! ���ӷ�������
	DWORD           m_dwWSFangYuRate;
   //! �����˺�����
	DWORD           m_dwFTShangHaiRate;
   //! �����˺��ٷֱ�
	DWORD           m_dwFTShangHaiBFB;
	//@} 

	//@{ River @ 2006-3-2: ���뾭��ֵ
	DWORD           m_dwKillGetExp;
	//@} 

	//! River @ 2006-3-21: �������ܹ�������,��������뾶
	//! River @ 2007-3-14: �����ܹ��������޸�Ϊ�����뾶
	DWORD           m_dwTriggerDis;
	DWORD           m_dwMonstSize;
	//@} 


	//@{ River @ 2006-3-28: ������º������
    /* Ϊ�˲��䶯�ļ��ṹ���˱����ݷ���SMonstData�ṹ��
	//! ��������������Ƿ񹥻�����.false ������������������true ��������������
	BOOL            m_bAttactByCondition;
	*/
	//! ��Ӧ���������������еڶ��������̶�������������ʹ�������ļ�
	DWORD           m_dwFCondition;
	DWORD           m_dwSCondition;
	DWORD           m_dwTCondition;
	
	//@} 


};
union union_AIData
{
	//! �汾1�õ�������,
	monstVersion1Data     m_sOldMagic;
	//! �汾2�õ�������
	monstAIData           m_sMonstAiData;
};

//! ������ص�����
struct SMonstData
{
	//! ����ID 
	DWORD        m_dwMonstId;
	//! ��������
	char         m_szMonstName[20];
	//! �������α��
	DWORD        m_dwMonstAppearCode;

	//! �������ࣺ
	DWORD        m_dwMonstType;

	//! ����ֵ
	DWORD        m_dwHpVal;

	//! ħ��ֵ��
	DWORD        m_dwNewReserved1;;

	//! �������Ѿ�ɾ�����ã��ڴ˴����ڼ�����ǰ��"�ֿ�ֵ����"
	DWORD        m_dwReserved1;

	//! ����ֵ
	DWORD        m_dwShanBiVal;
	//! �������Ѿ�ɾ�����ã��ڴ˴����ڼ�����ǰ��"�м�ֵ"
	//! 2007-1-13: ���о��롡0��100
	DWORD        m_dwHuJiaoDistance;

	//! ˫�����
	DWORD        m_dwDoubleOutput;

	//! ����ֵ�������������������˹����ܵ���������Ʒ�˺�ֵ   
	DWORD        m_dwHuJiaVal;

	//! �����ԣ��ٷ����������������˹����ܵ��������ܵ��������˺�����  
	DWORD        m_dwWuLiKangXing;

	//! �����ԣ��ٷ����������������˹����ܵ��������ܵ��ľ����˺�����  
	DWORD        m_dwJingShenKangXing;

	//! ħ�����ԣ��ٷ����������������˹����ܵ��������ܵ���ħ���˺����� 
	DWORD        m_dwMoFaKangXing;

	//! ��С������������������  
	DWORD        m_dwMinWuLiAttack;

	//! ���������������������  
	DWORD        m_dwMaxWuLiAttack;

	//! ��Сħ������������������  
	DWORD        m_dwMinMoFaAttack;

	//! ���ħ������������������  
	DWORD        m_dwMaxMoFaAttack;

	//! ��С���񹥻�������������  
	DWORD        m_dwMinJingShengAttack;

	//! ����񹥻�������������  
	DWORD        m_dwMaxJingShengAttack;

	//! �����ָ������������������ָ�������λ�Ѫ֮��ļ��ʱ��  
	DWORD        m_dwShengMingHuiFuJiangGe;

	//! ħ���ָ������������������ָ�������λ�ħ֮��ļ��ʱ��  
	DWORD        m_dwNewReserved2;

	//! �����ָ�ֵ��������������ָ����ÿ�λָ������ĵ���    
	DWORD        m_dwShengMingHuiFuVal;

	//! ħ���ָ�ֵ��������������ָ����ÿ�λָ�ħ���ĵ���  
	DWORD        m_dwNewReserved3;

	//! �����ٶȣ�������������ָ�������η���������֮���ʱ�� 
	DWORD        m_dwAttackSpeed;

	//! �������룺������������ָ�����ܹ���ʼ�������ľ���  
	DWORD        m_dwAttackDis;

	//! �������Ѿ�ɾ�����ã��ڴ˴����ڼ�����ǰ��"����ظ�ʱ��"
	DWORD        m_dwReserved3;

	//@{ River @ 2006-3-29: ��������ֵ����������ֵ
	//!  
	DWORD        m_dwShiliInc;
	DWORD        m_dwShiliDec;
	//@} 

	//! ��ǰ������������
	DWORD        m_dwEnemyType;

	//@{ River @ 2006-3-29: ������������
	//! 0:0,�޽ű�����  1,�ű���ʼ�� 2,��ȫ�ű�����
	DWORD        m_dwMonstPRunType;


	/*
	//! �������Ѿ�ɾ�����ã��ڴ˴����ڼ�����ǰ��"�����ƶ��ٶ�"
	DWORD        m_dwReserved7;
	ʹ����������������µ�����
	*/
	//! ��������������Ƿ񹥻�����.false ������������������true ��������������
	BOOL            m_bAttactByCondition;

	//! �����ƶ����    
	DWORD        m_dwZhengChangYiDongJianGe;

	//! ׷���ƶ����   
	DWORD        m_dwZhuiJiYiDongJianGe;

	//! �����ƶ�����������༭��?
	DWORD        m_dwTaoPaoYiDongJianGe;

	//! ׷���뾶���������������Թ������ս��״̬�ĵ�ΪԲ�ġ� 
	DWORD        m_dwZhuiJiBanJing;

	//! ���ܰ뾶�������������������ܵ�ΪԲ�ĵ�һ�����ܰ뾶�����ﵽ���ܰ뾶��ʱ��Ͳ������ܡ�
	DWORD        m_dwTaoPaoBanJing;

	//! ������Ұ�������������������˹����ڶ��پ������ܱ��˷��֡�
	DWORD        m_dwBeiDongShiYe;

	//! ������Ұ�������������������˹����ܿ������پ������Ŀ�ꡣ
	DWORD        m_dwZhuDongShiYe;

	//! ʬ���Ƿ��ܱ���ȡ      CheckBox?    BYTE 
	BOOL         m_bBoDuo;

	//! �Ƿ��ܱ��ջ󡡡�������CheckBox?    BYTE
	BOOL         m_bYouHuo;

	//! ���������������ݡ�
	union_AIData    m_unAiData;

	//! ������е��б��ȹ�����������е����ݶ�����󣬸�������б���������������
	DWORD*         m_vecEnemyMonstId;
	//! �����Ӧ���ID����Ŀ
	DWORD          m_dwEnemyIdNum;

	//! �����Ӧ�Ĳ���
	DWORD          m_dwBuFu;
};

//! �ӹ����id�����ֵõ������ӦĿ¼�Ĺ�����.
class gc_monsterIdNameMgr : public ost_Singleton<gc_monsterIdNameMgr> 
{
private:
	gc_monsterIdNameMgr();
	~gc_monsterIdNameMgr();


	std::vector< SMonstData >  m_vecMonstData;

	s_string          m_szFileName;

public:
	/** �õ�һ��gc_monsterIdNameMgr��Instanceָ��.
	 */
	static gc_monsterIdNameMgr*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void              DInstance( void );

	//! ����ȫ���Ĺ�������
	BOOL         init_monstData( const char* _fname );

	//! �ӹ����id�õ������Ӧ��Ŀ¼��
	const char*  get_monstDirFromId( int _id,DWORD& _visualField,
		            DWORD& _trigger,DWORD& _escape,DWORD& _callDis,float& _scale );


	//! �õ������������������Ŀ
	int          get_monsterNum( void ) { return (int)m_vecMonstData.size(); }
	SMonstData*  get_monsterFromIdx( int _idx );

	//! ���ݹ����id,�õ��˹����������������ڵ�����
	int          get_monsterIdxFromId( int _id );

	//! ���ù�����ƶ�����,���׼���
	void         set_monsterStepLength( int _idx,float _length );
	//! ���ù���������ƶ����.�������.
	void         set_monsterNormalMoveTime( int _idx,float _time );
	//! ���ù����׷���ƶ����.�������.
	void         set_monsterAttackMoveTime( int _idx,float _time );

	//! �洢�޸ĺ�Ĺ�����Ϣ
	void         save_monsterData( void );

};
