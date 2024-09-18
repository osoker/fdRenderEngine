///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_itemShapeMgr.cpp
 *
 *  Desc:     用于调入人物所有的装备名字，根据名字来确认切换人物装备的身体部位
 * 
 *  His:      River created @ 2006-8-1 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "gc_itemShapeMgr.h"

//! 全局装备文件的版本号
#define EQUIPLIST_FVERSION		 300

//! 标准装备的文件名
# define STD_ITEMINFOFILE      "equipment\\StdItem.txt"

// 职业编码
# define   CAREER_COMMON  0x00
# define   CAREER_FIGHTER 0x01
# define   CAREER_RABBI   0x02
# define   CAREER_PRIEST  0x03
// 性别编码
# define   SEX_COMMON 0x00
# define   SEX_MALE   0x01
# define   SEX_FEMALE 0x02

// 每一个角色对应的目录
# define   CHAR_NANZANS "nanzhan"
# define   CHAR_NVZANSI "nvzhans"
# define   CHAR_NANFSMB "nanfasi"
# define   CHAR_NVFASI1 "nvfasi1"
# define   CHAR_NANSROK "nanmosi"
# define   CHAR_NVSHROK "nvmoshi"

//! 几个主角人物对应的目录
s_string g_szHeroDir[6] = 
{
CHAR_NANZANS,  //男战士
CHAR_NVZANSI,  //女战士
CHAR_NANFSMB,  //男法师
CHAR_NVFASI1,  //女法师
CHAR_NANSROK,  //男牧师
CHAR_NVSHROK   //女牧师
};


CItemShape::CItemShape()
{
	m_pItemShapeDisplay = NULL;
}

CItemShape::~CItemShape()
{
	SAFE_DELETE( m_pItemShapeDisplay );
}


bool CItemShape::is_itemShapeDisplay( void )
{
	if( m_szItemEquFileName[0] == NULL )
		return false;
	else
		return true;	
}

void CItemShape::load_itemFromMem( BYTE*&  _ds )
{
	guard;

	osassert( _ds );

	// 读入装备的名字
	READ_MEM_OFF( m_szItemName,_ds,sizeof( char )*MAX_ITEMNAMELEN );
	// 根据职业，性别，和装备ID生成的唯一标识索引
	READ_MEM_OFF( &m_dwObjectID,_ds,sizeof( DWORD ) );
	// 读入装备的ID
	READ_MEM_OFF( &m_dwShapeId,_ds,sizeof( DWORD ) );
	// 装备对应图片文件名
	READ_MEM_OFF( m_szEquipPic,_ds,MAX_EQUIPDIRLEN );
	// 当前装备对应的文件名，存储相对目录。
	READ_MEM_OFF( m_szItemEquFileName,_ds,MAX_EQUIPDIRLEN );
	
	// 特效文件名字
	READ_MEM_OFF( m_szSpe, _ds, MAX_ITEMNAMELEN );
	// 图标ID
	READ_MEM_OFF( &m_dwIconId, _ds, sizeof( DWORD ) );

	unguard;
}

ITEMSHAPE_DISPLAY* CItemShape::get_itemShapeDisplay( void )
{
	guard;

	if( !is_itemShapeDisplay() )
		return NULL;

	if( !m_pItemShapeDisplay )
	{
		int     t_size, ver;
		BYTE*   t_fstart;
		char    t_str[4];

		if( m_szItemEquFileName[0] == NULL )
		{
			osassertex( false,va( "EQU:此物品<%d>无equ文件!\n", m_dwShapeId ) );
			return NULL;
		}

		if( !file_exist( m_szItemEquFileName ) ) 
		{
			osDebugOut( "EQU:文件不存在!<%s>\n", m_szItemEquFileName );
			return NULL;
		}

		int t_iGBufIdx;
		t_fstart = START_USEGBUF( t_iGBufIdx );
		t_size = read_fileToBuf( m_szItemEquFileName, t_fstart, TMP_BUFSIZE );
		osassert( t_size >= 0 );

		// 读入文件头.
		READ_MEM_OFF( t_str,t_fstart,sizeof( char )*4 );
		if( strcmp( t_str, EQUIPLIST_MAGIC ) )
			osassert( false );

		READ_MEM_OFF( &ver, t_fstart, sizeof( int ) );

		SAFE_DELETE( m_pItemShapeDisplay );
		m_pItemShapeDisplay = new ITEMSHAPE_DISPLAY;

		if( ver < 104 )
		{
			READ_MEM_OFF( m_pItemShapeDisplay->m_szItemName,t_fstart,MAX_EQUIPDIRLEN );
			READ_MEM_OFF( &t_size, t_fstart, sizeof( int ) );
		}

		// 读入动作相关数据。
		if( ver < 102 )
		{
			READ_MEM_OFF( &m_pItemShapeDisplay->m_iCActNum,t_fstart,sizeof( int ) );
			osassert( m_pItemShapeDisplay->m_iCActNum >= 0 );
		}

		// 读入人物身体部位替换的相关数据。
		READ_MEM_OFF( &m_pItemShapeDisplay->m_iCPartNum,t_fstart,sizeof( int ) );
		osassert( m_pItemShapeDisplay->m_iCPartNum >= 0 );
		t_size = sizeof( int ) * m_pItemShapeDisplay->m_iCPartNum;
		READ_MEM_OFF( m_pItemShapeDisplay->m_arrId,t_fstart,t_size );
		READ_MEM_OFF( m_pItemShapeDisplay->m_arrMesh,t_fstart,t_size );
		READ_MEM_OFF( m_pItemShapeDisplay->m_arrSkin,t_fstart,t_size );

		END_USEGBUF( t_iGBufIdx );

		if( ver < 103 )
		{
			for( int t_i=0;t_i<m_pItemShapeDisplay->m_iCPartNum;t_i ++ )
			{
				m_pItemShapeDisplay->m_arrMesh[t_i]++;
				m_pItemShapeDisplay->m_arrSkin[t_i]++;
			}
		}
	}

	return m_pItemShapeDisplay;

	unguard;
}

//! 得到此装备的job和sex信息
int CItemShape::get_itemJobSexInfo( void )
{
	guard;

	m_dwObjectID ;
	m_dwShapeId;

	return m_dwObjectID;

	unguard;
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////////////////////////
//! 初始化静态变量
template<> gc_itemMgr* ost_Singleton<gc_itemMgr>::m_ptrSingleton = NULL;


gc_itemMgr::gc_itemMgr()
{
	m_iEquipNum = 0;
	m_sEquipMap.clear();
}

gc_itemMgr::~gc_itemMgr()
{

}

/** 得到一个sg_timer的Instance指针.
*/
gc_itemMgr* gc_itemMgr::Instance( void )
{
	if( !gc_itemMgr::m_ptrSingleton )
	{

		gc_itemMgr::m_ptrSingleton = new gc_itemMgr;
		
		osassert( gc_itemMgr::m_ptrSingleton );
	}
	
	return gc_itemMgr::m_ptrSingleton;
}

/** 删除一个sg_timer的Instance指针.
*/
void gc_itemMgr::DInstance( void )
{
	SAFE_DELETE( gc_itemMgr::m_ptrSingleton );
}

# if 0
//! 得到主键ID
DWORD CItemShapeMgr::get_objectIdBy( DWORD _looksId, /*物品外观索引*/ BYTE _job, BYTE _sex )
{
	guard;
	osassert( _looksId >= 0 );
	osassert( _job >= 0 && _job < 7 );
	osassert( _sex >= 0 && _sex < 3 );
	DWORD dwObjectId = 0;
	dwObjectId = (((DWORD)(_job))<<29)|(((DWORD)(_sex))<<27)|_looksId;
	return dwObjectId;
	unguard;
}
# endif 


//! 全局的初始化当前的道具（装备）管理器
bool gc_itemMgr::init_itemMgr( void )
{
	guard;

	int       t_iSize;
	char      t_szStr[4];
	BYTE*     t_fstart;
	bool*     t_boolBuf = NULL;

	if( !file_exist( GLOBAL_EQUIPLIST ) )
	{
		osassertex( false,va( "打开文件<%s>失败..",GLOBAL_EQUIPLIST ) );
		return false;
	}

	int t_iGBufIdx;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_iSize = read_fileToBuf( GLOBAL_EQUIPLIST,t_fstart,TMP_BUFSIZE );
	osassert( t_iSize >= 0 );

	READ_MEM_OFF( t_szStr,t_fstart,sizeof( char )*4 );
	if( strcmp( t_szStr,EQUIPLIST_MAGIC ) )
	{
		osassert( false );
		return false;
	}

	READ_MEM_OFF( &t_iSize,t_fstart,sizeof( int ) );
	if( t_iSize < EQUIPLIST_FVERSION )
	{
		MessageBox( NULL, "装备文件版本太老!", "", MB_OK );
		osassert( false );
	}

	READ_MEM_OFF( &m_iEquipNum, t_fstart,sizeof( int ) );
	osassert( m_iEquipNum >= 0 );
	osassert( m_iEquipNum < 65535 );

	t_boolBuf = new bool[m_iEquipNum];
	READ_MEM_OFF( t_boolBuf, t_fstart,sizeof( bool )*m_iEquipNum );
	for( int i=0;i<m_iEquipNum;i++ )  
	{
		if( t_boolBuf[i] )
		{
			CItemShape equip;
			int        t_iKey;
			equip.load_itemFromMem( t_fstart );

			if( equip.get_itemEquFileName()[0] == NULL )
				continue;

			t_iKey = equip.get_objectId();
			m_sEquipMap.insert( std::make_pair( t_iKey,equip ) );

		}
	}
	END_USEGBUF( t_iGBufIdx );
	SAFE_DELETE_ARRAY( t_boolBuf );

	// 调入标准装备
	m_sStdItemMgr.LoadFile( STD_ITEMINFOFILE );


	return true;

	unguard;
}


//! 传入一个人物的目录名，确认此目录是否可以改变装备，只有主角人物所在的目录可以改变装备
bool gc_itemMgr::can_changeEquip( const char* _cdir )
{
	guard;

	for( int t_i=0;t_i<6;t_i ++ )
	{
		if( g_szHeroDir[t_i] == _cdir )
			return true;
	}

	return false;

	unguard;
}

//! 从一个目录名，得到job和sexID.
void gc_itemMgr::get_jobSexId( const char* _cdir,int& _job,int& _sex )
{
	guard;

	int   t_idx = -1;
	for( int t_i=0;t_i<6;t_i ++ )
	{
		if( g_szHeroDir[t_i] == _cdir )
		{
			t_idx = t_i;
			break;
		}
	}

	if( t_idx == -1 )
	{
		_job = 0;_sex = 0;
	}
	else
	{
		_job = t_i/2 + 1;
		_sex = t_i%2 + 1;
	}

	return;

	unguard;
}



//! 根据一个装备的名字，得到此装备需要切换的身体部位数目
bool gc_itemMgr::get_equipFromName( const char* _cdir,const char* _name,os_equipment& _equip )
{
	guard;

	// 先得到此装备的shapeId,然后再根据人物的目录名，再处理得到一个最后的objectId
	int   t_iShapeId = m_sStdItemMgr.get_itemShapeId( _name );
	if( t_iShapeId == -1 )
		return false;
	int   t_iJob,t_iSex,t_iKey;
	get_jobSexId( _cdir,t_iJob,t_iSex );
	t_iKey = (((DWORD)(t_iJob))<<29)|(((DWORD)(t_iSex))<<27)|t_iShapeId;

	MAP_equipment::iterator   t_iter;

	t_iter = m_sEquipMap.find( t_iKey );
	if( t_iter == m_sEquipMap.end() )
		return false;
	else
	{
		if( t_iter->second.is_itemShapeDisplay() )
		{
			ITEMSHAPE_DISPLAY*   t_ptrDis;
			t_ptrDis = t_iter->second.get_itemShapeDisplay();

			_equip.m_iCPartNum = t_ptrDis->m_iCPartNum;
			for( int t_i=0;t_i<_equip.m_iCPartNum;t_i ++ )
			{
				_equip.m_arrId[t_i] = t_ptrDis->m_arrId[t_i];
				_equip.m_arrMesh[t_i] = t_ptrDis->m_arrMesh[t_i];
				_equip.m_arrSkin[t_i] = t_ptrDis->m_arrSkin[t_i];
			}
		}
	}

	return true;

	unguard;
}


