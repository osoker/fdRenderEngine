///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: os_characterMtLoader.cpp
 * 
 *  His:      River created @ 2009-6-6
 *
 *  Desc:     ����Ķ��̵߳�����,Ϊ����Ϸ���³��������ʱ�򲻿�
 * 
 *  "���б�ĩ������ʼ�գ�֪���Ⱥ��������" 
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/osCharacter.h"
# include "../include/middlePipe.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../../backpipe/include/mainEngine.h"

/*
//! ���̵߳���ļ�����Ҫ�¼���
1: ��ʼ��cs��event
2: ���߳�����Ҫ���������,�����ѵ����̡߳�
3: �����߳�һֱ������ֱ���µ��������룬����ѭ���������������
4: �����̷߳����˳�ָ��
5: 
*/

//! ���ƽ��Ͷ���Ҫ�������������ʱ,ʹ�õ��ٽ���.
CRITICAL_SECTION osc_charLoadThread::m_sCharNameCS;

//! River @ 2011-2-25:ͬ��Ҫɾ��������ָ���ٽ���.
CRITICAL_SECTION osc_charLoadThread::m_sCharReleaseCS;

//! ��������߳��Ƿ��ڿ���״̬���ڵȴ��̱߳����ѵ���Ϣ
HANDLE osc_charLoadThread::m_sEventLCThreadFree = NULL;

//! �Ƿ�����������̵߳�event.
HANDLE osc_charLoadThread::m_sEventLCStartMt = NULL;

//! �˳���ǰ�߳�
HANDLE osc_charLoadThread::m_sEventLCExitThread = NULL;

//! ��ǰ�������Ƿ�������.
HANDLE osc_charLoadThread::m_sEventCLLoadFinish = NULL;


osc_charLoadThread::osc_charLoadThread()
{
	//!  char name�������
	::InitializeCriticalSection( &osc_charLoadThread::m_sCharNameCS );

	//! char release.
	::InitializeCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

	m_sEventLCStartMt = ::CreateEvent( 
		NULL,
		TRUE,
		FALSE,  // ��ʼ��Ϊ���ź�.
		NULL );
	osassert( m_sEventLCStartMt );


	//! ��������߳��Ƿ��ڿ���״̬���ڵȴ��̱߳����ѵ���Ϣ
	m_sEventLCThreadFree = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// ��ʼ��Ϊ���ź�
		NULL );
	osassert( m_sEventLCThreadFree );

	// �߳��˳�
	m_sEventLCExitThread = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,// ��ʼ��Ϊ���ź�
		NULL );
	osassert( m_sEventLCExitThread );

	m_sEventCLLoadFinish = ::CreateEvent(
		NULL,
		TRUE,
		FALSE, // ��ʼ��Ϊ���ź�.
		NULL );
	osassert( m_sEventCLLoadFinish );
	::ResetEvent( m_sEventCLLoadFinish );

}

osc_charLoadThread::~osc_charLoadThread()
{
	//! char name�������
	::DeleteCriticalSection( &osc_charLoadThread::m_sCharNameCS );
	::DeleteCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

	if( m_sEventLCStartMt )
		::CloseHandle( m_sEventLCStartMt );
	if( m_sEventLCThreadFree )
		::CloseHandle( m_sEventLCThreadFree );
}

//  
//! ������Ӧ��ָ��.
void osc_charLoadThread::init_mtLoadCharMgr( void )
{
	guard;




	unguard;
}


//! ÿһ֡�Զ��̵߳�ͼ������������д���
void osc_charLoadThread::frame_moveCLMgr( void )
{

}

/** \brief
*  ��������ָ��һ��Ҫ����ĵ�ͼ��
*
*  \param _osc_charLoadData* _ldata 
*  \param _waitForFinish �Ƿ�ȴ�������߳���ɵ�ͼ�ĵ���,
*                  �˻��������������ʽ�ĵ�ͼ����.
*/
void osc_charLoadThread::push_loadChar( os_charLoadData* _ldata,
									  BOOL _waitForFinish/* = FALSE*/ )
{
	guard;

	//  �Ƚ����ٽ���
	CCriticalSec t_charName(&osc_charLoadThread::m_sCharNameCS);

	if( _waitForFinish )
		memcpy( &m_sVipChar,_ldata,sizeof(os_charLoadData ) );
	else
	{
		m_listCharLoadData.push_back( *_ldata );
	}


	//! ������������߳�
	::SetEvent( m_sEventLCStartMt );
	t_charName.LeaveCriticalSec();

	// ����ȴ����յ���.
	if( _waitForFinish )
	{
		::WaitForSingleObject( m_sEventCLLoadFinish,INFINITE );
		::ResetEvent( m_sEventCLLoadFinish );
	}

	unguard;
}

//! ��չ�����Ҫ���������,���ͷ�������ͼʱ��������������߳����ڵ����������
void osc_charLoadThread::reset_loadCharMgr( void )
{
	guard;

	// �����ǰ�̴߳��ڵȴ�״̬,ֱ�ӷ���.
	if( !is_freeStatus() )
	{
		m_iDataPushPtr = 0;
		m_iProcessPtr = 0;

		// ����̵߳����ȼ���.
		setPriority( THREAD_PRIORITY_ABOVE_NORMAL );

		DWORD   t_dwRes;
		// �˴���Ӧ�õȴ�����¼������¼���ʾ�����߳̿�ʼ������������ɹ���
		//                ʹ��һ���µ�event��ȷ�����״̬��
		t_dwRes = ::WaitForSingleObject( m_sEventLCThreadFree,INFINITE );
		osassert( t_dwRes != WAIT_FAILED );

		::ResetEvent( m_sEventLCStartMt );

		// ȷ�ϴ˴��ĵ�ͼ�����̴߳��ڷǵ���״̬
		osassertex( is_freeStatus(),"�˳���������߳�ʱ���ǿ��е�״̬....\n" );

		// �����ͼ�����̵߳��Ż�����.
		setPriority( THREAD_PRIORITY_BELOW_NORMAL );
	}

	// ȷ�ϴ˴��ĵ�ͼ�����̴߳��ڷǵ���״̬
	osassertex( is_freeStatus(),"free״̬����...\n" );

	return;

	unguard;
}


//! ��ǰ�Ƿ�������״̬,���ȴ����߳��ƽ�Ҫ���������
bool osc_charLoadThread::is_freeStatus( void )
{
	guard;

	DWORD    t_dwWait;
	bool     t_bRes;

	// FreeStatus״̬���˲����ڵ���״̬�⣬����û����Ҫ����ĵ�ͼ��
	::EnterCriticalSection( &osc_charLoadThread::m_sCharNameCS );

	if( m_iProcessPtr>=m_iDataPushPtr )
	{
		::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
		
		t_dwWait = ::WaitForSingleObject( m_sEventLCStartMt,0 );
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
		::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
		t_bRes = false;
	}

	return t_bRes;

	unguard;
}

//! River @ 2011-2-12:�޸Ĵ˺�����ʹ�������Ч��
//! �õ����Ե����character,���������û�����񣬷���false.
bool osc_charLoadThread::get_frontChar( os_charLoadData& _cl )
{
	guard;

	m_bVipProcess = false;

	//  
	//! �˴�����ʹ��CS,��ͬ�߳̽��еĲ���,�����̼߳��롣
	::EnterCriticalSection( &osc_charLoadThread::m_sCharNameCS );
	// 
	// River @ 2011-2-12:���ڻ�û�д�����Ҫɾ������������ڵ�������Ҫɾ����
	//                   ���������ϵ���������ʱ����ɾ����
	LIST_charLoadData::iterator t_iter;
	if( m_listCharLoadData.size() >= 2 )
	{
		for( t_iter = m_listCharLoadData.begin();
			t_iter != m_listCharLoadData.end(); )
		{
			if( t_iter->m_bReleaseChar )
			{
				LIST_charLoadData::iterator  t_find;
				bool                         t_bPreRelease = false;
				for( t_find = m_listCharLoadData.begin();
					t_find != t_iter;  )
				{
					if( DWORD( t_iter->m_ptrSkinPtr) != DWORD( t_find->m_ptrSkinPtr) )
					{
						++ t_find;
						continue;
					}
					
					//! River mod @ 2011-3-6:Ϊ�˽�������ڵ�����Ч���µ�BUG
					//  ���Ҫ�ͷŵ����ǰ��������ָ����Ҫ���ŵ�����Ч����Ҫɾ����
					//  ��ɾ����Щָ�룬������Ҫɾ������������ָ�롣
					if( (!t_find->m_bPlaySwordGlossy)&&(!t_find->m_bReleaseChar)
						&&(!t_find->m_bChangeEquipent)  )
						t_bPreRelease = true;

					// ɾ������ָ��ṹ�Ĳ���
					m_listCharLoadData.erase( t_find++ );

				}

				if( !t_bPreRelease )
				{
					++ t_iter;
					continue;
				}

				// ɾ��ɾ��ָ��Ĳ���
				m_listCharLoadData.erase( t_iter ++ );
			}
			else // River @ 2011-3-7��ʹ����ȷ��ɾ��iterator�ķ�����
				++ t_iter;


			//! ��������һ����Ҳ�˳�.
			if( t_iter == m_listCharLoadData.end() )
				break;

			//�� ��ʱ�п�����������Ϊ�յ�,���Ϊ�գ��˳���
			if( m_listCharLoadData.size() == 0 )
				break;
		}
	}

	// VIP process�Ĵ���
	if( m_sVipChar.m_ptrSkinPtr )
	{
		//! �����ǰ��char���л�װ���Ļ����ȴ����ڶ����ڴ�ptrһ����������Ϣ��
		if( m_sVipChar.m_bChangeEquipent )
		{
			for( t_iter = m_listCharLoadData.begin();
				t_iter != m_listCharLoadData.end();t_iter ++ )
			{
				if( (DWORD)t_iter->m_ptrSkinPtr != (DWORD)m_sVipChar.m_ptrSkinPtr )
					continue;

				memcpy( &_cl,(os_charLoadData*)&(*t_iter),sizeof( os_charLoadData ) );
				m_listCharLoadData.erase( t_iter );

				// �ȴ���vip charָ���Ӧ������������Ϣ
				::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
				return true;
			}
		}

		memcpy( &_cl,&m_sVipChar,
			sizeof( os_charLoadData ) );
		m_bVipProcess = true;
		m_sVipChar.m_ptrSkinPtr = NULL;

		::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );

		return true;
	}

	if( m_listCharLoadData.size() == 0 )
	{
		// �鿴�Ƿ����˳��̵߳��źš�
		DWORD    t_dwWait;
		t_dwWait = ::WaitForSingleObject( m_sEventLCExitThread,0 );
		if( WAIT_TIMEOUT == t_dwWait )
		{
			// ���õȴ����߳��ƽ�������뵽���߳�.	
			::ResetEvent( osc_charLoadThread::m_sEventLCStartMt );
			
			::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );
			return false;
		}
		else if( WAIT_OBJECT_0 == t_dwWait )
		{
			_cl.m_ptrSkinPtr = NULL;
			::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );

			return true;
		}
	}

	// River @ 2011-2-25:�ҵ��������ȼ�����ߵ�character�����롣
	int  t_sLoadLevel = 0;
	LIST_charLoadData::iterator   t_iLoadIter;
	for( t_iter = m_listCharLoadData.begin();
		t_iter != m_listCharLoadData.end();t_iter ++ )
	{
		if( t_iter->m_ptrSkinPtr->get_mtLoadLevel() > t_sLoadLevel )
		{
			t_sLoadLevel = t_iter->m_ptrSkinPtr->get_mtLoadLevel();
			t_iLoadIter = t_iter;
		}
	}

	// ����ϲ���Ϊ�����˵�������ȼ���,�����ȵ����������.
	if( t_sLoadLevel > 0 )
	{
		memcpy( &_cl,(os_charLoadData*)&(*t_iLoadIter),sizeof( os_charLoadData ) );
		m_listCharLoadData.erase( t_iLoadIter );
	}
	else
	{
		// ������Ϸ����£�һ��������.
		t_iter = m_listCharLoadData.begin();
		memcpy( &_cl,(os_charLoadData*)&(*t_iter),sizeof( os_charLoadData ) );
		m_listCharLoadData.pop_front();
	}

	::LeaveCriticalSection( &osc_charLoadThread::m_sCharNameCS );

	return true;

	unguard;
}


/**
*  ÿһ�������඼�������صĺ���.
*
*  ������������:
*  
*/
DWORD osc_charLoadThread::ThreadProc()
{
	guard;

	while( 1 )
	{

		// ���û�е�ͼ,��ȴ���ͼ������¼�.����¼�����,�ظ������ͼ,ֱ�����.
		if( !get_frontChar( m_sCurLoadChar ) )
		{
			//osDebugOut( "�ȴ������ƽ������߳�,�����ʱ����ͣ�٣����߳�����!!!!\n" );

			// ������̵߳ĵ�ͼ��������ڣ��ò�����Ҫ����ĵ�ͼ��������ȴ���
			do
			{
				// �ȴ��̻߳���֮ǰ��Ϊfree�¼�
				::SetEvent( osc_charLoadThread::m_sEventLCThreadFree );

				::WaitForSingleObject( osc_charLoadThread::m_sEventLCStartMt,INFINITE );

				// �������free�¼�Ϊ���ź�
				::ResetEvent( osc_charLoadThread::m_sEventLCThreadFree );
			}
			while( (!get_frontChar( m_sCurLoadChar )) );
			
			//osDebugOut( "�ȴ������ƽ������߳�...done\n" );
		}

		// 
		// �˳��̵߳Ŀ���
		if( NULL == m_sCurLoadChar.m_ptrSkinPtr )
		{
			osDebugOut( "�˳��߳�charLoad�߳�...break\n" );
			break;
		}

		//! ����Reset�豸���ٽ���
		CCriticalSec t_shadowSec(&g_sShadowSec);

		osassert( m_sCurLoadChar.m_ptrSkinPtr );
		//! �����Ҫ�ͷ�����.
		if( m_sCurLoadChar.m_bReleaseChar )
		{
			m_sCurLoadChar.m_ptrSkinPtr->
				release_characterThread( m_sCurLoadChar.m_bSceneSm );
		}
		else if( m_sCurLoadChar.m_bChangeEquipent )
		{	
			m_sCurLoadChar.m_ptrSkinPtr->
				change_equipmentThread( m_sCurLoadChar.m_sEquipent );
		}
		// River @ 2011-2-25:���̴߳�������ĵ������ݵ���
		else if( m_sCurLoadChar.m_bPlaySwordGlossy )
		{
			m_sCurLoadChar.m_ptrSkinPtr->playSwordGlossyThread();
		}
		else
		{
			//! �˴���ʼ�������������ݵ���
			if( !m_sCurLoadChar.m_ptrSkinPtr->
				load_skinMeshSystem( &m_sCurLoadChar.m_sMeshInit ) )
			{
				osDebugOut( "��������<%s>ʧ��...\n",m_sCurLoadChar.m_sMeshInit.m_strSMDir );
			}
			else
			{
				// �˴������ʹ��CS,���ܸ��ж�����ɾ���󣬻�û������֮ǰ�����߳�����Ϊɾ��.
				::EnterCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );
				if( !m_sCurLoadChar.m_ptrSkinPtr->get_toBeRelease() )
					m_sCurLoadChar.m_ptrSkinPtr->set_loadLock( false );
				::LeaveCriticalSection( &osc_charLoadThread::m_sCharReleaseCS );

			}
		}

		// �����ʱ���õ�vip process,֪ͨ.
		if( m_bVipProcess )
		{   
			m_bVipProcess = false;
			::SetEvent( m_sEventCLLoadFinish );
		}

		//! �˳�reset�������ٽ���
		t_shadowSec.LeaveCriticalSec();

	}

	osDebugOut( "Exit charLoad Thread...\n" );

	return TRUE;

	unguard;
}

void osc_charLoadThread::ThreadExceptionProc()
{
	// �����ʱ���õ�vip process,֪ͨ.
	if( m_bVipProcess )
	{
		m_bVipProcess = false;
		::SetEvent( m_sEventCLLoadFinish );
	}
}

//! �����̵߳�ִ��
void osc_charLoadThread::end_thread( void )
{
	guard;

	// ʹ�õ�ͼ���봦�ڵȴ�״̬.
	reset_loadCharMgr();

	// 
	// �ȼ����ͼ�����̣߳��ټ����˳��߳��¼��������ͼ����
	// ����Ϊ�գ�����ʱ�˳��߳��¼����źţ����˳��̡߳�
	::SetEvent( osc_charLoadThread::m_sEventLCExitThread );
	::SetEvent( osc_charLoadThread::m_sEventLCStartMt );

	unguard;
}



