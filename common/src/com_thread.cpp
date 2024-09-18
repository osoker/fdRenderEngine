//====================================================================================================
/** \file  
 *  Filename: com_thread.cpp
 *  Desc:     ���߳�Ӧ�õķ�װ��,ʹ�õ�ǰ�ļ��ṩ������ü򵥵Ľӿ�ʹ�ö�
 *            �̺߳���.
 *  His:      River create @ 4/11 2003 0:02
 */
//====================================================================================================

# include "stdafx.h"

#include "../include/com_thread.h"
# include "../include/sgdebug.h"
#include "../include/timecounter.h"


COMMON_API ExceptionCatchHandler_multi_thread      g_pExceptionFuncHandler_multi_thread = NULL;

static DWORD WINAPI cThreadProc( com_thread *pThis )
{
  return pThis->thread_process();
}


//! ���߳����к����İ�װ.
DWORD com_thread::thread_process( void )
{

# if ( _DEBUGRUNTIME )
	CSE::MapSEToCE();
	try{	
#endif

	return ThreadProc();

# if ( _DEBUGRUNTIME )
	}catch(CSE& _se){
		if(g_pExceptionFuncHandler_multi_thread){
			(g_pExceptionFuncHandler_multi_thread)(_se);
		}

		// �̱߳���ִ�����¶�������� Event �ͷš���Ȼ�����˳���
		ThreadExceptionProc();
	}
#endif

	return 1;
}



/**
 *  �߳̽������еĺ���.
 */
void com_thread::End()
{
#if defined( _WIN32 ) && defined( _MT )

	if( m_threadHandle != NULL )
	{
		m_bIsRunning = false;
		WaitForSingleObject( m_threadHandle, INFINITE );
		CloseHandle( m_threadHandle );
		m_threadHandle = NULL;
	}
# else

# error "���붨��win32�Ͷ��̵߳ĺ�...."

# endif 

}

//SetThreadPrionrity

//! �����̵߳��Ż���.
void com_thread::setPriority( int _pri )
{
	guard;

	if( m_threadHandle )
	{
		if( !SetThreadPriority( m_threadHandle,_pri ) )
			osassertex( false,"�����߳����ȼ�ʧ��...\n" );
	}

	return ;

	unguard;
}


/**
*  �߳̿�ʼ���еĺ���.
*/
void com_thread::Begin()
{
	guard;

	{
#if defined( _WIN32 ) && defined( _MT )
		if( m_threadHandle )
			End();  // just to be safe.

		// Start the thread.
		m_threadHandle = CreateThread( NULL,
			0,
			(LPTHREAD_START_ROUTINE)cThreadProc,
			this,
			0,
			(LPDWORD)&m_threadID );
		if( m_threadHandle == NULL )
		{
			// Arrooga! Dive, dive!  And deal with the error, too!
			osassertex( false,"�����߳�ʧ��..\n" );
		}
		m_bIsRunning = true;
		if(sg_timer::Instance()->get_cpuNum() > 1){

			if(!SetThreadAffinityMask(m_threadHandle,2)){
				DWORD t_dwError = ::GetLastError();
				t_dwError = 0;

			}
		}

# else

# error "���붨��win32�Ͷ��̵߳ĺ�...."

# endif 
	}

	unguard;
}

