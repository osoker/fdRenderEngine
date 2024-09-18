///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_globalMData.cpp
 *
 *  Desc:     ������������id,id��ȫ����Ϣ
 * 
 *  His:      River created @ 2007-3-9
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

# include "gc_globalMData.h"

//! ��ʼ����̬����
template<> gc_monsterAppearance* ost_Singleton<gc_monsterAppearance>::m_ptrSingleton = NULL;

gc_monsterAppearance::gc_monsterAppearance()
{

}

gc_monsterAppearance::~gc_monsterAppearance()
{

}




/** �õ�һ��gc_monsterAppearance��Instanceָ��.
*/
gc_monsterAppearance* gc_monsterAppearance::Instance( void )
{
	guard;

	if( m_ptrSingleton )
		return m_ptrSingleton;
	
	m_ptrSingleton = new gc_monsterAppearance();
	return m_ptrSingleton;


	unguard;
}

/** ɾ��һ��sg_timer��Instanceָ��.
*/
void gc_monsterAppearance::DInstance( void )
{
	guard;

	SAFE_DELETE( m_ptrSingleton );

	unguard;
}

//! ��id�õ������������Ϣ
const LOOKSINFO* gc_monsterAppearance::get_mLooksinfoFromId( int _id,float& _scale )
{
	guard;

	for( int t_i=0;t_i<(int)m_vecMLInfo.size();t_i ++ )
	{
		int t_id = m_vecMLInfo[t_i].looksId;
		if( t_id == _id )
		{
			_scale = m_vecMLInfo[t_i].scale;
			return &m_vecMLInfo[t_i];
		}
	}

	char   t_sz[128];
	sprintf( t_sz,"�Ҳ���Looks ID:<%d>..\n",_id );
	MessageBox( NULL,t_sz,"������",MB_OK );

	return NULL;

	unguard;
}


//! ������������id�͹����Ŀ¼֮��Ķ�Ӧ��Ϣ
BOOL gc_monsterAppearance::load_globalMaInfo( const char* _fname )
{
	guard;

	int     size = 0;
	BYTE*   p = 0;
	char    str[4] = {0};
	int     count = 0;

	//! �Ѿ������
	if( m_vecMLInfo.size() > 0 )
		return TRUE;

	if( !file_exist( (char*)_fname ) )
	{
		osassertex( false,va( "���ļ�<%s>ʧ��...\n",_fname ) );
		return false;
	}

	int t_iGBufIdx;
	p = START_USEGBUF( t_iGBufIdx );
	size = read_fileToBuf( (char*)_fname, p, TMP_BUFSIZE );
	osassert( size >= 0 );
	
	READ_MEM_OFF( str, p, sizeof( char ) * 4 );
	if( strcmp( str, "mol" ) )
	{
		osassert( false );
		return false;
	}
	READ_MEM_OFF( &size, p, sizeof( int ) );
	osassertex( size == 400,va( "<%s>�������µİ汾...\n",_fname ) );

	READ_MEM_OFF( &count, p, sizeof( int ) );
	osassert( count > 0 );
	m_vecMLInfo.resize( count );

	
	LOOKSINFO   t_look;
	for( int t_i=0;t_i<count;t_i ++ )
	{
		READ_MEM_OFF( &m_vecMLInfo[t_i], p, sizeof( LOOKSINFO ) );
		memcpy( &t_look,&m_vecMLInfo[t_i],sizeof( LOOKSINFO ) );
	}
	END_USEGBUF( t_iGBufIdx );

	return TRUE;

	unguard;
}






//! ��ʼ����̬����
template<> gc_monsterIdNameMgr* ost_Singleton<gc_monsterIdNameMgr>::m_ptrSingleton = NULL;
gc_monsterIdNameMgr::gc_monsterIdNameMgr()
{

}

gc_monsterIdNameMgr::~gc_monsterIdNameMgr()
{

}


/** �õ�һ��gc_monsterIdNameMgr��Instanceָ��.
*/
gc_monsterIdNameMgr* gc_monsterIdNameMgr::Instance( void )
{
	if( m_ptrSingleton )
		return m_ptrSingleton;
	
	m_ptrSingleton = new gc_monsterIdNameMgr();
	return m_ptrSingleton;

}


/** ɾ��һ��sg_timer��Instanceָ��.
*/
void gc_monsterIdNameMgr::DInstance( void )
{
	SAFE_DELETE( m_ptrSingleton );
}

#define FILE_MSTHEADER             "mst"
#define FILE_VERSION_MONST         2
//! ����ȫ���Ĺ�������
BOOL gc_monsterIdNameMgr::init_monstData( const char* _fname )
{
	guard;

	osassert( _fname );
	osassert( _fname[0] );

	DWORD       t_dw;
	char        t_sz[4];
	char        t_info[256];
	BYTE*       t_fstart = NULL;

	int			t_size;

	// �Ѿ����������Ϣ
	if( m_vecMonstData.size() > 0 )
		return TRUE;

	m_szFileName = _fname;

	int t_iGBufIdx;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( (char*)_fname, t_fstart, TMP_BUFSIZE );
	osassert( t_size >= 0 );

	READ_MEM_OFF( t_sz, t_fstart, sizeof(char)*4 );
	if( strcmp( t_sz, FILE_MSTHEADER ) != 0 )
	{
		END_USEGBUF( t_iGBufIdx );
		osassert( false );
		return FALSE;
	}
	READ_MEM_OFF( &t_dw, t_fstart, sizeof( DWORD ) );
	if( t_dw != FILE_VERSION_MONST )
	{
		END_USEGBUF( t_iGBufIdx );
		sprintf( t_info, "<%s>�ļ��汾����!", _fname );
		osDebugOut( t_info );
		MessageBox( NULL, t_info, "�ļ��汾����", MB_OK );
		osassert( false );
		return FALSE;
	}
	READ_MEM_OFF( &t_dw, t_fstart, sizeof( DWORD ) );
	m_vecMonstData.resize( t_dw );
	READ_MEM_OFF( &m_vecMonstData[0], t_fstart, sizeof( SMonstData )*t_dw );
	END_USEGBUF( t_iGBufIdx );

	return TRUE;

	unguard;
}

//! �洢�޸ĺ�Ĺ�����Ϣ
void gc_monsterIdNameMgr::save_monsterData( void )
{
	guard;

	FILE*        t_file;

	t_file = fopen( m_szFileName.c_str(),"wb" );
	if( !t_file )
		MessageBox( NULL,"���ļ�ʧ��","ERROR",MB_OK );

	char    t_sz[4];
	DWORD   t_dw;

	strcpy( t_sz,FILE_MSTHEADER );
	t_sz[3] = NULL;
	fwrite( t_sz,sizeof( char ),4,t_file );
	t_dw = FILE_VERSION_MONST;
	fwrite( &t_dw,sizeof( DWORD ),1,t_file );

	// д����������.
	t_dw = (int)m_vecMonstData.size();
	fwrite( &t_dw,sizeof( DWORD ),1,t_file );

	for( int t_i=0;t_i<(int)t_dw;t_i ++ )
		fwrite( &m_vecMonstData[t_i],sizeof( SMonstData ),1,t_file );

	fclose( t_file );

	char     t_szInfo[256];
	sprintf( t_szInfo,"�����ļ�<%s>�ɹ�",this->m_szFileName.c_str() );
	MessageBox( NULL,t_szInfo,"����ɹ�",MB_OK );

	return ;

	unguard;
}


//! �ӹ����id�õ������Ӧ��Ŀ¼��
const char* gc_monsterIdNameMgr::get_monstDirFromId( int _id,
      DWORD& _visualField,DWORD& _trigger,DWORD& _escape,DWORD& _callDis,float& _scale )
{
	guard;

	const LOOKSINFO*   t_ptrInfo;
	
	for( int t_i=0;t_i<(int)m_vecMonstData.size();t_i ++ )
	{
		if( m_vecMonstData[t_i].m_dwMonstId == 0 )
			int t_m = 0;
		if( m_vecMonstData[t_i].m_dwMonstId == _id )
		{
			float   t_fScale = 1.0f;
			t_ptrInfo = gc_monsterAppearance::Instance()->
				get_mLooksinfoFromId( m_vecMonstData[t_i].m_dwMonstAppearCode,t_fScale );

			if( t_ptrInfo == NULL ) 
				return NULL;
			else
			{
				_visualField = m_vecMonstData[t_i].m_dwZhuDongShiYe;
				_trigger = m_vecMonstData[t_i].m_unAiData.m_sMonstAiData.m_dwTriggerDis;
				_escape = m_vecMonstData[t_i].m_dwTaoPaoBanJing;
				_callDis = m_vecMonstData[t_i].m_dwHuJiaoDistance;
				_scale = t_fScale;

				return t_ptrInfo->dir;		
			}
		}
	}

	char   t_sz[256];
	sprintf( t_sz,"�Ҳ�������ID <%d>..\n",_id );
	MessageBox( NULL,t_sz,"������",MB_OK );

	return NULL;

	unguard;
}

SMonstData* gc_monsterIdNameMgr::get_monsterFromIdx( int _idx )
{
	guard;

	osassert( _idx < (int)m_vecMonstData.size() );
	return &m_vecMonstData[_idx];

	unguard;
}

//! ���ݹ����id,�õ��˹����������������ڵ�����
int gc_monsterIdNameMgr::get_monsterIdxFromId( int _id )
{
	guard;

	for( int t_i=0;t_i<(int)m_vecMonstData.size();t_i ++ )
	{
		if( m_vecMonstData[t_i].m_dwMonstId == _id )
			return t_i;
	}

	return -1;

	unguard;
}

//! ���ù�����ƶ�����,���׼���
void gc_monsterIdNameMgr::set_monsterStepLength( int _idx,float _length )
{
	guard;

	return;
	unguard;
}
//! ���ù���������ƶ����.�������.
void gc_monsterIdNameMgr::set_monsterNormalMoveTime( int _idx,float _time )
{
	guard;


	return;
	unguard;
}
//! ���ù����׷���ƶ����.�������.
void gc_monsterIdNameMgr::set_monsterAttackMoveTime( int _idx,float _time )
{
	guard;

	return;

	unguard;
}

