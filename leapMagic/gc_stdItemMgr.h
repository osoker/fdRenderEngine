#pragma once
//! ������Ʒ������
#include "StdItemInfo.h"

#define MAX_PROPERTY_CNT 6
#define PROPERTY_FILE_NAME  "equipment\\��Ʒ��չ����.txt"
#define MAKEITEM_FILE_NAME  "equipment\\������Ʒ���ϱ�.syn"

const int ITEMFIELD	= 32;	//! �ֶ�

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

	// ���ļ���������
	bool LoadFile( const char* _FileName );

	//��׼��Ʒ������
	int GetStdItemCnt();

	//����ָ���ı�׼��Ʒ��������ָ���׼��Ʒ��ָ��
	TStdItemInfo* GetStdItem( int _ItemStdIdx );

	//! ����һ����Ʒ�����֣����õ�����Ʒ��ShapeId
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

	// ���
	void clear( void );	
	
	// �����Ʒ
	void addItems( const TStdItemInfo& rc );

	// ����չ�����б����
	void clearPropSet( TStdItemInfo* rc );

	// ���һ�����Լ�¼
	void addProp( TStdItemInfo* rc, const TItemPropertyRecord* rc2 );

	bool loadPropertyConfig();
	/** \brief
	*	��ȡ��Ʒ������ϱ�
	*
	*	\return	bool : ����ɹ�����true
	*/
	bool loadSynItemConfig();

	VEC_int     m_vecStdInfoNameHash;
	VEC_stdInfo records_;
	MAP_proType m_pPropertyMap;
	size_t count_;
	VEC_synInfo m_vSynItem;	//! �ϳ���Ʒ������
};
