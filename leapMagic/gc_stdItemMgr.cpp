#include "StdAfx.h"
#include "gc_stdItemMgr.h"
#include "XlsExportFile.h"


CStdItemMgr::CStdItemMgr(void)
{
	guard;
	clear();
	unguard;
}

CStdItemMgr::~CStdItemMgr(void)
{
	guard;
	clear();
	unguard;
}

// 从文件载入数据
bool CStdItemMgr::LoadFile( const char* _FileName )
{
	guard;
	clear();
	std::string fn1,fn2,fn3;
	fn1 = _FileName;

	int in = (int)fn1.rfind( '.' );
	fn2 = fn1.substr( 0, in );
	fn2 += "~1.txt";

	fn3 = fn1.substr( 0, in );
	fn3 += "~2.txt";

	XlsExportFile xef;
	std::vector<std::string> v;

	if( !xef.openXls( fn1.c_str(), ITEMFIELD ) )
		return false;
	TStdItemInfo rc;
	memset( &rc,0,sizeof(TStdItemInfo) );
	while( !xef.isXlsEOF() )
	{
		if( !xef.readXlsLine( v ) )
			continue;

		rc.iStdIdx        = atoi( v[0].c_str() );
		strcpy( rc.cItemStdName, v[1].c_str() );
		rc.iShape         = atoi( v[2].c_str() );
		rc.btItemType     = atoi( v[3].c_str() );
		rc.btItemSubType  = atoi( v[4].c_str() );
		rc.wPrice         = atoi( v[5].c_str() );
		rc.wMaxDura       = atoi( v[6].c_str() );
		rc.dwJob          = atoi( v[7].c_str() );
		rc.btGender       = atoi( v[8].c_str() );
		rc.dwMapCode      = atoi( v[9].c_str() );
		rc.btItemLv       = atoi( v[10].c_str() );
		rc.btNeedType     = atoi( v[11].c_str() );
		rc.btNeedValue    = atoi( v[12].c_str() );
		rc.btOverLay      = atoi( v[13].c_str() );
		rc.btOverMaxCnt   = atoi( v[14].c_str() );

		rc.btCDGroupId    = atoi( v[15].c_str() );
		rc.dwCDGroupTime  = atoi( v[16].c_str() );

		rc.dwPublicCdTime = atoi( v[17].c_str() );
		rc.dwItemCdTime   = atoi( v[18].c_str() );
		rc.dwItemProperty = atoi( v[19].c_str() );
		rc.dwGroupID      = atoi( v[20].c_str() );
		rc.dwGroupInfo    = atoi( v[21].c_str() );
		rc.btUsedObjType  = atoi( v[22].c_str() );
		rc.btDistance     = atoi( v[23].c_str() );
		rc.PropertyCnt    = atoi( v[24].c_str() );
		rc.wMakeExp       = atoi( v[25].c_str() );
		rc.wMakeStartExp  = atoi( v[26].c_str() );
		rc.wMakeEndExp    = atoi( v[27].c_str() );
		rc.bReBuild		  = atoi( v[28].c_str() );
		rc.iReBuild		  = atoi( v[29].c_str() );
		rc.iNameColor	  = atoi( v[30].c_str() );

		rc.btInjectRange  = atoi( v[31].c_str() );

		addItems( rc );
		count_ ++;
	}
	xef.closeXls();

	//------------------------------------------

	if( !xef.openXls( fn2.c_str(), 4 ) )
		return false;
	TItemPropertyRecord rc2;
	
	unsigned int itemId = 0;
	while( !xef.isXlsEOF() )
	{
		if( !xef.readXlsLine( v ) )
			continue;

		itemId = atoi( v[0].c_str() );

		rc2.btPropertyType = atoi( v[1].c_str () );
		rc2.iValue1 = atoi( v[2].c_str() );
		rc2.iValue2 = atoi( v[3].c_str() );
		addProp( GetStdItem( itemId ), &rc2 );
	}
	xef.closeXls();

	//-------------------------------------------
	if( !xef.openXls( fn3.c_str(), 2 ) )
		return false;

	while( !xef.isXlsEOF() )
	{
		if( !xef.readXlsLine( v ) )
			continue;

		itemId = atoi( v[0].c_str() );
		
		TStdItemInfo* info = GetStdItem( itemId );
		if( info )
			sprintf( info->cItemDes,"%s",v[1].c_str() );
	}
	xef.closeXls();


	if( !loadPropertyConfig() )
	{
		osassert(false);
		return false;
	}
	if( !loadSynItemConfig() )
	{
		osassert(false);
		return false;
	}

	// 
	m_vecStdInfoNameHash.resize( records_.size() );
	for( int t_i=0;t_i<(int)records_.size();t_i ++ )
		m_vecStdInfoNameHash[t_i] = string_hash( records_[t_i]->cItemStdName );

	return true;

	unguard;
}

//标准物品的数量
int CStdItemMgr::GetStdItemCnt()
{
	guard;
	return (int)count_;
	unguard;
}

//! 根据一个物品的名字，来得到此物品的ShapeId
int CStdItemMgr::get_itemShapeId( const char* _name )
{
	guard;

	int   t_iHashVal = string_hash( _name );

	for( int t_i=0;t_i<(int)m_vecStdInfoNameHash.size();t_i ++ )
	{
		if( t_iHashVal == m_vecStdInfoNameHash[t_i] )
		{
			if( strcmp( _name,records_[t_i]->cItemStdName ) == 0 )
				return records_[t_i]->iShape;
		}
	}

	return -1;

	unguard;
}


//根据指定的标准物品索引返回指向标准物品的指针
TStdItemInfo* CStdItemMgr::GetStdItem( int _ItemStdIdx )
{
	guard;
	if( _ItemStdIdx <= 0 || _ItemStdIdx > (int)records_.size() )
		return 0;
	if( records_[_ItemStdIdx-1]->iStdIdx==0 )
		return 0;
	else
		return records_[_ItemStdIdx-1];
	unguard;
}

/// 添加物品
void CStdItemMgr::addItems( const TStdItemInfo& rc )
{
	guard;
	TStdItemInfo* p = 0;
	while( (int)records_.size() < rc.iStdIdx )
	{
		p = new TStdItemInfo;
		memset( p, 0, sizeof(TStdItemInfo) );
		clearPropSet( p );
		records_.push_back( p );
	}
	*(records_[rc.iStdIdx-1]) = rc;
	clearPropSet( (records_[rc.iStdIdx-1]) );
	unguard;
}

/// 清空
void CStdItemMgr::clear( void )
{
	guard;
	VEC_stdInfo_it it;
	for( it = records_.begin(); it != records_.end(); ++it )
	{
		if( *it ) 
			delete (*it);
	}
	records_.clear();
	count_ = 0;

	VEC_synInfo_it itsyn;
	for( itsyn = m_vSynItem.begin(); itsyn != m_vSynItem.end(); ++itsyn )
	{
		if( *itsyn ) 
			delete (*itsyn);
	}
	m_vSynItem.clear();

	MAP_proType_it itPro = NULL;
	for( itPro= m_pPropertyMap.begin(); itPro != m_pPropertyMap.end(); )
	{
		if( itPro->second )
			delete itPro->second;
		++itPro;
	}
	m_pPropertyMap.clear();

	unguard;
}

/// 将扩展属性列表清空
void CStdItemMgr::clearPropSet( TStdItemInfo* rc )
{
	guard;
	for( int i = 0; i < ITEMPROPERTY_MAXCNT; ++i )
		rc->tPtopertyS[i].btPropertyType = 0xff;
	unguard;
}

/// 添加一个属性记录
void CStdItemMgr::addProp( TStdItemInfo* rc, const TItemPropertyRecord* rc2 )
{
	guard;
	for( int i = 0; i < ITEMPROPERTY_MAXCNT; ++i )
	{
		if( rc->tPtopertyS[i].btPropertyType == 0xff )
		{
			rc->tPtopertyS[i] = (*rc2);
			break;
		}
	}
	unguard;
}
//// 读取物品扩展属性名称对应表
//bool CStdItemMgr::loadPropertyConfig()
//{
//	guard;
//
//	TItemPropertyTypeRecord* record = NULL;
//	XlsExportFile xef;
//
//	if( !xef.openXls( PROPERTY_FILE_NAME, MAX_PROPERTY_CNT ) )
//	{
//		return false;
//	}
//	m_pPropertyMap.clear();
//	std::vector<std::string> v;
//	while( !xef.isXlsEOF() )
//	{
//		if( !xef.readXlsLine( v ) )
//			continue;
//		record = new TItemPropertyTypeRecord;
//		int id = atoi(v[0].c_str());
//		record->btPropertyType = id;
//		sprintf( record->cPropertyName,v[1].c_str() ); 
//		sprintf( record->cPropertyMemo,v[2].c_str() );
//		record->cPropertyColor = atoi( v[4].c_str() );
//		record->btShow = atoi( v[5].c_str() );
//
//		m_pPropertyMap.insert( std::make_pair(id,record) );
//	}
//	xef.closeXls();
//
//	return true;
//
//	unguard;
//}

// 读取物品扩展属性名称对应表
bool CStdItemMgr::loadPropertyConfig()
{
	guard;

	TItemPropertyTypeRecord* record = NULL;
	XlsExportFile xef;

	if( !xef.openXls( PROPERTY_FILE_NAME, EXT_FIELD ) )
	{
		return false;
	}
	m_pPropertyMap.clear();
	std::vector<std::string> v;
	while( !xef.isXlsEOF() )
	{
		if( !xef.readXlsLine( v ) )
			continue;
		record = new TItemPropertyTypeRecord;
		int id = atoi(v[0].c_str());
		record->btPropertyType = id;
		sprintf( record->cPropertyName,v[1].c_str() ); 
		//sprintf( record->cPropertyMemo,v[2].c_str() );
		record->cPropertyColor = atoi( v[4].c_str() );
		record->btShow = atoi( v[5].c_str() );
		record->btPropertyStructNum = atoi( v[6].c_str() );	//! 结构数量
		//! 结构		
		int cnt=0;	
		for( int i = 7; i < (7+record->btPropertyStructNum*2); i+=2 )
		{

			TPropertyMemoType t_sMemo;
			t_sMemo.btPropertyType = atoi( v[i].c_str() );
			strcpy( t_sMemo.cPropertyStr, v[i+1].c_str() );
			
			record->tMemoType[cnt++] = t_sMemo;
		}

		m_pPropertyMap.insert( std::make_pair(id,record) );
	}
	xef.closeXls();

	return true;

	unguard;
}

std::string CStdItemMgr::GetPropertyNameById( int _id )
{
	guard;

	MAP_proType_it it = m_pPropertyMap.find(_id);
	if( it!=m_pPropertyMap.end() )
		return it->second->cPropertyName;
	return "";

	unguard;
}

std::string CStdItemMgr::GetPropertyDisp( char _id,int _value1,int _value2 )
{
	guard;

	return "hello";

	unguard;
}

//std::string CStdItemMgr::GetPropertyDisp( char _id,int _value1,int _value2 )
//{
//	guard;
//
//	std::string disp,memo,temp;
//	char value1[16] = {0};
//	sprintf( value1,"%d",_value1 );
//	char value2[16] = {0};
//	sprintf( value2,"%d",_value2 );
//
//	MAP_proType_it it = m_pPropertyMap.find(_id);
//	if( it==m_pPropertyMap.end() )
//		return "";
//
//	if( !it->second->btShow )
//		return "";
//
//	memo = it->second->cPropertyMemo;
//	int t_pos = -1;
//	t_pos = memo.find("&");
//
//	if( t_pos!=-1 )
//	{
//		disp = memo.substr( 0,t_pos );
//		disp.append( value1 );
//	}
//
//	temp = memo.substr( t_pos+1,memo.length() );
//	t_pos = temp.find("#");
//	if( t_pos!=-1 )
//	{
//		int t_idx = temp.find("#@");
//		if( t_idx!=-1 )
//		{
//			disp.append(temp.substr( 0,t_idx ));
//			sprintf( value2,"%d",_value2/1000 );
//			disp.append( value2 );
//			t_pos = ++t_idx;
//		}
//		else
//		{
//			disp.append(temp.substr( 0,t_pos ));
//			disp.append( value2 );
//		}
//	}
//	disp.append(temp.substr( t_pos+1,memo.length() ));
//	return disp;
//
//	unguard;
//}

DWORD CStdItemMgr::GetPropertyColorByColorId( int _colorid )
{
	guard;

	DWORD color = 0xffffff;
	switch( _colorid )
	{
	case 1: //白
		color = 0xffffff;
		break;
	case 2: //蓝
		color = 0x28B9B2;
		break;
	case 3: //金
		color = 0xF3EB7B;
		break;
	case 4: //绿
		color = 0x28B743;
		break;
	case 5: //橙
		color = 0xFF743A;
		break;
	default:
		break;
	}
	return color;

	unguard;
}

DWORD CStdItemMgr::GetPropertyShowColor( int _id )
{
	guard;

	DWORD color = 0xffffff;
	MAP_proType_it it = m_pPropertyMap.find(_id);
	if( it==m_pPropertyMap.end() )
		return color;

	color = GetPropertyColorByColorId( it->second->cPropertyColor );

	return color;

	unguard;
}

//-----------------制造物品相关----------------
/** \brief
 *	读取物品制造材料表
 *
 *	\return	bool : 载入成功返回true
 */
bool CStdItemMgr::loadSynItemConfig()
{
	guard;

	XlsExportFile t_cLoadFile;
	if( !t_cLoadFile.openXls( MAKEITEM_FILE_NAME, SYN_FIELD ) )
	{
		return false;
	}

	VEC_strvec t_vBuffer;
	//! 读取版本信息
	if( !t_cLoadFile.readXlsLine( t_vBuffer ) )
	{
		return false;
	}
	/*
	for( int i=0;i<(int)t_vBuffer.size();i++ )
	{
		osDebugOut( "SYN:%s", t_vBuffer[i].c_str() );
	}
	*/
	if(	t_vBuffer[0].compare( SYN_INFO ) != 0 ||
		t_vBuffer[1].compare( SYN_VERSION ) != 0)
	{
		t_cLoadFile.closeXls();
		return false; 
	}

	//! 读取数据
	while( !t_cLoadFile.isXlsEOF() )
	{
		t_vBuffer.clear();
		if( !t_cLoadFile.readXlsLine( t_vBuffer ) )
			continue;

		int t_iStdItemID = atoi( t_vBuffer[0].c_str() );

		TSynItemInfo *t_pSII = new TSynItemInfo;
		t_pSII->wSynIndex = atoi( t_vBuffer[0].c_str() );
		t_pSII->dwStdItemID = atoi( t_vBuffer[1].c_str() );
		int t_iElement = 0;
		for( int i=0, t_iPos=2; t_iPos < SYN_FIELD-1; i++,t_iPos+=2 )
		{
			t_pSII->arrElement[i].dwStdItemID = atoi( t_vBuffer[t_iPos].c_str() );
			t_pSII->arrElement[i].btNumber = atoi( t_vBuffer[t_iPos+1].c_str() );
			if( !t_vBuffer[t_iPos].empty() )
			{
				t_iElement++;
			}
		}
		t_pSII->btElement = t_iElement;
		t_pSII->dwNeedMoney = atoi( t_vBuffer[SYN_FIELD-1].c_str() );

		m_vSynItem.push_back( t_pSII );	
	}

	t_cLoadFile.closeXls();

	return true;

	unguard;
}

int CStdItemMgr::getSynMakeItemGold( int _idx )
{
	guard;

	int gold = 0;
	VEC_synInfo_it it;
	for( it = m_vSynItem.begin();it!=m_vSynItem.end();++it )
	{
		if( (*it)->wSynIndex==_idx )
		{
			gold = (*it)->dwNeedMoney;
			break;
		}
	}
	return gold;

	unguard;
}

void CStdItemMgr::getSynMakeItemByIdx( int _idx,TMakeItemInfo& _info )
{
	guard;


	unguard;
}

int CStdItemMgr::getSynMakeMaterialCnt( int _idx )
{
	return 0;
}

void CStdItemMgr::getSynMakeMaterialByIdx( int _idx,int _pos,TMakeItemInfo& _info )
{
	return;
}