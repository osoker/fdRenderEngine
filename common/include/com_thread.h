//====================================================================================================
/** \file  
 *  Filename: com_thread.h
 *  Desc:     ���߳�Ӧ�õķ�װ��,ʹ�õ�ǰ�ļ��ṩ������ü򵥵Ľӿ�ʹ�ö�
 *            �̺߳���.
 *  His:      River create @ 4/11 2003 0:02
 */
//====================================================================================================
#pragma once


# include <windows.h>

# include "../common.h"

/** \brief �Զ��̵߳ķ�װ��.
 *  
 *  ʹ�����������ü򵥵Ľӿ�ʹ�ö��߳�,����Ҫ�ӵ�ǰ����������,��Ϊÿһ��
 *  �̶߳����Լ�����Ĵ�����.
 *
 */
class COMMON_API com_thread
{
protected:
	//! �̵߳�handle.
	HANDLE      m_threadHandle;
	//! �̵߳�ID.
	DWORD       m_threadID;
	//! �߳��ǲ�����������.
	bool        m_bIsRunning;

public:
	/**
	 *  �߳̽������еĺ���.
	 */
	void End();

	/**
	 *  �߳̿�ʼ���еĺ���.
	 */
	void Begin();


	//! �����̵߳��Ż���.
	void setPriority( int _pri );

	/**
	 *  �߳��ǲ������еĺ���.
	 */
	bool IsRunning()
	{
		return m_bIsRunning;
	}


	/**
	 *  ÿһ�������඼�������صĺ���.
	 */
	virtual DWORD ThreadProc() = 0;
	virtual void ThreadExceptionProc() = 0;

	com_thread(void)
	{
		m_threadID      = 0;
		m_threadHandle  = NULL;
		m_bIsRunning    = false;
	}

	virtual ~com_thread(void)
	{
		End();
	}

	//! ���߳����к����İ�װ.
	DWORD        thread_process( void );

};


/** \ brief ��Mutex��ķ�װ.
 *  
 *  ������װMutex��,ʹ�ö��߳̿��԰�ȫ��������ϵͳ��.
 *
 */
class COMMON_API com_mutex
{
	//! Mutex�ľ��
	HANDLE m_mutex;


public:
	com_mutex()
	{
#if defined( _WIN32 ) && defined( _MT )
		// This mutex will help the two threads share their toys.
		m_mutex = CreateMutex( NULL, false, NULL );
		if( m_mutex == NULL )
			throw ( "cMonitor::cMonitor() - Mutex creation failed." );
#endif
	}


	virtual ~com_mutex()
	{
#if defined( _WIN32 ) && defined( _MT )
		if( m_mutex != NULL )
		{
			CloseHandle( m_mutex );
			m_mutex = NULL;
		}
#endif
	}

	/**
	 *  ��ʼʹ��Mutex,����ؼ����ݲ�������
	 */
	void MutexOn() const
	{
#if defined( _WIN32 ) && defined( _MT )
		WaitForSingleObject( m_mutex, INFINITE );  // To be safe...
#endif
	}


	/**
	 *  �뿪�ؼ����ݲ��������ر�Mutex.
	 */
	void MutexOff() const
	{
#if defined( _WIN32 ) && defined( _MT )
		ReleaseMutex( m_mutex );  // To be safe...
#endif
	}


};

