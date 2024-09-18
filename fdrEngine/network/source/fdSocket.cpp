//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: fdSocket.cpp
 *
 *  Desc:     �����������õ�������ӿڡ�
 *
 *  His:      River created @ 2004-3-31
 *
 *
 */
//--------------------------------------------------------------------------------------------------------
# include "stdafx.h"
# include "../include/fdSocket.h"

# pragma comment( lib,"Ws2_32" )


//! ���ڱ�ʾ���ݰ���С�������ֽڵĳ��ȡ�
# define PACK_SIZELENGTH   2

//! ���ⲿʹ�õ����ݻ�����
#define CUSE_BUFFER_SIZE      (12*1024)      

#define MAX_CRYPT_DECODE_BUFFER_SIZE 1024 * 10

#ifdef FD_SETSIZE
#undef FD_SETSIZE
#define FD_SETSIZE  RECV_BUFFER_SIZE
#endif

char g_crypt_encode_buffer[MAX_CRYPT_DECODE_BUFFER_SIZE] = {0};
char g_crypt_decode_buffer[MAX_CRYPT_DECODE_BUFFER_SIZE] = {0};

BOOL m_bJustConnectServer = FALSE;


# define SOCKET_RECORD  0

//! ��������ʱ�����ڻ������е���ʼλ�á�
int osc_fdSocket::m_iRecvPos = 0;
//! 
int osc_fdSocket::m_iProcPos = 0;
BYTE* osc_fdSocket::m_pRecvBuf = NULL;
//! �������ջ�����CS
CRITICAL_SECTION osc_fdSocket::m_sRecvBuf;

//!
SOCKET osc_fdSocket::m_sSocket;

HANDLE osc_fdSocket::m_sWsaEvent = NULL;

//! �˳���Ϣ�����̵߳���Ϣ
HANDLE osc_fdSocket::m_sThreadExit = NULL;

//! ���������������Ѿ��ɹ�
BOOL osc_fdSocket::m_bConnectSuccess = false;


# if SOCKET_RECORD
FILE*   g_ptrReceive = NULL;
FILE*   g_ptrDecode = NULL;
# endif 

osc_fdSocket::osc_fdSocket()
{
	m_pRecvBuf = new BYTE[RECV_BUFFER_SIZE];

	// River @ 2005-8-30:���ٷ��ͻ����С����ʡ�ڴ棬����ֻ�������͵�����Ϣ�Ľӿڡ�
	m_pSendBuf = new BYTE[CUSE_BUFFER_SIZE];
	m_pClientUseDBuf = new BYTE[CUSE_BUFFER_SIZE];

	m_iSendPos = 0;
	m_iRecvPos = 0;
	m_iProcPos = 0;
	m_llRecvDataLen = m_llSendDataLen = 0;

	m_bInit = false;

	m_sWsaEvent = FALSE;
	m_threadHandle = NULL;

	// �����ٽ���
	::InitializeCriticalSection( &m_sRecvBuf );

	m_sThreadExit = WSACreateEvent();
	osassertex( WSA_INVALID_EVENT != m_sThreadExit,"���������߳��˳��¼�ʧ��...\n" );

	::InitializeCriticalSection(&m_sendOneMsgSection);

#if USE_ENGINE_CRYPT
	m_pJzCrypt = _exGetjzCryptObj();
	osassert(m_pJzCrypt);
	m_bHasRecvKeyValue = FALSE;
	m_wTotalSize = m_wPackIndex = 0;
#endif //USE_ENGINE_CRYPT

# if SOCKET_RECORD
	g_ptrReceive = fopen( "d:\\bintest\\receive.bin","wb" );
	//g_ptrDecode = fopen( "d:\\t2.bin","wb" );
	fclose( g_ptrReceive );
	//fclose( g_ptrDecode );
# endif 

}


osc_fdSocket::~osc_fdSocket()
{
	close_socket();

	SAFE_DELETE_ARRAY( m_pSendBuf );
	SAFE_DELETE_ARRAY( m_pRecvBuf );
	SAFE_DELETE_ARRAY( m_pClientUseDBuf );

	::DeleteCriticalSection( &m_sRecvBuf );

#if USE_ENGINE_CRYPT
	_exDestroyjzCryptObj(m_pJzCrypt);
#endif //USE_ENGINE_CRYPT

	::DeleteCriticalSection(&m_sendOneMsgSection);
}

/*! \brief �����㷨��Key���ɷ��������ȷ��͹�����
*
*	\param _key						: key value
*/
void osc_fdSocket::set_cryptKey(char* _key,int _len)
{
	guard;

#if USE_ENGINE_CRYPT
	
	osassert(m_pJzCrypt);
	m_pJzCrypt->InitData(_key,_len);

	m_bHasRecvKeyValue = TRUE;
#endif //USE_ENGINE_CRYPT

	unguard;
}
/** \brief
*  ���������Լ��ĳ�ʼ��������
*/
BOOL osc_fdSocket::WSAInitialize ( void )
{
	guard;

   WSADATA WSAData;

   if( WSAStartup(MAKEWORD(1,1), &WSAData) != 0) 
	   return FALSE;

   return TRUE;

	unguard;
}


/** \brief
*  ���ӵ��������Ľӿڡ�
*/
bool osc_fdSocket::connect_server( const char* _hostAddr,int _port,int _WSA,HWND _hwnd )
{
	guard;

	SOCKADDR_IN    t_InAddr;

	osassert( _hostAddr );
	osassert( _hwnd );

	this->m_iRecvPos = 0;
	this->m_iSendPos = 0;
	this->m_iProcPos = 0;
	if( m_sSocket ) 
		close_socket();

	t_InAddr.sin_addr.s_addr = inet_addr( _hostAddr );
	t_InAddr.sin_family = AF_INET;
	t_InAddr.sin_port = htons( (WORD)_port );       
	m_sSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( m_sSocket == INVALID_SOCKET) {
		osDebugOut( "socket return Error code %d",::WSAGetLastError() );
		return FALSE;
	}


	// ����֮ǰѡ��ʹ������ģʽ
	m_sWsaEvent = WSACreateEvent();
	if( WSA_INVALID_EVENT == m_sWsaEvent )
	{
		osDebugOut( "������������¼�ʧ��...\n" );
		return false;
	}

	if( SOCKET_ERROR == WSAEventSelect( m_sSocket,m_sWsaEvent,FD_READ | FD_CLOSE | FD_CONNECT ) )
	{
		// �ر��¼�
		if( m_sWsaEvent )
		{
			WSACloseEvent( m_sWsaEvent );
			m_sWsaEvent = NULL;
		}

		osDebugOut( "�������Ӵ���....\n" );
		return false; 
	}

	// Start the thread.
	m_threadHandle = NULL;
	::WSAResetEvent( m_sThreadExit );
	m_threadHandle = CreateThread( NULL,
		0,
		(LPTHREAD_START_ROUTINE)osc_fdSocket::recv_thread,
		this,
		0,
		(LPDWORD)&m_threadID );

	osDebugOut(va("\nconnect time .... socket index = %ld \n",m_sSocket));

	// River: ����Ϣǰȷ������������ӳɹ�.
	m_bConnectSuccess = false;
	if( connect(m_sSocket,(PSOCKADDR)&t_InAddr,sizeof(t_InAddr)) ==  SOCKET_ERROR )       
	{ 
		if(  WSAEWOULDBLOCK != WSAGetLastError()  )
		{
			osDebugOut( "close_socket: WSAEWOULDBLOCK != WSAGetLastError(), is<%d>\n", WSAGetLastError() );

			// �ڲ���������,��Ҫ�ر���Ӧ���¼����´�����
			close_socket();

			return false;
		}
	}

	m_bInit = true;
	osDebugOut( "connect_server:done.\n" );


	m_bJustConnectServer = TRUE;

	return true;

	unguard;
}


//! ��ǰ��Socket�Ƿ�ر�
bool osc_fdSocket::is_socketClosed( void )
{
	if( m_sSocket == 0 )
		return true;
	else
		return false;
}


/** \brief
*  �رյ�ǰ�����ӡ�
*/
bool osc_fdSocket::close_socket( void )
{
	guard;
	osDebugOut( "close_socket()\n" );

	::EnterCriticalSection( &m_sRecvBuf );

	m_iRecvPos    = 0;
	m_iProcPos    = 0;

	::LeaveCriticalSection( &m_sRecvBuf );


	// �ر��¼�
	if( m_sWsaEvent )
	{
		WSACloseEvent( m_sWsaEvent );
		m_sWsaEvent = NULL;
	}

	// �ر����ݽ����߳�
	if( m_threadHandle )
	{
		::WSASetEvent( m_sThreadExit );
		DWORD dwRet = WaitForSingleObject( m_threadHandle, 15000 ); // �ȴ��˳�
		if( dwRet == WAIT_TIMEOUT)
		{
			osDebugOut( "�����߳��޷��˳���ǿ��ɱ��" );
			TerminateThread(m_threadHandle, 1);
		}

		BOOL  t_bRes = ::CloseHandle( m_threadHandle );
		osassert( t_bRes );
		m_threadHandle = NULL;
		osDebugOut( "close_socket:�ر��߳�\n" );
	}

	m_iSendPos    = 0;
	m_bInit = false;


	if( m_sSocket != 0 ) 
	{

# ifdef _DEBUG
		int t_error = WSAGetLastError();
		osDebugOut( "Close socket,WSA last Error is:%d..\n",t_error );
# endif 

		closesocket( this->m_sSocket );
		m_sSocket = NULL;
		osDebugOut( "close_socket():�ر�socket\n" );
	}

	m_sSocket = 0;


#if USE_ENGINE_CRYPT
	m_bHasRecvKeyValue = FALSE;
	m_wTotalSize = m_wPackIndex = 0;
#endif //USE_ENGINE_CRYPT

	return true;

	unguard;
}


//! ����رպ�Ҫ���õĻص�����
sockClosed_callbackFunc osc_fdSocket::m_ptrClosedCallbackFunc = NULL;

//! ע������رպ���õĿͻ��˻ص����� 
bool osc_fdSocket::register_socketCloseCallback( sockClosed_callbackFunc _func )
{
	guard;

	//osassertex( _func,"����Ϊ��" );
	m_ptrClosedCallbackFunc = _func;

	return true;

	unguard;
}


//! �������ݵ�thread
int osc_fdSocket::recv_thread( void* _param )
{
	guard;

	osassert( _param );
	osc_fdSocket*   t_ptrSock = (osc_fdSocket*)_param;

	enum
	{ 
		EVENT_EXIT, EVENT_PROCESS, MAX_EVENT
	};
	HANDLE   t_sEventArray[MAX_EVENT] = { osc_fdSocket::m_sThreadExit,osc_fdSocket::m_sWsaEvent };


	while( true )
	{
		DWORD t_dwRes = WSAWaitForMultipleEvents( MAX_EVENT,t_sEventArray,false,WSA_INFINITE,false );

		if( t_dwRes == WSA_WAIT_FAILED )
		{
			osDebugOut( "t_dwRes == WSA_WAIT_FAILED\n" );
			break;
		}

		// �߳��˳��¼�����
		if( t_dwRes == EVENT_EXIT )
		{
			osDebugOut( "t_dwRes == EVENT_EXIT\n" );
			return 0;
		}

		WSANETWORKEVENTS t_netevent;
		memset( &t_netevent, 0, sizeof(t_netevent));

		if( WSAEnumNetworkEvents( t_ptrSock->m_sSocket,
			t_ptrSock->m_sWsaEvent, &t_netevent) == SOCKET_ERROR)
		{
			osDebugOut( "WSAEnumNetworkEvents == SOCKET_ERROR\n" );
			break;
		}

		// ���ӳɹ�����ʧ�ܵ��¼�
		if ((t_netevent.lNetworkEvents & FD_CONNECT) == FD_CONNECT)
		{
			if (t_netevent.iErrorCode[FD_CONNECT_BIT] > 0)
			{
				osDebugOut( "��������������ʧ��..\n" );
				break;
			}
			else
			{
				osDebugOut( "���ӳɹ�.....\n" );

				// River: ����Ϣǰȷ������������ӳɹ�.
				m_bConnectSuccess = true;
			}
		}

		// �ر�����...
		if( (t_netevent.lNetworkEvents & FD_CLOSE )== FD_CLOSE )
		{
			int t_error = WSAGetLastError();
			osDebugOut( "���յ�������Ҫ�ر��������Ϣ���ر�����...WSAGetLastError<%d>, NetErrorCode:<%d>\n", 
				t_error, t_netevent.iErrorCode[FD_CLOSE_BIT] );

			// 
			// River @ 2009-1-5: �����ϲ������رջص�����
			if( m_ptrClosedCallbackFunc )
			{
				m_ptrClosedCallbackFunc( NULL );
				m_ptrClosedCallbackFunc = NULL;
			}

			break;
		}

		// ������Ϣ
		if( (t_netevent.lNetworkEvents & FD_READ) == FD_READ )
		{
			if( !osc_fdSocket::receive(t_ptrSock) )	{		
				osDebugOut( "�̶߳�ȡ��Ϣʧ��!\n" );
				break;
			}
		}
	}

	// 
	// ���ر�socket,���������ر�socket�Ĵ��������߳������ô���
	if( t_ptrSock->m_sSocket != 0 ) 
	{

# ifdef _DEBUG
		int t_error = WSAGetLastError();
		osDebugOut( "Close socket,WSA last Error is:%d..\n",t_error );
# endif 

		closesocket( t_ptrSock->m_sSocket );
		t_ptrSock->m_sSocket = NULL;

		osDebugOut( "�߳�closesocket!\n" );
	}

	osDebugOut( "�̷߳���!\n" );
	return 0;

	unguard;
}


//@{
/** \brief
*  ������Ϣ��
*/
bool osc_fdSocket::receive( osc_fdSocket* _ptrSocket )
{
	guard;

	static FILE*   t_file = NULL;

	// River @ 2006-8-30: ����������ڿռ�̫С����ȴ�������̴߳���
	// ֱ�������������㹻�Ŀռ�,��Ϊ�˴�Ϊ��ȡ���ݣ����Բ������ٽ���
	int Rest = RECV_BUFFER_SIZE - m_iRecvPos;
	while( Rest < 8192 )
	{
		::Sleep( 5 );
		osDebugOut( "Socket buffer over....\n" );
		Rest = RECV_BUFFER_SIZE - m_iRecvPos;
	}

	::EnterCriticalSection( &m_sRecvBuf );


	int tReceiveSize = recv( osc_fdSocket::m_sSocket,(char*)( m_pRecvBuf + m_iRecvPos ), Rest, 0);
	if( tReceiveSize == SOCKET_ERROR ) 
	{
		::LeaveCriticalSection( &m_sRecvBuf );
		osassertex( false ,va( "Receive Message Error:%d..\n",WSAGetLastError() )  );
		return false;
	}

	if( tReceiveSize == 0 )
	{
		::LeaveCriticalSection( &m_sRecvBuf );
		osDebugOut( "The socket is closed ....\n" );
		return false;
	}
	
	//osDebugOut( "receive byte<%d>\n", tReceiveSize );

	//
	// ���ջ������ռ�̫С,����ʹ���յ������ݳ���С�ڽ��ջ������Ŀռ��С.
	// ATTENTION TO FIX:��Ӧ�������ʹ�������İ취
	if( tReceiveSize > Rest ) 
	{
		::LeaveCriticalSection( &m_sRecvBuf );
		osassertex( false,"������Ϣ�������ջ��������..\n" );
		return false;
	}

# if SOCKET_RECORD
	g_ptrReceive = fopen( "d:\\bintest\\receive.bin","ab" );
	fseek( g_ptrReceive,0,SEEK_END ); 
	fwrite( (char*)( m_pRecvBuf + m_iRecvPos ),tReceiveSize,sizeof( char ),g_ptrReceive );
	fclose( g_ptrReceive );
# endif 

	m_iRecvPos = m_iRecvPos + tReceiveSize;
	_ptrSocket->m_llRecvDataLen += tReceiveSize;

	::LeaveCriticalSection( &m_sRecvBuf );


	//osDebugOut( "receive end , m_iRecvPos<%d>, tReceiveSize<%d>\n", m_iRecvPos, tReceiveSize );
	return true;

	unguard;
}

#if USE_ENGINE_CRYPT
	//! get the first decode message
	char* osc_fdSocket::GetDecodeMessage(WORD& _size)
	{
		guard;

		if(m_wTotalSize > m_wPackIndex){
			osassert(m_bHasRecvKeyValue);
			const WORD t_orgIndex = m_wPackIndex;

			_size = *(WORD*)(g_crypt_decode_buffer + m_wPackIndex);

			osassert(_size <= 8192);

			m_wPackIndex += _size;

			return g_crypt_decode_buffer + t_orgIndex ;
		}

		return NULL;
		unguard;
	}

	//! decode the message
	char* osc_fdSocket::DecodeMessage(char* _encode,WORD& _size)
	{
		guard;

		if(m_bHasRecvKeyValue){
			m_wTotalSize = m_pJzCrypt->decode((char*)m_pClientUseDBuf,_size,g_crypt_decode_buffer);
			_size = *((WORD*)g_crypt_decode_buffer);

			osassert(_size <= 8192);

			m_wPackIndex = _size;

			return g_crypt_decode_buffer;
		}else{
			return (char*)_encode;
		}

		unguard;
	}
#endif //USE_ENGINE_CRYPT

/** \brief
*  ����Ϣ�������ж���һ����Ϣ��
*
*  ATTENTION TO FIX:
*  ����ײ���������Ϳͻ�����ʹ��ͬһ�����ݻ��壬�п����ƻ����ݡ�
*  
*/
char* osc_fdSocket::read_message( WORD& _msgSize,int* _errorCode,int* _errorType )
{
	guard;
	char*      t_ptrMsg = NULL;
	osassert( _errorCode );
	osassert( _errorType );
	*_errorCode = 0;

	CCriticalSec t_recvSec(&m_sRecvBuf);

#if USE_ENGINE_CRYPT
	if(t_ptrMsg = GetDecodeMessage(_msgSize)){
		return t_ptrMsg;
	}
#endif //USE_ENGINE_CRYPT

	// ���û���㹻�Ŀռ�����ҵ����ݡ�
	if( m_iProcPos >= m_iRecvPos )
	{   
		_msgSize = 0;
		m_iRecvPos = 0;
		m_iProcPos = 0;
		return NULL;
	}

	//
	// �����ǰ��Socketû�г�ʼ��.
	if( !m_bInit ){
		return NULL;
	}

	if( ( m_iRecvPos - m_iProcPos) < PACK_SIZELENGTH ) 
	{
		return NULL;
	}

	WORD*  t_ptrWord;
	t_ptrWord = (WORD*)(m_pRecvBuf + m_iProcPos );
	_msgSize = *t_ptrWord;

	osassertex(_msgSize <= 8192,"������ȷ����������ڴ����ݣ���");

	//
	// �����ǰ���ݰ��ĳ���С�������Ѿ����յ������ݵĳ��ȣ�
	// �ȴ���һ�����ݰ���TCP���ܻ��һ��������ݰ��ֿ����͡�
	if( _msgSize > (m_iRecvPos-m_iProcPos) )
	{
		// River @ 2006-8-25:
		// �����������ղ��������Σ���size����64k�İ�,
		// �����ʱ����ĳһ����Ϣû�н����꣬��Ѵ���Ϣ�����������Ŀ�ͷ��
		// �´εĽ��մ������ͷ��ʼ
		if( m_iRecvPos == RECV_BUFFER_SIZE )
		{
			memcpy( m_pRecvBuf,(const char*)(m_pRecvBuf + m_iProcPos),
				( m_iRecvPos-m_iProcPos ) );
			m_iRecvPos = m_iRecvPos - m_iProcPos;
			m_iProcPos = 0;
		}

		return NULL;
	}

	/*t_ptrMsg = (char*)(m_pRecvBuf + m_iProcPos + 2);
	m_iProcPos += _msgSize ;
	_msgSize -= 2;*/

	// reserve the length of message
	//
	t_ptrMsg = (char*)(m_pRecvBuf + m_iProcPos + 0);
	m_iProcPos += _msgSize ;

	// 
	// ATTENTION TO FIX: ����������ϲ���޸�
	// River 2005-8-5: �����µ����ݻ�����,ȷ���ײ�Ľ�����Ϣ�������������.
	osassertex( _msgSize<=CUSE_BUFFER_SIZE,"Packet Overrun at rec Buffer...\n" );
	memcpy( m_pClientUseDBuf,t_ptrMsg,_msgSize );


# if SOCKET_RECORD
	static int t_iTimess = 0;
	g_ptrDecode = fopen( va( "d:\\bintest\\de%d.bin",t_iTimess++ ),"ab" );
	fwrite( (char*)m_pClientUseDBuf,_msgSize,sizeof( char ),g_ptrDecode );
	fclose( g_ptrDecode );
# endif 


#if USE_ENGINE_CRYPT

	return DecodeMessage((char*)m_pClientUseDBuf,_msgSize);

#else //USE_ENGINE_CRYPT

	return (char*)m_pClientUseDBuf;

#endif //USE_ENGINE_CRYPT

	
	unguard;
}

char* osc_fdSocket::read_buffer( WORD& _msgSize )
{
	osassertex(false,"this function is not suggested");

	//char*      t_ptrMsg;
	::EnterCriticalSection( &m_sRecvBuf );

	if( m_iProcPos >= m_iRecvPos )
	{   
		_msgSize = 0;
		m_iRecvPos = 0;
		m_iProcPos = 0;
		::LeaveCriticalSection( &m_sRecvBuf );
		return NULL;
	}
	if( m_iRecvPos > 0 && m_iProcPos < m_iRecvPos )
	{
		if( m_iRecvPos - m_iProcPos >= CUSE_BUFFER_SIZE )
		{
			memcpy( m_pClientUseDBuf, m_pRecvBuf + m_iProcPos, CUSE_BUFFER_SIZE );
			_msgSize = CUSE_BUFFER_SIZE;
			m_iProcPos += CUSE_BUFFER_SIZE;
			::LeaveCriticalSection( &m_sRecvBuf );
			return (char*)m_pClientUseDBuf;
		}
		else
		{
			memcpy( m_pClientUseDBuf, m_pRecvBuf + m_iProcPos, m_iRecvPos- m_iProcPos );
			_msgSize = m_iRecvPos- m_iProcPos;
			m_iRecvPos = 0;
			m_iProcPos = 0;
			::LeaveCriticalSection( &m_sRecvBuf );
			return (char*)m_pClientUseDBuf;
		}
	}


	::LeaveCriticalSection( &m_sRecvBuf );

	//can not run here!!!
	return NULL;
}


//@}


//@{
//! ������ص���Ϣ��
bool osc_fdSocket::add_message( const char* _pmsg,WORD _size )
{
	guard;

	WORD*  t_ptrWord;

	osassert( _size <= MAX_MESSAGE_SIZE );

	// ����������ݹ���
	if( ( m_iSendPos + _size + PACK_SIZELENGTH )>= SEND_BUFFER_SIZE ) 
		return false;

	//
	// �������ݴ�С������ͷ�����ҿ������ݵ����ǵĻ�������
	t_ptrWord = (WORD*)(m_pSendBuf+m_iSendPos);


	// û��ѹ��
	//*t_ptrWord = _size + 2;
	//memcpy( (t_ptrWord + 1), _pmsg , _size );
	//*(t_ptrWord + 1) = _size;	
	//m_iSendPos += ( _size + 2 );

	memcpy( t_ptrWord, _pmsg , _size );
	m_iSendPos += _size;

	return true;

	unguard;
}

bool osc_fdSocket::send_message( void )
{
	guard;

	if( (m_iSendPos > SEND_BUFFER_SIZE) || 
		(m_iSendPos < 0 ) || (m_sSocket == 0)  )
	{  
		osDebugOut( "Error with Send Pos m_iSendPos[%d]m_sSocket[%d]...\n",m_iSendPos,m_sSocket );
		m_iSendPos = 0;
		return false;
	}

	int SendCount = 0;
	int SendTotalBufLen = 0;	//!<��ʵ�ʷ��ͳ���
	bool bSendBufError = false;	//!< ����ʱ����
	do 
	{

#if USE_ENGINE_CRYPT

		m_pJzCrypt->simpledealdata((char*)(m_pSendBuf+SendTotalBufLen),m_iSendPos-SendTotalBufLen);
		int t_iSend = send( m_sSocket,(const char*)m_pSendBuf+SendTotalBufLen, m_iSendPos-SendTotalBufLen, 0 );

#else
		int t_iSend = send( m_sSocket,(const char*)m_pSendBuf+SendTotalBufLen, m_iSendPos-SendTotalBufLen, 0 );

#endif //USE_ENGINE_CRYPT
		
		if (t_iSend==SOCKET_ERROR)
		{
			if( SendCount > 10 )
			{
				bSendBufError = true;
				osDebugOut( "!!!(1)osc_fdSocket::send(), m_sSocket:<%d>, SendTotalBufLen:<%d>, m_iSendPos:<%d>\n", 
					m_sSocket, SendTotalBufLen, m_iSendPos );
				break;
			}

			osDebugOut( "!!!(2)osc_fdSocket::send(), m_sSocket:<%d>, SendTotalBufLen:<%d>, m_iSendPos:<%d>\n", 
				m_sSocket, SendTotalBufLen, m_iSendPos );
			Sleep( 100 );
			SendCount ++;

			continue;
			//break;
		}
		SendTotalBufLen+=t_iSend;
	} while(SendTotalBufLen!=m_iSendPos);


	if (bSendBufError)
	{
		int t_error = WSAGetLastError();
		osDebugOut( "Send msg failed,WSA last Error is:%d..\n",t_error );
		return false;
	}

	/*int t_iSend = send( m_sSocket,(const char*)m_pSendBuf,m_iSendPos, 0 );
	if( t_iSend == SOCKET_ERROR || t_iSend != m_iSendPos )
	{  
		m_iSendPos = 0;

		int t_error = WSAGetLastError();
		osDebugOut( "Send msg failed,WSA last Error is:%d..\n",t_error );

		return false;
	}*/

	m_iSendPos = 0;
	return true; 

	unguard;
}


//! ���͵�����ָ���õĺ�����
bool osc_fdSocket::send_oneMsg( const char* _msg,WORD _size )
{
	guard;

	osassert( _msg );
	osassert( _size >= 0 );
	osassert( _size <= MAX_MESSAGE_SIZE );
	
	CCriticalSec t_sec(&m_sendOneMsgSection);

	if(is_socketClosed()){return true;}
		
	// 
	// River mod @ 2008-11-26: ȷ�Ϸ���Ϣʱ�����������ӳɹ�
	if( !m_bConnectSuccess )
	{
		int   t_iWaitTimes = 0;
		while( !m_bConnectSuccess )
		{
			::Sleep( 20 );
			t_iWaitTimes ++;
			if( t_iWaitTimes > 50 )
				return false;
		}
	}

	if( !add_message( _msg,_size ) )
	{
		MessageBox( NULL, "add_message()�����ۼƳ������buffer!", "socket error", MB_OK );
		return false;
	}

	if( !send_message() )
	{
		osDebugOut( "send_message()����!,last error is:<%d>..\n",WSAGetLastError());
		return true;
	}

	m_llSendDataLen += _size;

	return true;

	unguard;
}
//@}
