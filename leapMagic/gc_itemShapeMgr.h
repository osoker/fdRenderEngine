///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_itemShapeMgr.h
 *
 *  Desc:     用于调入人物所有的装备名字，根据名字来确认切换人物装备的身体部位
 * 
 *  His:      River created @ 2006-8-1 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once

# include "stdafx.h"
# include "gc_command.h"
# include "gc_stdItemMgr.h"

//物品名称的长度
#define ITEMNAME_LENGTH			21		
//! 物品外形定义名字的最大长度
#define MAX_ITEMNAMELEN		32
//! 装备对应的装备文件的目录名的最大长度.
#define MAX_EQUIPDIRLEN			64
//! 动作名字的最大长度
#define MAX_ACTNAMELENGTH		32

#define MAX_ACTPERNAME			4

#define GLOBAL_EQUIPLIST		"equipment\\g_equip_fd.eql"

# define    EQUIPLIST_MAGIC   "equ"

//! 动作改变相关的数据结构。
struct gcs_actionEle
{
	char      m_szActName[MAX_ACTNAMELENGTH];
	int       m_iActNum;
	int       m_vecActId[MAX_ACTPERNAME];

	//! 从内存中调入当前的动作信息。
	//void       load_fromMem( BYTE*& _mstart );
};

typedef std::vector<gcs_actionEle>  VEC_actEle;

//! 物品外形显示数据，可以在人物身上体现出来的数据。
struct   ITEMSHAPE_DISPLAY
{
	//! 当前装备的名字。
	char			m_szItemName[MAX_ITEMNAMELEN];
	//! 需要多少个部分更换显示相关资源。
	int				m_iCPartNum;
	//! 需要替换的部位的id.
	int				m_arrId[MAX_SKINPART];
	//! 需要替换的meshId.
	int				m_arrMesh[MAX_SKINPART];
	//! 需要替换的skinId.
	int				m_arrSkin[MAX_SKINPART];
	//! 需要变化的动作数目。
	int				m_iCActNum;
	VEC_actEle		m_vecActNeedChange;//fixme:无用数据?
};



//! 客户端用到的装备数据结构
class CItemShape
{
private:
	//显示数据。
	ITEMSHAPE_DISPLAY*		m_pItemShapeDisplay;
	// 根据职业，性别，和装备ID生成的唯一标识索引
	DWORD               m_dwObjectID;
	//! 外形id.
	DWORD				m_dwShapeId;
	//! 物品的名字
	char				m_szItemName[MAX_ITEMNAMELEN];
	//! 装备对应图片文件.  //fixme:没有用到？
	char				m_szEquipPic[MAX_EQUIPDIRLEN];
	//! 当前物品对应的euq文件名，存储相对目录。
	char				m_szItemEquFileName[MAX_EQUIPDIRLEN];
	//! 图标的id
	DWORD				m_dwIconId;
	//! 特效文件
	char				m_szSpe[MAX_ITEMNAMELEN];

public:
	CItemShape();
	~CItemShape();

	bool			    is_itemShapeDisplay( void );
	void			    load_itemFromMem( BYTE*&  _ds );
	ITEMSHAPE_DISPLAY*	get_itemShapeDisplay( void );
	char*			    get_itemEquFileName( void ) { return m_szItemEquFileName; } 

	//! 获得物品特效文件名称
	char*               get_speffFileName(){ return m_szSpe; }
	//! 得到装备的名字
	const char*         get_itemName( void ) { return this->m_szItemName; } 

	//! 得到装备的looks ID
	int                 get_looksId( void )  { return this->m_dwShapeId; } 
	//! 得到装备的objectId
	int                 get_objectId( void ) { return this->m_dwObjectID; } 

	//! 得到此装备的job和sex信息
	int                 get_itemJobSexInfo( void );


};

typedef std::map< int, CItemShape >  MAP_equipment;


/** \brief
 *  全局的装备管理器，传入装备的名字，可以得到装备需要改变的身体部位。
 * 
 */
class gc_itemMgr : public ost_Singleton<gc_itemMgr>
{
private:
	//! 对每一个调入的item,item名字的hash_value和相应的结构构成此map
	MAP_equipment       m_sEquipMap;

	//! 
	int                 m_iEquipNum;

	//! 标准装备管理器
	CStdItemMgr         m_sStdItemMgr;

	gc_itemMgr();
	~gc_itemMgr();

	//! 从一个目录名，得到job和sexID.
	void                get_jobSexId( const char* _cdir,int& _job,int& _sex );

public:
	/** 得到一个sg_timer的Instance指针.
	 */
	static gc_itemMgr*    Instance( void );
	
	/** 删除一个sg_timer的Instance指针.
	 */
	static void         DInstance( void );

	//! 全局的初始化当前的道具（装备）管理器
	bool               init_itemMgr( void );


	//! 根据一个装备的名字，得到此装备需要切换的身体部位数目
	bool               get_equipFromName( const char* _cdir,const char* _name,os_equipment& _equip );

	//! 传入一个人物的目录名，确认此目录是否可以改变装备，只有主角人物所在的目录可以改变装备
	bool               can_changeEquip( const char* _cdir );

};