//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: tgObjoadThread.cpp
 *
 *  His:      River created @ 2004-9-25
 *
 *  Des:      �����ͼ�ϵ���Ʒʱ��ʹ�ö��̡߳�
 *  
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"


# include "../../backPipe/include/osCamera.h"

# include  "../../mfpipe/include/osMesh.h"

# include "../../backpipe/include/mainEngine.h"



//! TEST CODE:��ǰ�Ƿ��ڶ��̵߳�ͼ����ʱ��
OSENGINE_API BOOL     g_bMapLoadding = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ���̵߳ĵ�ͼ���������.
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! ȫ��ʹ�õ�critical_section.
CRITICAL_SECTION osc_mapLoadThread::m_sMapNameCS;


//! �ٽ紴������ʱ�õ�����Դ.
CRITICAL_SECTION osc_mapLoadThread::m_sParticleCS;

//! �ٽ紴��meshʱ�õ�����Դ.
CRITICAL_SECTION osc_mapLoadThread::m_sMeshCS;

//! �ٽ紴��osaʱ�õ�����Դ.
CRITICAL_SECTION osc_mapLoadThread::m_sOsaCS;

//! �ٽ紴����Ч�õ�����Դ
CRITICAL_SECTION osc_mapLoadThread::m_sEffectCS;

//! �����ڻ��ͼ�б����ʱ���õ����ٽ�����Դ���ϲ�����õ���ص���Ϣ
CRITICAL_SECTION osc_mapLoadThread::m_sActiveMapCS;


//! �ٽ紴��billboard�õ�����Դ.
CRITICAL_SECTION osc_mapLoadThread::m_sBbCS;


//! �ٽ�shader�����õ�����Դ.
CRITICAL_SECTION osc_mapLoadThread::m_sShaderCS;

//! ����Effect��Ҫ��CS.
CRITICAL_SECTION osc_mapLoadThread::m_sDxEffectCS;


//! �ٽ�shader�����õ�����Դ.
CRITICAL_SECTION osc_mapLoadThread::m_sShaderReleaseCS;


//! �Ƿ��ѵ�ͼ�����̵߳�event.
HANDLE osc_mapLoadThread::m_sEventStartMt = NULL;

//! ��ͼ�����߳��Ƿ��ڿ���״̬���ڵȴ��̱߳����ѵ���Ϣ
HANDLE osc_mapLoadThread::m_sEventThreadFree = NULL;


osc_mapLoadThread::osc_mapLoadThread()
{
	::InitializeCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	//! �ٽ紴������ʱ�õ�����Դ.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	//! �ٽ紴��meshʱ�õ�����Դ.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sMeshCS );

	//! �ٽ紴��osaʱ�õ�����Դ.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	//! �ٽ紴��billboard�õ�����Դ.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sBbCS );

	//! �ٽ紴��billboard�õ�����Դ.
	::InitializeCriticalSection( &osc_mapLoadThread::m_sShaderCS );

	::InitializeCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );

	::InitializeCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );

	::InitializeCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	//! �����ڻ��ͼ�б����ʱ���õ����ٽ�����Դ���ϲ�����õ���ص���Ϣ
	::InitializeCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );


	m_ptrLRMgr = NULL;
	m_ptrTGMgr = NULL;

	m_sEventStartMt = ::CreateEvent( 
		NULL,
		TRUE,
		FALSE,  // ��ʼ��Ϊ���ź�.
		NULL );

	osassert( m_sEventStartMt );

	m_sEventMapLoadFinish = ::CreateEvent(
		NULL,
		TRUE,
		FALSE, // ��ʼ��Ϊ���ź�.
		NULL );
	osassert( m_sEventMapLoadFinish );
	::ResetEvent( m_sEventMapLoadFinish );


	// �߳��˳�
	m_sEventExitThread = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// ��ʼ��Ϊ���ź�
		NULL );
	osassert( m_sEventExitThread );

	//! ��ͼ�����߳��Ƿ��ڿ���״̬���ڵȴ��̱߳����ѵ���Ϣ
	m_sEventThreadFree = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// ��ʼ��Ϊ���ź�
		NULL );
	osassert( m_sEventThreadFree );

}

osc_mapLoadThread::~osc_mapLoadThread()
{

	::DeleteCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	//! �ٽ紴������ʱ�õ�����Դ.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sParticleCS );

	//! �ٽ紴��meshʱ�õ�����Դ.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sMeshCS );

	//! �ٽ紴��osaʱ�õ�����Դ.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sOsaCS );

	//! �ٽ紴��billboard�õ�����Դ.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sBbCS );

	//! �ٽ紴��billboard�õ�����Դ.
	::DeleteCriticalSection( &osc_mapLoadThread::m_sShaderCS );
	::DeleteCriticalSection( &osc_mapLoadThread::m_sDxEffectCS );

	::DeleteCriticalSection( &osc_mapLoadThread::m_sShaderReleaseCS );

	::DeleteCriticalSection( &osc_mapLoadThread::m_sEffectCS );

	::DeleteCriticalSection( &osc_mapLoadThread::m_sActiveMapCS );


	//! 
	if( m_sEventStartMt )
		CloseHandle( m_sEventStartMt );
	if( m_sEventMapLoadFinish )
		::CloseHandle( m_sEventMapLoadFinish );

	if( m_sEventThreadFree )
		::CloseHandle( m_sEventThreadFree );

	//syq
	if( m_sEventExitThread )
		::CloseHandle( m_sEventExitThread );

}

//! �����ڲ�ʹ�õ�ָ��
void osc_mapLoadThread::init_mtLoadMapMgr( 
	osc_TGManager* _mgr,osc_tgLoadReleaseMgr* _lrMgr )
{
	guard;

	osassert( _mgr );
	osassert( _lrMgr );

	m_ptrTGMgr = _mgr;
	this->m_ptrLRMgr = _lrMgr;



	unguard;
}


/** \brief
*  �Ƿ��ǿɲ�����TG,����ʹ�õ�tg.
*
*  �ڵ�ͼ�����̵߳��û���ͼ������û�е�ǰtg.���tg����activeTg,
*  Ϊ�ϲ���Ⱦ��Ҫʹ�õ���tg.
*/
bool osc_mapLoadThread::is_activeTg( int _idx )
{
	guard;

	bool   t_bRes = true;
	std::list< os_tgMTLoader >::iterator     t_iter;

	osassert( _idx < m_ptrLRMgr->m_iCurTgNum );

	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	// 
	// �����ǰtg��ʹ��״̬�ͷǡ��Ŷӵ��롱״̬����鿴�û����������û�У��򷵻�Ϊactive.
	// ���tg����ʹ��״̬����ֱ�ӷ��ط�active.
	if( m_ptrLRMgr->m_ptrRealTg[_idx].is_tgInuse() &&
		(!m_ptrLRMgr->m_ptrRealTg[_idx].get_mtQueueState()) )
	{
		// ���̶߳����д��ڴ�tg,Ϊ���ɲ���tg.
		for( t_iter = m_sMapNeedLoad.begin();
			t_iter != m_sMapNeedLoad.end();t_iter ++ )
		{
			if( t_iter->m_iMapIdx == _idx )
			{
				t_bRes = false;
				break;
			}
		}

		// Ҫ����ĵ�ͼ�������ڵ���ĵ�ͼ�б���.
		if( t_iter == m_sMapNeedLoad.end() )
			t_bRes = true;
	}
	else
		t_bRes = false;

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	return t_bRes;

	unguard;
}


//! �����̵߳�ִ��
void osc_mapLoadThread::end_thread( void )
{
	guard;

	// ʹ�õ�ͼ���봦�ڵȴ�״̬.
	reset_loadMapMgr();

	// 
	// �ȼ����ͼ�����̣߳��ټ����˳��߳��¼��������ͼ����
	// ����Ϊ�գ�����ʱ�˳��߳��¼����źţ����˳��̡߳�
	::SetEvent( osc_mapLoadThread::m_sEventStartMt );
	::SetEvent( m_sEventExitThread );

	unguard;
}



//! ��ǰ�Ƿ�������״̬,���ȴ����߳��ƽ�Ҫ����ĵ�ͼ.
bool osc_mapLoadThread::is_freeStatus( void )
{
	guard;

	DWORD    t_dwWait;
	bool     t_bRes;

	// FreeStatus״̬���˲����ڵ���״̬�⣬����û����Ҫ����ĵ�ͼ��
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	if( 0 == m_sMapNeedLoad.size() )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
		
		t_dwWait = ::WaitForSingleObject( m_sEventStartMt,0 );
		if( t_dwWait == WAIT_OBJECT_0 )
		{
			t_bRes = false;
		}
		else if( t_dwWait == WAIT_TIMEOUT )
			t_bRes = true;
		else
			osassertex( false,"����Ԥ֪�ķ��ؽ��...\n" );
	}
	else
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
		t_bRes = false;
	}


	return t_bRes;

	unguard;
}




/** \brief
*  ��������ָ��һ��Ҫ����ĵ�ͼ��
*
*  \param _mapName Ҫ����ͼ������.
*  \param _mapIdx �����ͼʱ,ʹ�õĵ�ͼ����.ATTENTION TO FIX:
*  \param _waitForFinish ��ͼ������ɺ�,�Ƿ�֪ͨ������̵߳�ͼ�������,
*                  �˻��������������ʽ�ĵ�ͼ����,���������һ��Insert������
*                  ��ʱ��
*/
void osc_mapLoadThread::push_loadMap( 
	char* _mapName,int _mapIdx,BOOL _waitForFinish/* = FALSE*/ )
{
	guard;

	osassert( _mapName&&(_mapName[0] ) );

	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	os_tgMTLoader    t_ptrLoad;

	//! �����ǰ��list��û�������ͼ,���ƽ�,���򷵻�.
	std::list< os_tgMTLoader >::iterator     t_iter;

	for( t_iter = m_sMapNeedLoad.begin();t_iter != m_sMapNeedLoad.end();t_iter ++ )
	{
		if( t_iter->m_szMapName == _mapName )
		{
			::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
			return ;
		}

		// 
		// ������ֲ�ͬ,ȷ��Ҫ����ĵ�ͼ������������ʹ��
		osassertex( (t_iter->m_iMapIdx != _mapIdx),
			va( "The loadMap Idx<%s>in use,Need Load map<%s>",
			t_iter->m_szMapName.c_str(),_mapName ) );
	}

	
	t_ptrLoad.m_ptrTG = &m_ptrLRMgr->m_ptrRealTg[_mapIdx];
	t_ptrLoad.m_ptrTG->set_mtQueueState( TRUE );

	t_ptrLoad.m_szMapName = _mapName;
	t_ptrLoad.m_iMapIdx = _mapIdx;
	t_ptrLoad.m_bFinishNotify = _waitForFinish;

	m_sMapNeedLoad.push_back( t_ptrLoad );

	osDebugOut( "�ͷ�<%d>��ͼ<%s>,�����ͼ<%s>..,�����е�ͼ��Ŀ:<%d>...\n",_mapIdx,
		t_ptrLoad.m_ptrTG->get_mapFName(),_mapName,m_sMapNeedLoad.size() );

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	// 
	// �����¼�,���������̴߳��ڵȴ�״̬,����ִ��.
	::SetEvent( osc_mapLoadThread::m_sEventStartMt );


	//
	// ���������ʽ�ĵ���,�ȴ���ͼ������¼�����,�������¼�.
	if( _waitForFinish )
	{
		::WaitForSingleObject( m_sEventMapLoadFinish,INFINITE );
		::ResetEvent( m_sEventMapLoadFinish );
	}

	return ;

	unguard;
}

//! �Ӷ����е�����ͼ�б�.
void osc_mapLoadThread::pop_loadMap()
{
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	m_sMapNeedLoad.pop_front();

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

}

//! ���Ҫ�����ͼ�Ķ���.
void osc_mapLoadThread::clear_loadMap( void )
{
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );


	while( !m_sMapNeedLoad.empty() )
	{
		// ȷ�����֮ǰ�����Ŷ�״̬���
		m_sMapNeedLoad.front().m_ptrTG->set_mtQueueState( false );
		m_sMapNeedLoad.pop_front();

	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
}


//! �õ���ǰ�˵ĵ�ͼ.
BOOL osc_mapLoadThread::get_frontMap( os_tgMTLoader& _tgLoad )
{
	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	if( 0 == m_sMapNeedLoad.size() )
	{
		::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
	
		// �鿴�Ƿ����˳��̵߳��ź�
		DWORD    t_dwWait;
		t_dwWait = ::WaitForSingleObject( this->m_sEventExitThread,0 );
		if( WAIT_TIMEOUT == t_dwWait )
		{
			return FALSE;
		}
		else if( WAIT_OBJECT_0 == t_dwWait )
		{
			_tgLoad.m_ptrTG = NULL;
			return TRUE;
		}
	}

	osassert( m_sMapNeedLoad.size() > 0 );
	_tgLoad.m_ptrTG = m_sMapNeedLoad.front().m_ptrTG;
	_tgLoad.m_szMapName = m_sMapNeedLoad.front().m_szMapName;
	_tgLoad.m_bFinishNotify = m_sMapNeedLoad.front().m_bFinishNotify;
	_tgLoad.m_iMapIdx = m_sMapNeedLoad.front().m_iMapIdx;

	// �˺���Ϊ�����߳����ã��õ����ݺ������߳���ɾ����map name.
	m_sMapNeedLoad.pop_front();

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	return TRUE;
}



//! ��չ�����Ҫ����ĵ�ͼ,���ͷ�������ͼʱ��������������߳����ڵ����ͼ������
void osc_mapLoadThread::reset_loadMapMgr( void )
{
	guard;

	// �����ǰ�̴߳��ڵȴ�״̬,ֱ�ӷ���.
	if( !is_freeStatus() )
	{
		clear_loadMap();

		// ����̵߳����ȼ���.
		setPriority( THREAD_PRIORITY_ABOVE_NORMAL );

		DWORD   t_dwRes;

		// �˴���Ӧ�õȴ�����¼������¼���ʾ�����߳̿�ʼ������������ɹ���
		//                ʹ��һ���µ�event��ȷ�����״̬��
		t_dwRes = ::WaitForSingleObject( m_sEventThreadFree,INFINITE );
		osassert( t_dwRes != WAIT_FAILED );

		// ���ܻ������������Σ�
		// ���߳��������¼�m_sEventStartMt,����ͼ�����߳��Ѿ�ִ�е���
		// ::ResetEvent( osc_mapLoadThread::m_sEventStartMt );���֮�⣬
		// ����������£��ͻ����m_sEventStartMtһֱ�����ź�״̬�µ����Σ�
		// �˴���Ҫ�ֹ��޸�m_sEventStartMtΪ���ź�״̬
		::ResetEvent( m_sEventStartMt );

		// ȷ�ϴ˴��ĵ�ͼ�����̴߳��ڷǵ���״̬
		osassertex( is_freeStatus(),
			va( "��free״̬����,��ͼ��<%d>....\n",this->m_sMapNeedLoad.size() ) );

		// �����ͼ�����̵߳��Ż�����.
		setPriority( THREAD_PRIORITY_BELOW_NORMAL );

	}

	// ȷ�ϴ˴��ĵ�ͼ�����̴߳��ڷǵ���״̬
	osassertex( is_freeStatus(),"free״̬����...\n" );

	return;


	unguard;
}


//! �õ�Ҫ����ĵ�ͼ������.
void osc_mapLoadThread::get_loadMapName( int _idx,char* _name,int _maxname )
{
	guard;

	::EnterCriticalSection( &osc_mapLoadThread::m_sMapNameCS );

	osassert( (int)m_sMapNeedLoad.size() > _idx );
	int t_i = 0;
	std::list< os_tgMTLoader >::iterator     t_iter;

	for( t_iter = m_sMapNeedLoad.begin();t_iter != m_sMapNeedLoad.end();t_iter ++ )
	{
		if( t_i == _idx )
		{
			osassert( (int)strlen( t_iter->m_szMapName.c_str() ) < _maxname );
			strcpy( _name,t_iter->m_szMapName.c_str() );
			break;
		}

		t_i ++;
	}

	::LeaveCriticalSection( &osc_mapLoadThread::m_sMapNameCS );
	
	return ;

	unguard;
}




/**
*  ÿһ�������඼�������صĺ���.
*
*  ������������: Ӧ����һ���¼�
*  
*/
DWORD osc_mapLoadThread::ThreadProc()
{
	guard;

	while( 1 )
	{

		// ���û�е�ͼ,��ȴ���ͼ������¼�.����¼�����,�ظ������ͼ,ֱ�����.
		if( !get_frontMap( m_sCurLoadMap ) )
		{
			// ���õȴ����߳��ƽ���ͼ�����߳�.	
			::ResetEvent( osc_mapLoadThread::m_sEventStartMt );

			// 
			// ���������߳��ڵȴ���ͼ������ɵ���Ϣ,֪ͨ�¼�.
			// ֻ���ڵ�ǰ���߳�������ȴ���Ϣ�󣬲��ܷ�����ͼ������ɵ���Ϣ��
			if( m_sCurLoadMap.m_bFinishNotify )
				::SetEvent( m_sEventMapLoadFinish );


			osDebugOut( "�ȴ���ͼ�ƽ������߳�,�����ʱ����ͣ�٣����߳�����!!!!\n" );

			// ������̵߳ĵ�ͼ��������ڣ��ò�����Ҫ����ĵ�ͼ��������ȴ���
			do
			{
				
				// �ȴ��̻߳���֮ǰ��Ϊfree�¼�
				::SetEvent( m_sEventThreadFree );

				::WaitForSingleObject( osc_mapLoadThread::m_sEventStartMt,INFINITE );
				
				// �������free�¼�Ϊ���ź�
				::ResetEvent( m_sEventThreadFree );
			}
			while( (!get_frontMap( m_sCurLoadMap )) );
			
			osDebugOut( "�ȴ���ͼ�ƽ������߳�...done\n" );
		}

		// 
		// �˳��̵߳Ŀ���
		if( NULL == m_sCurLoadMap.m_ptrTG ){
			osDebugOut( "�˳��̵߳Ŀ���...break\n" );
			break;
		}

		osDebugOut( "���ͷŵ�ͼ\n" );

		//! ���ͷŵ�ͼ
		m_sCurLoadMap.m_ptrTG->release_TG();

		osDebugOut("start threadSafe_loadNMap...\n");
		//
		//! �����ͼ�ĺ���
		if( !m_ptrLRMgr->threadSafe_loadNMap( 
			(char*)m_sCurLoadMap.m_szMapName.c_str(),m_sCurLoadMap.m_ptrTG ) )
			return FALSE;

	}


	osDebugOut( "Exit mapLoad Thread...\n" );

	return TRUE;

	unguard;
}












	



