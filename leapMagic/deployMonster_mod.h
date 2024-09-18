///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: deployMonster_mod.h
 *
 *  Desc:     �����޸�ˢ����Ϣ��ͷ�ļ�
 * 
 *  His:      River created @ 2007-3-8
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "gc_globalMData.h"


//! ����������Ķ�����Ϣ
struct SPoints
{
	int   mCount;
    POINT mPoints[MAX_POINTS];
};

//!����ĸ�����Ϣ
//!add by blueboy @2007,7,26
struct sAreaInfoEx
{
	//!�����Ƿ񼤻�
	BOOL	m_bActive;
	//����
	int		m_nReserve[50];
};

enum EAREA_TYPE
{
	TYPE_RECT = 0, //��������
	TYPE_SPECIAL,  //��������
	TYPE_RANDOM,   //�������
};

struct sAreaInfo 
{	
	//!������Ϣ
	DWORD			m_dwId;
	char			m_szName[20];

	//!��������
	EAREA_TYPE		m_AreaType;

	//!��������ʱ��Ϊ0,վ��(���ƶ�);Ϊ1,˳��Ѳ��;Ϊ2,����Ѳ��;Ϊ3,ֻ��һ��
	int		m_MotionType;
	
	//!�����1���������ʾ��ǵ��б������2��������ʾ����ζ����б�
	SPoints m_PointList;

	//!����Χ�����������������˼
	LONG    m_lCen_x;
	LONG    m_lCen_y;
	LONG    m_lWidth;
	LONG    m_lHeight;
	
	DWORD	m_dwColor;

	//!�������
	DWORD	m_dwnMonsts;
	//!�����ƫ��
	DWORD	m_dwofsMonst;

	//!��չ��Ϣ
	sAreaInfoEx m_AreaEx;

};


struct sMonstInfo {
	DWORD		m_dwMonstId;
	char		m_szMonstName[20];

	POINT       m_ptPostion;

	//!����ˢ������
	DWORD		m_dwMonstNum;
	//!0������ʱ��,1��ʵʱ��
	DWORD		m_dwMonstType;
	DWORD		m_dwBeginTime;
	DWORD		m_dwEndTime;
	char		m_szRemark[20];


};

struct MonstDistribHeader {
	char id[4];
	DWORD version;
	DWORD nAreas;
	DWORD ofsArea;
	DWORD nMonsts;
	DWORD ofsMonst;
};



//! ��leapMagic���ݲ������Ĺ�����Ϣ�ṹ
struct gc_monsterInsert
{
	//! ����λ��
	POINT      m_sInsertPt;
	//! ����Ķ�ӦĿ¼
	char       m_szMonsterDir[128];
	//! �����ڴ�������󣬹����handle.
	int        m_iMonsterHandle;

	//! �������Ұ��Ϣ,
	float      m_fVisualField;

	//! ����Ĵ����뾶.
	float      m_fTriggerDis;
	//! ��������ܰ뾶
	float      m_fEscapeDis;
	//! ����ĺ��о���
	float      m_fCallDis;
	//! ��������ű���
	float      m_fScale;


};



//! ��ͼ���ϰ汾ͷ�ṹ
struct MapVersionHead
{
	// �汾��
	unsigned int	m_dwVersion;
public:
	MapVersionHead()
	{
		m_dwVersion = 0;
	}
};

//! һ�����������Ĺ���������
# define MAX_MONSTERTYPENUM  20
/** \brief
 *  ˢ����Ϣ����,�����ά�༭���ڵ�ˢ����Ϣ,����ά���޸�
 *
 *
 */
class gc_deployMonsterMod : public ost_Singleton<gc_deployMonsterMod> 
{
private:

	gc_deployMonsterMod();
	~gc_deployMonsterMod();

	//! ����ˢ����Ϣ�Ƿ��Ѿ�����
	bool      m_bLoadedDeployData;


	std::vector<sAreaInfo>	      m_sAreaList;
	std::vector<sMonstInfo>	      m_sMonstList;
	MonstDistribHeader            m_sHeader;

	// River mod @ 2007-6-11:�ļ�ͷ���
	MapVersionHead                m_sMVHead;


	std::vector<gc_monsterInsert> m_vecDmmMonInfo;

	//! ˢ����Ϣ�ļ�������
	s_string                      m_szDmmFileName;

	//! ���ˢ���������Ŀ.
	int                        m_iRandomAreaNum;

	//! �õ���n���������
	sAreaInfo*                 get_randAreaFromIdx( int _idx );

	//! �õ�������������
	bool                        get_randAreaFromIdx( int _idx,int& _resIdx );


	// �������ʱʹ�õ�һЩ������Ϣ.
	static  s_string   m_sMonsterTypeDir[MAX_MONSTERTYPENUM];
	static  int        m_sMonsterTypeId[MAX_MONSTERTYPENUM];
	static  int        m_iMonsterTypeNum;

public:

	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static gc_deployMonsterMod*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void              DInstance( void );

	//! ����ʹ洢
	BOOL   lod_dmmData( const char* _fname );
	BOOL   save_dmmData( void );
	BOOL   is_dmmDataLoaded( void ) { return m_bLoadedDeployData; } 

	//! ���³��������е�ˢ�ֵ�λ����Ϣ
	BOOL   update_dmmPosInfo( void );


	//! �������Ϣ�༭��
	void   reset_dmmData( void );


	//! �Թ�����в����ĺ���
	bool   mod_monsterPos( osVec3D& _newpos );

	//! �õ�ˢ�����ϵĺ���
	bool   get_monsterData( int _idx );

	//! �õ��������Ŀ
	int    get_monsterAreaNum( void );
	const char* get_monsterAreaName( int _idx );

	//! �õ�����ļ�������
	const SPoints* get_monAreaGeoData( int _idx );
	
	//! �õ��������ڹ������Ŀ
	int    get_monsterNum( int _idx );
	//! �õ���������ÿһ��������Ϣ
	gc_monsterInsert* get_monsterInfoFromIdx( int _areaIdx,int _monIdx );

	//! ��һ��������ɾ��һ������
	bool   delete_monsterInArea( int _areaIdx,int _monIdx,int _handle );

	/** \brief
	 *  ��һ�����������һ������
	 *
	 *  \return  gc_monsterInsert* ���ع���Ĳ���ṹ,�������ϲ㴦������handle��Ϣ
	 */
	gc_monsterInsert*   insert_monsterInArea( int _areaIdx,int _monIdx,int _x,int _z );

	//! �õ���ǰ�������ܹ��й�������
	int          get_monsterType( int _areaIdx );
	const char*  get_monsterTypeDir( int _areaIdx,int _monIdx );
	//! �����������͹��������õ��������Ϣ
	bool         get_monInfoFromAreaTypeIdx( int _areaIdx,int _monIdx,sMonstInfo& _res );
	//
	bool         get_monInsertFromAreaTypeIdx( int _areaIdx,int _monIdx,gc_monsterInsert& _res );

	//! ��������Ŀ¼����
	const char*                process_monsterDir( char* _dir );


};