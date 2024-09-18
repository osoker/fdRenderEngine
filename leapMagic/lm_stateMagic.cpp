///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_stateMagic.h
 *
 *  Desc:     魔法demo客户端用到的人物状态相关数据，用于加入人物状态后显示特效
 * 
 *  His:      River created @ 2006-8-7
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

#include "lm_stateMagic.h"


CStateIconMgr::CStateIconMgr() :
m_iFadeStateID(-1),
m_fFadeStateAlpha(1.f)
{
	guard;
	unguard;
}

CStateIconMgr::~CStateIconMgr()
{
	guard;
	MAP_STATEDATA::iterator it = m_mapStateData.begin();
	while( it != m_mapStateData.end() )
	{
		for( int i=0;i<(int)it->second.m_dwDescCount;i++ )
		{
			delete [] it->second.m_pDispDesc[i];
		}
		delete [] it->second.m_pDispDesc;
		++it;
	}
	
	unguard;
}

//初始化管理器数据
bool CStateIconMgr::init()
{
	guard;

	int			t_size;
	int			t_iFileSize;
	BYTE*		t_fstart;
	char  szFileFlag[4];
	DWORD dwVersion, dwStateCnt;
    STATEDATA s;
	if( !file_exist( (char*)STATECONFILENAME ) )
	{
		osassertex( false,va( "<%s>文件不存在...\n",STATECONFILENAME) );
		return false;
	}

	t_iFileSize = get_fileSize( (char*)STATECONFILENAME );
	int t_iGBufIdx;
	t_fstart = START_USEGBUF( t_iGBufIdx );
	t_size = read_fileToBuf( (char*)STATECONFILENAME, t_fstart, t_iFileSize );
	osassert( t_size >= 0 );

	// 文件标识和版本号
	READ_MEM_OFF( &szFileFlag, t_fstart, sizeof(char)*4 );
	READ_MEM_OFF( &dwVersion, t_fstart, sizeof(DWORD) );
	if( strcmp(szFileFlag,STATEFILE_FLAG) != 0 )
	{
		osassert( false );
		return false;
	}
	// 隐身
	if( dwVersion >= 0x02 )
	{
		READ_MEM_OFF( &m_iFadeStateID, t_fstart, sizeof(int) );
		READ_MEM_OFF( &m_fFadeStateAlpha, t_fstart, sizeof(float) );
	}
	// 个数
	READ_MEM_OFF( &dwStateCnt, t_fstart, sizeof(DWORD) );
	// 数据
	m_mapStateData.clear();

	int   t_iHashId;
	for( DWORD i=0; i<dwStateCnt; i++ )
	{
		if( dwVersion == 6 )
		{
			READ_MEM_OFF( &s, t_fstart,sizeof(STATEDATA) );

			if( s.m_dwDescCount > 0 )
			{
				s.m_pDispDesc = new char*[s.m_dwDescCount];
				for ( int j=0; j<(int)s.m_dwDescCount; j++ )
				{
					s.m_pDispDesc[j] = new char[64];
					READ_MEM_OFF( s.m_pDispDesc[j], t_fstart,sizeof(char)*64 );
				}
			}
			else
			{
				s.m_pDispDesc = NULL;
			}

			t_iHashId = string_hash( s.m_szStateName );

			m_mapStateData.insert( std::make_pair( t_iHashId,s ) );
		}
		else
			osassert( false );

	}
	END_USEGBUF( t_iGBufIdx );

	return true;
	unguard;
}

const char* CStateIconMgr::getStateSpeffName( const char* _sname )
{
	guard;

	osassert( _sname );
	if( _sname[0] == NULL )
		return NULL;
	int t_iHashId = string_hash( _sname );
	MAP_STATEDATA::iterator it = m_mapStateData.find( t_iHashId );
	if( it!=m_mapStateData.end() )
	{
		if( it->second.m_szStateEffect[0] )
			return it->second.m_szStateEffect;
		else
			return NULL;
	}
	else
	{
		// 使用索引来查找我们要播放的state
		int t_idx = atoi( _sname );

		MAP_STATEDATA::iterator   t_iter;
		int  t_sidx = 0;
		for( t_iter = m_mapStateData.begin();
			t_iter != m_mapStateData.end();t_iter ++ )
		{
			osDebugOut( "The id is:<%d>,name is:<%s>..\n",
				t_iter->second.m_iStateID,t_iter->second.m_szStateEffect );

			if( t_iter->second.m_iStateID == t_idx )
			{
				if( t_iter->second.m_szStateEffect[0] )
					return t_iter->second.m_szStateEffect;
				else
					return NULL;			
			}
		}
	}
	
	return NULL;

	unguard;
}
