///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_itemShapeMgr.h
 *
 *  Desc:     ���ڵ����������е�װ�����֣�����������ȷ���л�����װ�������岿λ
 * 
 *  His:      River created @ 2006-8-1 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once

# include "stdafx.h"
# include "gc_command.h"
# include "gc_stdItemMgr.h"

//��Ʒ���Ƶĳ���
#define ITEMNAME_LENGTH			21		
//! ��Ʒ���ζ������ֵ���󳤶�
#define MAX_ITEMNAMELEN		32
//! װ����Ӧ��װ���ļ���Ŀ¼������󳤶�.
#define MAX_EQUIPDIRLEN			64
//! �������ֵ���󳤶�
#define MAX_ACTNAMELENGTH		32

#define MAX_ACTPERNAME			4

#define GLOBAL_EQUIPLIST		"equipment\\g_equip_fd.eql"

# define    EQUIPLIST_MAGIC   "equ"

//! �����ı���ص����ݽṹ��
struct gcs_actionEle
{
	char      m_szActName[MAX_ACTNAMELENGTH];
	int       m_iActNum;
	int       m_vecActId[MAX_ACTPERNAME];

	//! ���ڴ��е��뵱ǰ�Ķ�����Ϣ��
	//void       load_fromMem( BYTE*& _mstart );
};

typedef std::vector<gcs_actionEle>  VEC_actEle;

//! ��Ʒ������ʾ���ݣ������������������ֳ��������ݡ�
struct   ITEMSHAPE_DISPLAY
{
	//! ��ǰװ�������֡�
	char			m_szItemName[MAX_ITEMNAMELEN];
	//! ��Ҫ���ٸ����ָ�����ʾ�����Դ��
	int				m_iCPartNum;
	//! ��Ҫ�滻�Ĳ�λ��id.
	int				m_arrId[MAX_SKINPART];
	//! ��Ҫ�滻��meshId.
	int				m_arrMesh[MAX_SKINPART];
	//! ��Ҫ�滻��skinId.
	int				m_arrSkin[MAX_SKINPART];
	//! ��Ҫ�仯�Ķ�����Ŀ��
	int				m_iCActNum;
	VEC_actEle		m_vecActNeedChange;//fixme:��������?
};



//! �ͻ����õ���װ�����ݽṹ
class CItemShape
{
private:
	//��ʾ���ݡ�
	ITEMSHAPE_DISPLAY*		m_pItemShapeDisplay;
	// ����ְҵ���Ա𣬺�װ��ID���ɵ�Ψһ��ʶ����
	DWORD               m_dwObjectID;
	//! ����id.
	DWORD				m_dwShapeId;
	//! ��Ʒ������
	char				m_szItemName[MAX_ITEMNAMELEN];
	//! װ����ӦͼƬ�ļ�.  //fixme:û���õ���
	char				m_szEquipPic[MAX_EQUIPDIRLEN];
	//! ��ǰ��Ʒ��Ӧ��euq�ļ������洢���Ŀ¼��
	char				m_szItemEquFileName[MAX_EQUIPDIRLEN];
	//! ͼ���id
	DWORD				m_dwIconId;
	//! ��Ч�ļ�
	char				m_szSpe[MAX_ITEMNAMELEN];

public:
	CItemShape();
	~CItemShape();

	bool			    is_itemShapeDisplay( void );
	void			    load_itemFromMem( BYTE*&  _ds );
	ITEMSHAPE_DISPLAY*	get_itemShapeDisplay( void );
	char*			    get_itemEquFileName( void ) { return m_szItemEquFileName; } 

	//! �����Ʒ��Ч�ļ�����
	char*               get_speffFileName(){ return m_szSpe; }
	//! �õ�װ��������
	const char*         get_itemName( void ) { return this->m_szItemName; } 

	//! �õ�װ����looks ID
	int                 get_looksId( void )  { return this->m_dwShapeId; } 
	//! �õ�װ����objectId
	int                 get_objectId( void ) { return this->m_dwObjectID; } 

	//! �õ���װ����job��sex��Ϣ
	int                 get_itemJobSexInfo( void );


};

typedef std::map< int, CItemShape >  MAP_equipment;


/** \brief
 *  ȫ�ֵ�װ��������������װ�������֣����Եõ�װ����Ҫ�ı�����岿λ��
 * 
 */
class gc_itemMgr : public ost_Singleton<gc_itemMgr>
{
private:
	//! ��ÿһ�������item,item���ֵ�hash_value����Ӧ�Ľṹ���ɴ�map
	MAP_equipment       m_sEquipMap;

	//! 
	int                 m_iEquipNum;

	//! ��׼װ��������
	CStdItemMgr         m_sStdItemMgr;

	gc_itemMgr();
	~gc_itemMgr();

	//! ��һ��Ŀ¼�����õ�job��sexID.
	void                get_jobSexId( const char* _cdir,int& _job,int& _sex );

public:
	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static gc_itemMgr*    Instance( void );
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void         DInstance( void );

	//! ȫ�ֵĳ�ʼ����ǰ�ĵ��ߣ�װ����������
	bool               init_itemMgr( void );


	//! ����һ��װ�������֣��õ���װ����Ҫ�л������岿λ��Ŀ
	bool               get_equipFromName( const char* _cdir,const char* _name,os_equipment& _equip );

	//! ����һ�������Ŀ¼����ȷ�ϴ�Ŀ¼�Ƿ���Ըı�װ����ֻ�������������ڵ�Ŀ¼���Ըı�װ��
	bool               can_changeEquip( const char* _cdir );

};