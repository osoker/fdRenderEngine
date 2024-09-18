#pragma once
//! 标推物品管理器
#include "StdItemInfo.h"

#define MAX_PROPERTY_CNT 6
#define PROPERTY_FILE_NAME  "equipment\\物品扩展属性.txt"
#define MAKEITEM_FILE_NAME  "equipment\\制造物品材料表.syn"

const int ITEMFIELD	= 32;	//! 字段

#define SYN_INFO		"SYN_ITEM"
#define SYN_VERSION		"2"
#define SYN_FIELD		23

typedef std::vector< std::string > VEC_strvec;
typedef VEC_strvec::iterator VEC_strvec_it;

typedef std::vector< TSynItemInfo* > VEC_synInfo;
typedef VEC_synInfo::iterator VEC_synInfo_it;

typedef std::map< unsigned int,TItemPropertyTypeRecord* > MAP_proType;
typedef MAP_proType::iterator MAP_proType_it;

typedef std::vector< TStdItemInfo* > VEC_stdInfo;
typedef VEC_stdInfo::iterator VEC_stdInfo_it;

struct TMakeItemInfo
{
	int stdid;
	int iconid;
	int itemcnt;
	std::string name;
};

class CStdItemMgr
{
public:
	CStdItemMgr(void);
	~CStdItemMgr(void);

	// 从文件载入数据
	bool LoadFile( const char* _FileName );

	//标准物品的数量
	int GetStdItemCnt();

	//根据指定的标准物品索引返回指向标准物品的指针
	TStdItemInfo* GetStdItem( int _ItemStdIdx );

	//! 根据一个物品的名字，来得到此物品的ShapeId
	int           get_itemShapeId( const char* _name );


	std::string GetPropertyNameById( int _id );
	std::string GetPropertyDisp( char _id,int _value1,int _value2 );
	DWORD GetPropertyShowColor( int _id );
	DWORD GetPropertyColorByColorId( int _colorid );

	void getSynMakeItemByIdx( int _idx,TMakeItemInfo& _info );
	void getSynMakeMaterialByIdx( int _idx,int _pos,TMakeItemInfo& _info );
	int getSynMakeItemGold( int _idx );
	int getSynMakeMaterialCnt( int _idx );

private:

	// 清空
	void clear( void );	
	
	// 添加物品
	void addItems( const TStdItemInfo& rc );

	// 将扩展属性列表清空
	void clearPropSet( TStdItemInfo* rc );

	// 添加一个属性记录
	void addProp( TStdItemInfo* rc, const TItemPropertyRecord* rc2 );

	bool loadPropertyConfig();
	/** \brief
	*	读取物品制造材料表
	*
	*	\return	bool : 载入成功返回true
	*/
	bool loadSynItemConfig();

	VEC_int     m_vecStdInfoNameHash;
	VEC_stdInfo records_;
	MAP_proType m_pPropertyMap;
	size_t count_;
	VEC_synInfo m_vSynItem;	//! 合成物品的数组
};
