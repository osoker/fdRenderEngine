///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_itemShapeMgr.cpp
 *
 *  Desc:     ���ڵ����������е�װ�����֣�����������ȷ���л�����װ�������岿λ
 * 
 *  His:      River created @ 2006-8-1 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "gc_itemShapeMgr.h"

//! ȫ��װ���ļ��İ汾��
#define EQUIPLIST_FVERSION		 300

//! ��׼װ�����ļ���
# define STD_ITEMINFOFILE      "equipment\\StdItem.txt"

// ְҵ����
# define   CAREER_COMMON  0x00
# define   CAREER_FIGHTER 0x01
# define   CAREER_RABBI   0x02
# define   CAREER_PRIEST  0x03
// �Ա����
# define   SEX_COMMON 0x00
# define   SEX_MALE   0x01
# define   SEX_FEMALE 0x02

// ÿһ����ɫ��Ӧ��Ŀ¼
# define   CHAR_NANZANS "nanzhan"
# define   CHAR_NVZANSI "nvzhans"
# define   CHAR_NANFSMB "nanfasi"
# define   CHAR_NVFASI1 "nvfasi1"
# define   CHAR_NANSROK "nanmosi"
# define   CHAR_NVSHROK "nvmoshi"

//! �������������Ӧ��Ŀ¼
s_string g_szHeroDir[6] = 
{
CHAR_NANZANS,  //��սʿ
CHAR_NVZANSI,  //Ůսʿ
CHAR_NANFSMB,  //�з�ʦ
CHAR_NVFASI1,  //Ů��ʦ
CHAR_NANSROK,  //����ʦ
CHAR_NVSHROK   //Ů��ʦ
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

	// ����װ��������
	READ_MEM_OFF( m_szItemName,_ds,sizeof( char )*MAX_ITEMNAMELEN );
	// ����ְҵ���Ա𣬺�װ��ID���ɵ�Ψһ��ʶ����
	READ_MEM_OFF( &m_dwObjectID,_ds,sizeof( DWORD ) );
	// ����װ����ID
	READ_MEM_OFF( &m_dwShapeId,_ds,sizeof( DWORD ) );
	// װ����ӦͼƬ�ļ���
	READ_MEM_OFF( m_szEquipPic,_ds,MAX_EQUIPDIRLEN );
	// ��ǰװ����Ӧ���ļ������洢���Ŀ¼��
	READ_MEM_OFF( m_szItemEquFileName,_ds,MAX_EQUIPDIRLEN );
	
	// ��Ч�ļ�����
	READ_MEM_OFF( m_szSpe, _ds, MAX_ITEMNAMELEN );
	// ͼ��ID
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
			osassertex( false,va( "EQU:����Ʒ<%d>��equ�ļ�!\n", m_dwShapeId ) );
			return NULL;
		}

		if( !file_exist( m_szItemEquFileName ) ) 
		{
			osDebugOut( "EQU:�ļ�������!<%s>\n", m_szItemEquFileName );
			return NULL;
		}

		int t_iGBufIdx;
		t_fstart = START_USEGBUF( t_iGBufIdx );
		t_size = read_fileToBuf( m_szItemEquFileName, t_fstart, TMP_BUFSIZE );
		osassert( t_size >= 0 );

		// �����ļ�ͷ.
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

		// ���붯��������ݡ�
		if( ver < 102 )
		{
			READ_MEM_OFF( &m_pItemShapeDisplay->m_iCActNum,t_fstart,sizeof( int ) );
			osassert( m_pItemShapeDisplay->m_iCActNum >= 0 );
		}

		// �����������岿λ�滻��������ݡ�
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

//! �õ���װ����job��sex��Ϣ
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
//! ��ʼ����̬����
template<> gc_itemMgr* ost_Singleton<gc_itemMgr>::m_ptrSingleton = NULL;


gc_itemMgr::gc_itemMgr()
{
	m_iEquipNum = 0;
	m_sEquipMap.clear();
}

gc_itemMgr::~gc_itemMgr()
{

}

/** �õ�һ��sg_timer��Instanceָ��.
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

/** ɾ��һ��sg_timer��Instanceָ��.
*/
void gc_itemMgr::DInstance( void )
{
	SAFE_DELETE( gc_itemMgr::m_ptrSingleton );
}

# if 0
//! �õ�����ID
DWORD CItemShapeMgr::get_objectIdBy( DWORD _looksId, /*��Ʒ�������*/ BYTE _job, BYTE _sex )
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


//! ȫ�ֵĳ�ʼ����ǰ�ĵ��ߣ�װ����������
bool gc_itemMgr::init_itemMgr( void )
{
	guard;

	int       t_iSize;
	char      t_szStr[4];
	BYTE*     t_fstart;
	bool*     t_boolBuf = NULL;

	if( !file_exist( GLOBAL_EQUIPLIST ) )
	{
		osassertex( false,va( "���ļ�<%s>ʧ��..",GLOBAL_EQUIPLIST ) );
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
		MessageBox( NULL, "װ���ļ��汾̫��!", "", MB_OK );
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

	// �����׼װ��
	m_sStdItemMgr.LoadFile( STD_ITEMINFOFILE );


	return true;

	unguard;
}


//! ����һ�������Ŀ¼����ȷ�ϴ�Ŀ¼�Ƿ���Ըı�װ����ֻ�������������ڵ�Ŀ¼���Ըı�װ��
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

//! ��һ��Ŀ¼�����õ�job��sexID.
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



//! ����һ��װ�������֣��õ���װ����Ҫ�л������岿λ��Ŀ
bool gc_itemMgr::get_equipFromName( const char* _cdir,const char* _name,os_equipment& _equip )
{
	guard;

	// �ȵõ���װ����shapeId,Ȼ���ٸ��������Ŀ¼�����ٴ���õ�һ������objectId
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


